#pragma once

#include <cstdint>
#include <string>

const std::string DUMMY_IP = "0.0.0.0";

struct Peer {
  std::string ip;
  uint16_t port;
};
