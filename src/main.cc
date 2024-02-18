#include "bencode/bencode.hpp"

int main(int argc, char **argv) {
  namespace bc = bencode;

  bc::bvalue b = bc::decode_value("l3:fooi2ee");

  // check if the first list element is a string
  if (holds_list(b) && holds_string(b[0])) {
    std::cout << "success" << '\n';
  }

  // type tag based type check, return false
  bc::holds_alternative<bc::btype::dict>(b);

  // access the first element of the list "foo" and move it
  // out of the bvalue into v1
  std::string v1 = bc::get_string(std::move(b[0]));

  // access the second element
  std::size_t v2 = bc::get_integer(b[1]);
}
