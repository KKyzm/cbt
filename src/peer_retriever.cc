#include "peer_retriever.hh"

#include <cpr/cpr.h>
#include <fmt/core.h>

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

#include "bencode_parser.hh"

PeerRetriever::PeerRetriever(std::string announce, std::string info_hash, std::string peer_id, size_t file_size)
    : _announce(announce), _info_hash(info_hash), _peer_id(peer_id), _file_size(file_size) {}

std::vector<Peer> PeerRetriever::update_peers(size_t downloaded) {
  cpr::Response r = cpr::Get(
      cpr::Url{_announce}, cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
      cpr::Parameters{
          {"info_hash", _info_hash},
          {"peer_id", _peer_id},
          {"port", "6889"},  // ?
          {"uploaded", "0"},
          {"downloaded", std::to_string(downloaded)},
          {"compact", "1"},
          {"left", std::to_string(_file_size - downloaded)}});

  auto parser = TrackerParser(r.text);
  _interval = parser.get_interval();
  auto raw_peers = parser.get_peers();
  return decode_raw_peers(raw_peers);
}

std::vector<Peer> PeerRetriever::decode_raw_peers(std::string raw_peers) {
  auto res = std::vector<Peer>();
  assert(raw_peers.length() % 6 == 0);
  for (size_t i = 0; i < raw_peers.length(); i += 6) {
    auto peer = Peer();
    auto raw_peer = raw_peers.substr(i, 6);
    // extract ip
    auto ss = std::stringstream();
    ss << std::to_string(static_cast<unsigned char>(raw_peer[0])) << "."
       << std::to_string(static_cast<unsigned char>(raw_peer[1])) << "."
       << std::to_string(static_cast<unsigned char>(raw_peer[2])) << "."
       << std::to_string(static_cast<unsigned char>(raw_peer[3]));
    peer.ip = ss.str();
    // extract port
    auto port = uint16_t{0};
    port += static_cast<uint8_t>(raw_peer[4]);
    port = port << 8;
    port += static_cast<uint8_t>(raw_peer[5]);
    peer.port = port;

    res.push_back(peer);
  }

  return res;
}
