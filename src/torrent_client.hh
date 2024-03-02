#pragma once

#include <string>

#include "bencode_parser.hh"
#include "peer_retriever.hh"

struct client_config {
  std::string torrent_file;
  std::string output_dir;
  std::string log_dir;
};

class TorrentClient {
 public:
  TorrentClient(client_config);

  TorrentClient(const TorrentClient&) = delete;
  TorrentClient(TorrentClient&&) = delete;
  auto operator=(const TorrentClient&) -> TorrentClient& = delete;
  auto operator=(TorrentClient&&) -> TorrentClient& = delete;

  void download();

 private:
  std::string _peer_id;
  client_config _conf;
  Torrent _torrent;

  PeerRetriever _retriever;
};
