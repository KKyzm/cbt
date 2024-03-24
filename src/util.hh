#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
namespace fs = std::filesystem;

// read and return all content of a file
auto read_file(const fs::path &path) -> std::string;

auto SHA1(const std::string &data) -> std::string;

// convert string to hex repersenation
auto to_hex(const std::string data) -> std::string;
