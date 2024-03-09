#pragma once

#include <cstddef>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

struct Block {
  size_t piece_idx = 0;
  size_t offset = 0;
  size_t length = 0;
};

class PiecesManager {
 public:
  explicit PiecesManager(std::string file, size_t file_length, size_t piece_length, std::shared_ptr<std::vector<std::string>> piece_hashs);

  // register peer with its bitfield
  void peer_add(std::string peer_id, std::string bitfield);

  // unregister peer
  void peer_del(std::string peer_id);

  // update peer's bitfield
  void peer_update(std::string peer_id, size_t piece_idx);

  // get block that should be requested from peer
  auto next_request(std::string peer_id) -> Block;

  // submit data recieved from peer
  void block_recieved(std::string peer_id, size_t piece_idx, size_t block_offset, const std::string& data);

  // whether all pieces are correctly recieved
  auto finished() -> bool { return _missing.empty() && _requesting.empty(); }

 private:
  size_t _file_length;
  size_t _piece_length;
  std::shared_ptr<std::vector<std::string>> _piece_hashs;
  std::ofstream _file;

  std::mutex mtx{};

  // bitfield of each registered peer
  std::map<std::string, std::set<size_t>> _bitfields{};
  // blocks that have not been downloaded from peers
  std::map<size_t, std::queue<Block>> _missing{};
  // blocks that being requested by connections
  std::map<std::string, std::vector<Block>> _requesting{};
  // blocks that have been downloaded
  std::map<size_t, std::map<size_t, std::string>> _reveived{};
  // number of blocks of each piece
  std::map<size_t, size_t> _num_blocks{};

  // return all blocks info of piece
  auto blocks_of(size_t piece_idx) -> std::queue<Block>;
  auto blocks_of(size_t piece_idx, size_t piece_length) -> std::queue<Block>;

  void piece_recieved(size_t piece_idx);
};
