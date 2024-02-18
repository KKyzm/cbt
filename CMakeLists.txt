include(FetchContent)

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(cbt CXX)

FetchContent_Declare(bencode
  GIT_REPOSITORY https://github.com/fbdtemme/bencode.git
  GIT_TAG "main")

FetchContent_MakeAvailable(bencode)

add_executable(cbt)

add_subdirectory(src)

target_link_libraries(cbt_lib PRIVATE bencode::bencode)

target_link_libraries(cbt cbt_lib)
