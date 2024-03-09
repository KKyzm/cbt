#include "pieces_manager.hh"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <sstream>
#include <vector>

#include "util.hh"

constexpr size_t BLOCK_SIZE = 1 << 14;

PiecesManager::PiecesManager(std::string file, size_t file_length, size_t piece_length, std::shared_ptr<std::vector<std::string>> piece_hashs)
    : _file(file, std::ios::out | std::ios::binary), _file_length(file_length), _piece_length(piece_length), _piece_hashs(piece_hashs) {
  // initialize _missing with all pieces' blocks

  //
  // for example, if we have a file of size 34, piece_length equals 10, and block_length equals 3,
  // then _missing should contain...
  //     ---------------------------------------
  //     |    10    |    10    |    10    | 4  |
  //     ---------------------------------------
  //        piece0     piece1     piece2   piece4 (fragment)
  //          ^          ^          ^          ^
  // ------------  ------------  ------------  ------
  // | 3| 3| 3|1|  | 3| 3| 3|1|  | 3| 3| 3|1|  | 3|1|
  // ------------  ------------  ------------  ------
  //  blocks0-3     blocks0-3     blocks0-3   blocks0-1
  //
  for (size_t idx = 0; idx < std::ceil(static_cast<double>(file_length) / piece_length); idx++) {
    _missing.insert({idx, blocks_of(idx)});
  }
}

void PiecesManager::peer_add(std::string peer_id, std::string bitfield) {
  mtx.lock();
  for (size_t i = 0; i < bitfield.length(); i++) {
    assert(bitfield[i] == '0' || bitfield[i] == '1');
    if (bitfield[i] == '1') {
      _bitfields[peer_id].insert(i);
    }
  }
  mtx.unlock();
}

void PiecesManager::peer_del(std::string peer_id) {
  if (peer_id.empty()) {
    return;
  }
  mtx.lock();
  _bitfields.erase(peer_id);
  // move requested blocks from this peer from _requesting to _missing
  for (auto &block : _requesting[peer_id]) {
    _missing[block.piece_idx].push(block);
  }
  _requesting.erase(peer_id);
}

void PiecesManager::peer_update(std::string peer_id, size_t piece_idx) {
  mtx.lock();
  if (_bitfields.contains(peer_id)) {
    _bitfields[peer_id].insert(piece_idx);
  }
  mtx.unlock();
}

auto PiecesManager::next_request(std::string peer_id) -> Block {
  mtx.lock();
  auto bitfield = _bitfields[peer_id];
  // find first common piece and return one block
  for (auto idx : bitfield) {
    if (!_missing.contains(idx)) {
      continue;
    }
    auto request = _missing[idx].front();
    _missing[idx].pop();
    if (_missing[idx].empty()) {
      _missing.erase(idx);
    }
    _requesting[peer_id].push_back(request);
    mtx.unlock();
    return request;
  }
  mtx.unlock();
  return {};
}

void PiecesManager::block_recieved(std::string peer_id, size_t piece_idx, size_t block_offset, const std::string &data) {
  mtx.lock();
  // remove recieved block from _requesting
  auto erased = std::erase_if(_requesting[peer_id], [&](Block &block) -> bool {
    return block.piece_idx == piece_idx && block.offset == block_offset && block.length == data.length();
  });
  assert(erased == 1);

  // insert recieved block into _reveived
  auto blocks = _reveived[piece_idx];
  auto it = blocks.begin();
  for (; it != blocks.end(); it++) {
    if (it->first > block_offset) {
      break;  // find position to insert
    }
  }
  blocks.insert(it, {block_offset, data});

  // if all blocks of this piece is received, check and write it into file
  if (blocks.size() == _num_blocks[piece_idx]) {
    piece_recieved(piece_idx);
  }
  mtx.unlock();
}

void PiecesManager::piece_recieved(size_t piece_idx) {
  // put blocks together
  auto block_buf = std::stringstream{};
  auto offset_cur = size_t{0};  // initial block offset
  auto blocks = _reveived[piece_idx];
  for (auto [offset, data] : blocks) {
    assert(offset_cur == offset);
    block_buf << data;
    offset_cur += offset;
  }

  // validation
  if (SHA1(block_buf.str()) != _piece_hashs->at(piece_idx)) {
    // validation fails, give blocks back to _missing
    _missing[piece_idx] = blocks_of(piece_idx);
    return;
  }
  // write validated piece into file
  _file.seekp(piece_idx * _piece_length);
  _file << block_buf.str();
}

auto PiecesManager::blocks_of(size_t piece_idx) -> std::queue<Block> {
  static auto normal_piece_num = _file_length / _piece_length;     // number of pieces that has normal size (piece_length)
  static auto fragment_piece_size = _file_length % _piece_length;  // size of last piece
  assert(fragment_piece_size > 0);
  if (piece_idx < normal_piece_num) {
    return blocks_of(piece_idx, _piece_length);
  } else {
    return blocks_of(piece_idx, fragment_piece_size);
  }
}

auto PiecesManager::blocks_of(size_t piece_idx, size_t piece_length) -> std::queue<Block> {
  auto normal_block_num = piece_length / BLOCK_SIZE;
  auto fragment_block_size = piece_length % BLOCK_SIZE;
  auto blocks = std::queue<Block>{};
  auto idx = size_t{0};
  for (; idx < normal_block_num; idx++) {
    blocks.push({.piece_idx = piece_idx, .offset = idx * BLOCK_SIZE, .length = BLOCK_SIZE});
  }
  if (fragment_block_size > 0) {
    blocks.push({.piece_idx = piece_idx, .offset = idx * BLOCK_SIZE, .length = fragment_block_size});
  }
  // record number of blocks of this piece
  _num_blocks[piece_idx] = blocks.size();
  return blocks;
}
