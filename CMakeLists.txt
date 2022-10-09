cmake_minimum_required(VERSION 3.23)
project(server)

set(CMAKE_CXX_STANDARD 17)

add_executable(server main.cpp include/Protocol.hpp src/ServerPart.hpp src/Message.hpp src/MessageMaker.hpp src/MessageMaker.cpp src/ClientPart.hpp)
