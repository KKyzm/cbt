#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
namespace fs = std::filesystem;

// read and return all content of a file
std::string read_file(const fs::path &path);

std::string SHA1(std::string &data);

// convert string to hex repersenation
std::string to_hex(const std::string data);

// genrate random numbers
std::string random_number(const size_t length);
