#include "peer_connection.hh"

#include <netinet/in.h>

#include <cassert>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <string>

#include "sock_util.hh"

void PeerConnection::start() {
  while (true) {
    try {
      // reset connection parameters and restart connection
      _conn.release();
      _peer_id = "";
      _peer = _queue->pop_front();
      if (_peer.ip == DUMMY_IP) {
        break;
      }
      _status = {};
      establish_peer_connection();

      auto request_pending = false;
      // get next block to request
      auto block = _pieces_manager->next_request(_peer_id);
      // loop of request and receive
      while (!_pieces_manager->finished()) {
        auto msg = recieve_message();
        auto type = msg.type;
        if (type == msg_type::choke) {
          _status.choked = true;
        } else if (type == msg_type::unchoke) {
          _status.choked = false;
        } else if (type == msg_type::piece) {
          auto piece_idx = htoi(msg.payload.substr(0, 4));
          auto offset = htoi(msg.payload.substr(4, 4));
          auto block_data = msg.payload.substr(8);
          // data recieved should be consist with requested block
          assert(piece_idx == block.piece_idx);
          assert(offset == block.offset);
          assert(block_data.length() == block.length);
          _pieces_manager->block_recieved(_peer_id, piece_idx, offset, msg.payload);
          request_pending = false;
        } else if (type == msg_type::have) {
          auto piece_idx = htoi(msg.payload);
          _pieces_manager->peer_update(_peer_id, piece_idx);
        }
        if (!_status.choked && _status.interested && !request_pending) {
          request(block);
          request_pending = true;
          block = _pieces_manager->next_request(_peer_id);
        }
      }
    } catch (std::runtime_error &e) {
      _pieces_manager->peer_del(_peer_id);
      // FIX: don't throw exception, cause it will terminate the whole process, log instead
      throw std::runtime_error("Error in connection with peer [" + _peer.ip + ":" + std::to_string(_peer.port) + "]: " + e.what());
    }
  }
}

void PeerConnection::establish_peer_connection() {
  try {
    _conn = make_unique<ConnectionWrapper>(_peer.ip, _peer.port);
  } catch (std::runtime_error &e) {
    throw std::runtime_error("Failed to connect to peer");
  }

  try {
    perform_handshake();
    receive_bitfield();
    send_interested();
  } catch (std::runtime_error &e) {
    throw std::runtime_error("Failed at preparation work before request");
  }
}

void PeerConnection::perform_handshake() {
  // send handshake
  auto shake_send = serialize_handshake({.info_hash = _info_hash, .peer_id = _client_id});
  _conn->send_data(shake_send);
  // receive handshake
  // WARNING: hard code, pstr may different
  auto shake_reply = deserialize_handshake(_conn->recv_data(shake_send.length()));
  _peer_id = shake_reply.peer_id;
  if (shake_reply.info_hash != _info_hash) {
    throw std::runtime_error("Failed to handshake with peer " + _peer.ip + ": [Received mismatching info hash]");
  }
}

void PeerConnection::receive_bitfield() {
  auto msg = recieve_message();
  if (msg.type == msg_type::bitfield) {
    auto bitfield = msg.payload;
    // bitfiled of a peer is recorded by _pieces_manager,
    // and will be used to determine which pieces current connection should request
    _pieces_manager->peer_add(_peer_id, bitfield);
  } else {
    // The bitfield message may only be sent immediately after the handshaking sequence is completed, and before any
    // other messages are sent. [BitTorrentSpecification](wiki.theory.org/BitTorrentSpecification)
    throw std::runtime_error("Failed to receive bitfield: [Received message type " + std::to_string(msg.type) + "]");
  }
}

void PeerConnection::send_interested() {
  auto interested = std::string{2};
  interested[0] = 1;
  interested[1] = msg_type::interested;
  _conn->send_data(interested);
}

// TODO:
void PeerConnection::request(Block b) {}

auto PeerConnection::serialize_handshake(HandShake h) -> std::string {
  auto res = std::string{};
  res += static_cast<char>(h.pstr.length());
  res += h.pstr;
  res += std::string(h.reserved, 8);
  res += h.info_hash;
  res += h.peer_id;
  return res;
}

auto PeerConnection::deserialize_handshake(std::string s) -> HandShake {
  assert(s.length() > 0);

  auto shake = HandShake{};
  // pstrlen
  auto offset = 0;
  auto pstr_len = static_cast<int>(s[offset]);
  assert((1 + pstr_len + 8 + 20 + 20) == s.length());
  // pstr
  offset += 1;
  shake.pstr = s.substr(offset, pstr_len);
  // reserved
  offset += pstr_len;
  auto reserved = s.substr(offset, 8);
  for (size_t i = 0; i < reserved.length(); i++) {
    shake.reserved[i] = reserved[i];
  }
  // info_hash
  offset += 8;
  shake.info_hash = s.substr(offset, 20);
  // peer_id
  offset += 20;
  shake.peer_id = s.substr(offset, 20);

  return shake;
}

auto PeerConnection::recieve_message() -> Message {
  auto message = _conn->recv_data();
  if (message.empty()) {
    return {msg_type::keep_alive, {}};
  } else {
    auto type = message[0];
    auto payload = message.substr(1);
    return {type, payload};
  }
}
