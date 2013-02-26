cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -Werror")

include_directories(".")
set(STRUCTSTREAM_SOURCES
  "src/node_base.cpp"
  "src/node_container.cpp"
  "src/node_primitive.cpp"
  "src/node_blob.cpp"
  "src/io.cpp"
  "src/registry.cpp"
  "src/reader.cpp"
  "src/utils.cpp"
  "src/writer.cpp"
  "src/deserializer.cpp"
  "src/serializer.cpp"
)

set(DEPS)

find_package(GnuTLS)

if (DEFINED GNUTLS_FOUND)
  add_definitions(-DWITH_GNUTLS)
  add_definitions(${GNUTLS_DEFINITIONS})
  include_directories(${GNUTLS_INCLUDE_DIR})
  list(APPEND DEPS ${GNUTLS_LIBRARIES})
  list(APPEND STRUCTSTREAM_SOURCES "src/hashing_gnutls.cpp")
endif (DEFINED GNUTLS_FOUND)

add_library(structstream++ ${STRUCTSTREAM_SOURCES})

set(CATCH_INCLUDE_DIRS "Catch/single_include")
set(TEST_SOURCES
  "tests/encode_varint.cpp"
  "tests/encode_record.cpp"
  "tests/encode_container.cpp"
  "tests/decode_record.cpp"
  "tests/decode_container.cpp"
  "tests/deserialize.cpp"
  "tests/serialize.cpp"
)

include_directories(${CATCH_INCLUDE_DIRS})
add_executable(run_tests tests/main.cpp ${TEST_SOURCES} )
target_link_libraries(run_tests structstream++)
