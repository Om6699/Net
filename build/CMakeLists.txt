cmake_minimum_required(VERSION 2.8)
project(Net)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

include_directories("../Service")

aux_source_directory("../" SRC)
aux_source_directory("../Service" SRC)

add_executable(Net ${SRC})