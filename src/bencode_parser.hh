#pragma once

#include <bencode/bencode.hpp>
#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "bencode/detail/bvalue/accessors.hpp"

namespace bc = bencode;

struct TorrentInfo {
  size_t piece_length;
  std::vector<std::string> pieces_hash;
  std::string name;
  size_t length;
};

struct Torrent {
  std::string announce;
  std::string info_hash;
  TorrentInfo info;
};

/*
 * parse .torrent file
 * */
class TorrentParser {
 public:
  explicit TorrentParser(std::string torrent_file);

  // get non-optional fields
  auto get_torrent() -> Torrent;

  // get optional fields
  auto get_author() -> std::optional<std::string>;
  auto get_creation_date() -> std::optional<std::string>;
  auto get_comment() -> std::optional<std::string>;

 private:
  std::string _torrent_file;
  bc::bvalue _bc_data;
};

/*
 * parse tracker's response
 * */
class TrackerParser {
 public:
  explicit TrackerParser(std::string text) { _bc_data = bc::decode_value(text); }

  auto get_interval() -> size_t {
    assert(bc::holds_dict(_bc_data));
    auto interval = bc::get_integer(_bc_data.at("interval"));
    return static_cast<size_t>(interval);
  }

  auto get_peers() -> std::string {
    assert(bc::holds_dict(_bc_data));
    return bc::get_string(_bc_data.at("peers"));
  }

 private:
  bc::bvalue _bc_data;
};
