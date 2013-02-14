cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -Werror")

include_directories(".")
set(EBML_SOURCES
  "src/strstr_nodes.cpp"
  "src/strstr_io.cpp"
  "src/strstr_registry.cpp"
  "src/strstr_reader.cpp"
  "src/strstr_utils.cpp"
  "src/strstr_writer.cpp"
)
add_library(structstream++ SHARED ${EBML_SOURCES})

set(CATCH_INCLUDE_DIRS "Catch/single_include")
set(TEST_SOURCES
  "tests/encode_varint.cpp"
  "tests/encode_record.cpp"
  "tests/encode_container.cpp"
  "tests/decode_record.cpp"
  "tests/decode_container.cpp"
)

include_directories(${CATCH_INCLUDE_DIRS})
add_executable(run_tests tests/main.cpp ${TEST_SOURCES} )
target_link_libraries(run_tests structstream++)
