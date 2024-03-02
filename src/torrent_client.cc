#include "torrent_client.hh"

#include "bencode_parser.hh"
#include "peer_retriever.hh"
#include "util.hh"

TorrentClient::TorrentClient(client_config conf) : _conf(conf) {
  _peer_id = std::string("-CB2024-") + random_number(12);
  _torrent = TorrentParser(conf.torrent_file).get_torrent();
  _retriever = PeerRetriever(_torrent.announce, _torrent.info_hash, _peer_id, _torrent.info.length);
}

void download() {
  //
}
