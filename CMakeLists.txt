cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -Werror")

include_directories(".")
set(EBML_SOURCES
  "src/strstr_nodes.cpp"
  "src/strstr_io.cpp"
  "src/strstr_registry.cpp"
  "src/strstr_reader.cpp"
)
add_library(structstream++ SHARED ${EBML_SOURCES})
