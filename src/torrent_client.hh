#pragma once

#include <string>

#include "bencode_parser.hh"
#include "peer_retriever.hh"

struct client_para {
  std::string torrent_file;
  std::string output_dir;
  std::string log_dir;
};

class TorrentClient {
 public:
  TorrentClient(client_para para);

  TorrentClient(const TorrentClient&) = delete;
  TorrentClient(TorrentClient&&) = delete;
  TorrentClient& operator=(const TorrentClient&) = delete;
  TorrentClient& operator=(TorrentClient&&) = delete;

  void download();

 private:
  std::string _peer_id;
  client_para _para;
  Torrent _torrent;

  PeerRetriever _retriever;
};
