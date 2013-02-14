#include "catch.hpp"

#include <cstring>

#include "include/strstr_io.hpp"
#include "include/strstr_utils.hpp"

using namespace StructStream;

TEST_CASE ("encode/varuint/1", "Encode a 1 byte long varuint")
{
    static const uint8_t expected[] = {
        0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x7f);

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/2", "Encode a 2 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x7f, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x3fff);

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/3", "Encode a 3 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x3f, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x1fffff);

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/4", "Encode a 4 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x1f, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x0fffffff);

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/5", "Encode a 5 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x0f, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x07ffffffff);

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/6", "Encode a 6 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x07, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x03ffffffffff);

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/7", "Encode a 7 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x01ffffffffffff);

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/8", "Encode a 8 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0xffffffffffffff);

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}
