include(FetchContent)

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(cbt CXX)

# fetch bencode
FetchContent_Declare(bencode
  GIT_REPOSITORY https://github.com/fbdtemme/bencode.git
  GIT_TAG "main")

FetchContent_MakeAvailable(bencode)

# build cbt_lib
file(GLOB_RECURSE lib_srcs CONFIGURE_DEPENDS
  src/main.cc
  src/torrent_client.cc
  src/peer_retriever.cc
  src/bencode_parser.cc
  src/util.cc
)
add_executable(cbt ${lib_srcs})
# link bencode to cbt
target_link_libraries(cbt PUBLIC bencode::bencode)
target_include_directories(cbt PUBLIC src/)
# link cryptopp to cbt
find_package(cryptopp CONFIG REQUIRED)
target_link_libraries(cbt PRIVATE cryptopp::cryptopp)
# link cpr to cbt
find_package(cpr CONFIG REQUIRED)
target_link_libraries(cbt PRIVATE cpr::cpr)
# link spdlog to cbt
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(cbt PRIVATE spdlog::spdlog)
