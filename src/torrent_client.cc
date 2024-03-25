#include "torrent_client.hh"

#include <chrono>
#include <cstddef>
#include <memory>
#include <thread>

#include "bencode_parser.hh"
#include "peer.hh"
#include "peer_connection.hh"
#include "peer_retriever.hh"
#include "pieces_manager.hh"
#include "shared_queue.hh"
#include "util.hh"

const std::string TorrentClient::_client_id = "-CB2024-LTY-20120712";

TorrentClient::TorrentClient(ClientConfig conf)
    : _conf(conf),
      _torrent(TorrentParser(conf.torrent_file).get_torrent()),
      _retriever(_torrent.announce, _torrent.info_hash, _client_id, _torrent.info.length) {}

void TorrentClient::download() {
  _piece_manager =
      std::make_shared<PiecesManager>(_conf.output_dir, _torrent.info.name, _torrent.info.length, _torrent.info.piece_length, _torrent.info.pieces_hash);
  _queue = std::make_shared<SharedQueue<Peer>>();
  update_peers();

  // launch multi-thread peer-connection
  for (auto i = 0; i < _conf.thread_num; i++) {
    auto conn = std::make_shared<PeerConnection>(_queue, _piece_manager, _client_id, _torrent.info_hash);
    auto thrd = std::thread(&PeerConnection::start, conn.get());
    _connections.push_back(conn);
    _thread_pool.push_back(std::move(thrd));
  }

  // update peers periodically until download complete
  auto start_time = std::chrono::steady_clock::now();
  while (!_piece_manager->finished()) {
    auto diff = std::chrono::steady_clock::now() - start_time;
    if (std::chrono::duration<double>(diff).count() > _retriever.get_interval()) {
      start_time = std::chrono::steady_clock::now();
      update_peers();
    }
    std::this_thread::sleep_for(std::chrono::duration<int>(1));
  }

  // wait until all thread return
  wait();
}

void TorrentClient::update_peers() {
  std::unique_lock<std::mutex> mlock(_queue->mutex_);
  auto peers = _retriever.update_peers(_piece_manager->downloaded());
  _queue->clear();
  for (auto &&peer : peers) {
    _queue->push_back(peer);
  }
}

void TorrentClient::wait() {
  for (auto i = 0; i < _conf.thread_num; i++) {
    _queue->push_back({DUMMY_IP, 0});
  }
  for (auto &thrd : _thread_pool) {
    if (thrd.joinable()) thrd.join();
  }
}
