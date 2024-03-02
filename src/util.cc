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

auto SHA1(std::string &data) -> std::string {
  CryptoPP::byte digest[CryptoPP::SHA1::DIGESTSIZE] = {0};
  CryptoPP::SHA1().CalculateDigest(digest, reinterpret_cast<CryptoPP::byte *>(data.data()), data.length());
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

auto random_number(const size_t length) -> std::string {
  std::random_device rd;   // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd());  // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(1, 9);
  auto res = std::string();
  for (size_t i = 0; i < length; i++) {
    res += std::to_string(distrib(gen));
  }
  return res;
}
