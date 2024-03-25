include(FetchContent)

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(cbt CXX)

# build cbt_lib
file(GLOB_RECURSE lib_srcs CONFIGURE_DEPENDS
  # src/main.cc
  src/torrent_client.cc
  src/peer_retriever.cc
  src/bencode_parser.cc
  src/util.cc
  src/peer_connection.cc
  src/pieces_manager.cc
  src/sock_util.cc
)

add_library(cbt_lib ${lib_srcs})
target_include_directories(cbt_lib PUBLIC src)

# link bencode to cbt
FetchContent_Declare(bencode
  GIT_REPOSITORY https://github.com/fbdtemme/bencode.git
  GIT_TAG "main")
FetchContent_MakeAvailable(bencode)
target_link_libraries(cbt_lib PUBLIC bencode::bencode)
# link cpr to cbt_lib
find_package(cpr CONFIG REQUIRED)
target_link_libraries(cbt_lib PUBLIC cpr::cpr)
# link cryptopp to cbt_lib
find_package(cryptopp CONFIG REQUIRED)
target_link_libraries(cbt_lib PUBLIC cryptopp::cryptopp)
# link spdlog to cbt_lib
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(cbt_lib PUBLIC spdlog::spdlog)

add_executable(cbt src/main.cc)
target_link_libraries(cbt PRIVATE cbt_lib)

add_subdirectory(test)
