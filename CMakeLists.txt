cmake_minimum_required(VERSION 3.24)
project(PTMK)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
        pqxx
        URL      https://github.com/jtv/libpqxx/archive/refs/tags/7.7.3.tar.gz
)

FetchContent_MakeAvailable(pqxx)

add_executable(PTMK main.cpp)

target_link_libraries(
        PTMK
        pqxx
)