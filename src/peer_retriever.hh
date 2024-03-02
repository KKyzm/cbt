#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct Peer {
  std::string ip;
  uint16_t port;
};

class PeerRetriever {
 public:
  PeerRetriever() = default;
  PeerRetriever(std::string announce, std::string info_hash, std::string peer_id, size_t file_size);

  std::vector<Peer> update_peers(size_t downloaded);

  size_t get_interval() { return _interval; }

 private:
  std::string _announce;
  std::string _info_hash;
  std::string _peer_id;
  size_t _file_size;

  size_t _interval;  // unit: second

  std::vector<Peer> decode_raw_peers(std::string raw_peers);
};
