#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>

#include "peer.hh"
#include "pieces_manager.hh"
#include "shared_queue.hh"
#include "sock_util.hh"

struct HandShake {
  std::string pstr = "BitTorrent protocol";
  char reserved[8] = {};
  std::string info_hash;
  std::string peer_id;
};

struct ConnectionStatus {
  bool choked = true;
  bool interested = false;
};

namespace msg_type {
constexpr int keep_alive = -1;
constexpr int choke = 0;
constexpr int unchoke = 1;
constexpr int interested = 2;
constexpr int notInterested = 3;
constexpr int have = 4;
constexpr int bitfield = 5;
constexpr int request = 6;
constexpr int piece = 7;
constexpr int cancel = 8;
};  // namespace msg_type

struct Message {
  int type;
  std::string payload;
};

class PeerConnection {
 public:
  PeerConnection(std::shared_ptr<SharedQueue<Peer>> queue, std::shared_ptr<PiecesManager> pieces_manager, std::string client_id, std::string info_hash)
      : _queue(queue), _pieces_manager(pieces_manager), _client_id(client_id), _info_hash(info_hash), _conn(nullptr){};

  void start();

 private:
  std::shared_ptr<SharedQueue<Peer>> _queue;
  std::shared_ptr<PiecesManager> _pieces_manager;
  std::string _client_id;
  std::string _info_hash;

  // parameters of current connection
  Peer _peer;                                // current connected peer
  std::string _peer_id;                      // id of connected peer
  ConnectionStatus _status;                  // current connection status
  std::unique_ptr<ConnectionWrapper> _conn;  // wrapper help functions of current connection

  // Start a peer connection, and return whether successful or not,
  // a start of a connection includes:
  //    1. complete handshake
  //    2. send interested
  //    3. reciece bitfiled
  void establish_peer_connection();

  void perform_handshake();
  void receive_bitfield();
  void send_interested();
  void request(Block b);

  // convert between HandShake struct and serialized handshake data
  auto serialize_handshake(HandShake h) -> std::string;
  auto deserialize_handshake(std::string s) -> HandShake;

  auto recieve_message() -> Message;
};
