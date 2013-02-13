#include "catch.hpp"

#include "include/strstr_reader.hpp"

using namespace StructStream;

TEST_CASE ("decode/records/uint32", "Test decode of a uint32 record")
{
    static const uint8_t data[] = {(uint8_t)(RT_UINT32) | 0x80, uint8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78};

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    UInt32Record *rec = dynamic_cast<UInt32Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == 0x78563412);
}

TEST_CASE ("decode/records/uint64", "Test decode of a uint64 record")
{
    static const uint8_t data[] = {(uint8_t)(RT_UINT64) | 0x80, uint8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF};

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    UInt64Record *rec = dynamic_cast<UInt64Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == 0xFFDEBC9A78563412);
}

TEST_CASE ("decode/records/int32", "Test decode of a int32 record")
{
    static const uint8_t data[] = {(uint8_t)(RT_INT32) | 0x80, int8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78};

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    Int32Record *rec = dynamic_cast<Int32Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == (int32_t)0x78563412);
}

TEST_CASE ("decode/records/int64", "Test decode of a int64 record")
{
    static const uint8_t data[] = {(uint8_t)(RT_INT64) | 0x80, int8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF};

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    Int64Record *rec = dynamic_cast<Int64Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == (int64_t)0xFFDEBC9A78563412);
}
