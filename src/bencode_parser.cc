#include "bencode_parser.hh"

#include <chrono>
#include <cstddef>
#include <fstream>
#include <stdexcept>

#include "bencode/detail/bvalue/accessors.hpp"
#include "bencode/detail/decode_value.hpp"
#include "util.hh"

TorrentParser::TorrentParser(std::string torrent_file) : _torrent_file(torrent_file) {
  auto content = read_file(torrent_file);
  _bc_data = bc::decode_value(content);
}

auto TorrentParser::get_torrent() -> Torrent {
  assert(bc::holds_dict(_bc_data));
  auto torrent = Torrent();
  torrent.announce = bc::get_string(_bc_data.at("announce"));

  auto bc_info = _bc_data.at("info");
  assert(bc::holds_dict(bc_info));
  torrent.info.length = bc::get_integer(bc_info.at("length"));
  torrent.info.piece_length = bc::get_integer(bc_info.at("piece length"));
  torrent.info.name = bc::get_string(bc_info.at("name"));
  auto pieces = bc::get_string(bc_info.at("pieces"));
  assert(pieces.length() % 20 == 0);
  for (size_t i = 0; i < pieces.length(); i += 20) {
    torrent.info.pieces.push_back(pieces.substr(i, 20));
  }

  auto info_str = bc::encode(bc_info);
  torrent.info_hash = SHA1(info_str);

  return torrent;
}

auto TorrentParser::get_author() -> std::optional<std::string> {
  if (!_bc_data.contains("created by")) {
    return {};
  }
  return bc::get_string(_bc_data.at("created by"));
}

auto TorrentParser::get_comment() -> std::optional<std::string> {
  if (!_bc_data.contains("comment")) {
    return {};
  }
  return bc::get_string(_bc_data.at("comment"));
}

auto TorrentParser::get_creation_date() -> std::optional<std::string> {
  if (!_bc_data.contains("creation date")) {
    return {};
  }
  auto creation_timestamp = bc::get_integer(_bc_data.at("creation date"));
  auto creation_date = std::chrono::sys_seconds{std::chrono::seconds(creation_timestamp)};
  auto ss = std::stringstream();
  ss << creation_date;
  return ss.str();
}
