#include <cpr/cpr.h>

#include <iostream>

#include "peer_retriever.hh"
#include "torrent_client.hh"

using namespace std;

auto main(int argc, char* argv[]) -> int {
  const std::string client_id = "-CB2024-LTY-20120712";
  auto torrent = TorrentParser("/home/yaniru/workshop/projects/cbt/res/debian-12.4.0-amd64-netinst.iso.torrent").get_torrent();

  cpr::Response r = cpr::Get(
      cpr::Url{torrent.announce}, cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
      cpr::Parameters{
          {"info_hash", torrent.info_hash},
          {"peer_id", client_id},
          {"port", "8080"},  // ?
          {"uploaded", "0"},
          {"downloaded", "0"},
          {"left", std::to_string(torrent.info.length)},
          {"compact", "1"},
      });
  std::cout << "Status code: " << r.status_code << '\n';
  std::cout << "Header:\n";
  for (const std::pair<const std::basic_string<char>, std::basic_string<char>>& kv : r.header) {
    std::cout << '\t' << kv.first << ':' << kv.second << '\n';
  }
  std::cout << "Text: " << r.text << '\n';

  return 0;
}
