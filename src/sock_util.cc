#include "sock_util.hh"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>

constexpr size_t CONNECT_TIMEOUT = 3;  // 3 seconds
constexpr size_t READ_TIMEOUT = 3000;  // 3 seconds

auto ConnectionWrapper::connect_to(std::string ip, uint16_t port) -> int {
  auto sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    throw std::runtime_error("Failed to create socket");
  }
  // build sockaddr_in struct
  auto addr = sockaddr_in{.sin_family = AF_INET, .sin_port = htons(port), .sin_zero = {0}};
  if (inet_pton(AF_INET, ip.data(), &addr) <= 0) {
    close(sock);
    throw std::runtime_error("Invalid IP address: " + ip);
  }
  // connect
  // may consume too much time, add timeout functionality later
  if (connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
    close(sock);
    throw std::runtime_error("Failed to connect to socket " + std::to_string(sock));
  }
  return sock;
}

void ConnectionWrapper::send_data(const std::string &data) {
  auto len = send(_sock, data.data(), data.length(), 0);
  if (len < 0) {
    throw std::runtime_error("Failed to Send data on socket " + std::to_string(_sock));
  }
}

auto ConnectionWrapper::recv_data(size_t buf_size) -> std::string {
  if (buf_size == 0) {
    buf_size = htoi(recv_data(4));
  }

  auto reply = std::string{};
  if (buf_size == 0) return reply;

  // code below mainly from https://github.com/ss16118/torrent-client-cpp/blob/main/src/connect.cpp
  // If the buffer size is greater than uint16_t max, a segfault will occur when initializing the buffer (remaining
  // proved)
  if (buf_size > std::numeric_limits<uint16_t>::max()) throw std::runtime_error("Received corrupted data [Received buffer size greater than 2 ^ 16 - 1]");

  char buffer[buf_size];
  // Receives reply from the host
  // Keeps reading from the buffer until all expected bytes are received
  long bytesRead = 0;
  long bytesToRead = buf_size;
  // If not all expected bytes are received within the period of time
  // specified by READ_TIMEOUT, the read process will stop.
  auto startTime = std::chrono::steady_clock::now();
  do {
    auto diff = std::chrono::steady_clock::now() - startTime;
    if (std::chrono::duration<double, std::milli>(diff).count() > READ_TIMEOUT) {
      throw std::runtime_error("Read timeout from socket " + std::to_string(_sock));
    }
    bytesRead = recv(_sock, buffer, buf_size, 0);

    if (bytesRead <= 0) {
      throw std::runtime_error("Failed to receive data from socket " + std::to_string(_sock));
    }
    bytesToRead -= bytesRead;
    reply += std::string(buffer, bytesRead);
  } while (bytesToRead > 0);

  return reply;
}

auto htoi(std::string str) -> size_t {
  auto res = size_t{0};
  for (auto c : str) {
    res += c;
    res <<= 8;
  }
  return res;
}
