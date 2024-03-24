#include "util.hh"

#include <cryptopp/sha.h>

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>

auto read_file(const fs::path &path) -> std::string {
  auto ifs = std::ifstream(path, std::ios::binary | std::ios::ate);
  auto pos = ifs.tellg();
  auto content = std::string(pos, '0');
  ifs.seekg(0, std::ios::beg);
  ifs.read(content.data(), pos);
  return content;
}

auto SHA1(const std::string &data) -> std::string {
  CryptoPP::byte digest[CryptoPP::SHA1::DIGESTSIZE] = {0};
  CryptoPP::SHA1().CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte *>(data.data()), data.length());
  return std::string(reinterpret_cast<char *>(digest), CryptoPP::SHA1::DIGESTSIZE);
}

auto to_hex(const std::string data) -> std::string {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (auto c : data) {
    ss << std::setw(2) << static_cast<uint8_t>(c);
  }
  return ss.str();
}
