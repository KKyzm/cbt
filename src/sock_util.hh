#pragma once

#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <string>

const std::string DUMMY_IP = "0.0.0.0";

class ConnectionWrapper {
 public:
  ConnectionWrapper(std::string ip, uint16_t port) { _sock = connect_to(ip, port); }
  ~ConnectionWrapper() { close(_sock); }

  void send_data(const std::string &data);
  auto recv_data(size_t buf_size = 0) -> std::string;

 private:
  int _sock;

  auto connect_to(std::string ip, uint16_t port) -> int;
};

auto htoi(std::string str) -> size_t;
