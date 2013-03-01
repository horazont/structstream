cmake_minimum_required(VERSION 2.6)
# set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -Werror")

include_directories(".")
set(STRUCTSTREAM_SOURCES
  "src/node_base.cpp"
  "src/node_container.cpp"
  "src/node_primitive.cpp"
  "src/node_blob.cpp"
  "src/io_base.cpp"
  "src/io_memory.cpp"
  "src/io_std.cpp"
  "src/io_hash.cpp"
  "src/io.cpp"
  "src/registry.cpp"
  "src/utils.cpp"
  "src/deserializer.cpp"
  "src/serializer.cpp"
  "src/streaming_base.cpp"
  "src/streaming_utils.cpp"
  "src/streaming_tree.cpp"
  "src/streaming_bitstream.cpp"
  "src/streaming.cpp"
  "src/hashing_base.cpp"
  "src/hashing_gnutls.cpp"
  "src/hashing.cpp"
  "src/iterators.cpp"
)

set(DEPS)

find_package(GnuTLS)

if (GNUTLS_FOUND)
  add_definitions(-DWITH_GNUTLS)
  add_definitions(${GNUTLS_DEFINITIONS})
  include_directories(${GNUTLS_INCLUDE_DIR})
  list(APPEND DEPS ${GNUTLS_LIBRARIES})
endif (GNUTLS_FOUND)

add_library(structstream++ ${STRUCTSTREAM_SOURCES})
target_link_libraries(structstream++ ${DEPS})

set(CATCH_INCLUDE_DIRS "Catch/single_include")
set(TEST_SOURCES
  "tests/encode_varint.cpp"
  "tests/decode_varint.cpp"
  "tests/encode_record.cpp"
  "tests/encode_container.cpp"
  "tests/decode_record.cpp"
  "tests/decode_container.cpp"
  "tests/decode_forgiving.cpp"
  "tests/decode_failures.cpp"
  "tests/deserialize.cpp"
  "tests/serialize.cpp"
  "tests/hashing.cpp"
  "tests/iterators.cpp"
)

include_directories(${CATCH_INCLUDE_DIRS})
add_executable(run_tests tests/main.cpp ${TEST_SOURCES} )
target_link_libraries(run_tests structstream++)

add_executable(examples/person EXCLUDE_FROM_ALL examples/person.cpp)
target_link_libraries(examples/person structstream++)