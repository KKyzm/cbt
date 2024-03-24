#pragma once

#include <string>

#include "bencode_parser.hh"
#include "peer_retriever.hh"

struct ClientConfig {
  std::string torrent_file;
  std::string output_dir;
};

class TorrentClient {
 public:
  TorrentClient(ClientConfig);

  TorrentClient(const TorrentClient&) = delete;
  TorrentClient(TorrentClient&&) = delete;
  auto operator=(const TorrentClient&) -> TorrentClient& = delete;
  auto operator=(TorrentClient&&) -> TorrentClient& = delete;

  // start download task
  void download();

 private:
  ClientConfig _conf;
  Torrent _torrent;

  static const std::string _client_id;

  // update peers periodically according to `interval`
  void PeerUpdater();
};
