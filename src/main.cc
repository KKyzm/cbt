#include <cpr/cpr.h>
#include <fmt/core.h>

#include "logger.h"
#include "torrent_client.hh"
#include "util.hh"

auto main(int argc, char **argv) -> int {
  init_logger();  // program exit if logger initialization fails

  auto para = ClientConfig{
      .torrent_file = "/home/yaniru/workshop/projects/cbt/res/debian-12.4.0-amd64-netinst.iso.torrent",
      .output_dir = ".",
  };

  auto client = TorrentClient(para);

  client.download();

  return 0;
}
