#include "torrent_client.hh"

#include <chrono>
#include <cstddef>
#include <memory>
#include <thread>

#include "bencode_parser.hh"
#include "peer_retriever.hh"
#include "pieces_manager.hh"
#include "util.hh"

const std::string TorrentClient::_client_id = "-CB2024-LTY-20120712";

TorrentClient::TorrentClient(ClientConfig conf) : _conf(conf) { _torrent = TorrentParser(conf.torrent_file).get_torrent(); }

void TorrentClient::download() {
  auto piece_manager =
      std::make_shared<PiecesManager>(_conf.output_dir, _torrent.info.name, _torrent.info.length, _torrent.info.piece_length, _torrent.info.pieces_hash);
  auto retriever = PeerRetriever(_torrent.announce, _torrent.info_hash, _client_id, _torrent.info.length);
  auto peers = retriever.update_peers(0);

  //
  // TODO: launch multi-thread peer-connection here
  //

  while (!piece_manager->finished()) {
    // FIXME: dont sleep so long
    std::this_thread::sleep_for(std::chrono::duration<size_t>(retriever.get_interval()));
    peers = retriever.update_peers(piece_manager->downloaded());
  }
}
