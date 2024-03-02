#include "torrent_client.hh"

#include "bencode_parser.hh"
#include "peer_retriever.hh"
#include "util.hh"

TorrentClient::TorrentClient(client_para para) : _para(para) {
  _peer_id = std::string("-CB2024-") + random_number(12);
  auto parser = TorrentParser(para.torrent_file);
  _torrent = parser.get_torrent();
  _retriever = PeerRetriever(_torrent.announce, _torrent.info_hash, _peer_id, _torrent.info.length);
}

void download() {
  //
}
