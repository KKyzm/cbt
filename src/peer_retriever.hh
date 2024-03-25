#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "peer.hh"
#include "shared_queue.hh"

class PeerRetriever {
 public:
  PeerRetriever() = delete;
  PeerRetriever(std::string announce, std::string info_hash, std::string client_id, size_t file_size);

  auto update_peers(size_t downloaded) -> std::vector<Peer>;

  auto get_interval() -> size_t { return _interval; }

 private:
  std::string _announce;
  std::string _info_hash;
  std::string _client_id;
  size_t _file_size;

  size_t _interval;  // unit: second

  auto decode_raw_peers(std::string raw_peers) -> std::vector<Peer>;
};
