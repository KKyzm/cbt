#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "bencode_parser.hh"
#include "peer_connection.hh"
#include "peer_retriever.hh"
#include "pieces_manager.hh"
#include "shared_queue.hh"

struct ClientConfig {
  std::string torrent_file;
  std::string output_dir;
  size_t thread_num{4};
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
  std::shared_ptr<PiecesManager> _piece_manager;
  std::shared_ptr<SharedQueue<Peer>> _queue;
  PeerRetriever _retriever;

  std::vector<std::shared_ptr<PeerConnection>> _connections;
  std::vector<std::thread> _thread_pool;

  static const std::string _client_id;

  // update peers periodically according to `interval`
  void update_peers();
  void wait();
};
