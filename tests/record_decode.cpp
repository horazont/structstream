#include "catch.hpp"

#include "include/strstr_reader.hpp"

using namespace StructStream;

TEST_CASE ("decode/records/uint32", "Test decode of a uint32 record")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78
    };

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
    static const uint8_t data[] = {
        (uint8_t)(RT_UINT64) | 0x80, uint8_t(0x01) | 0x80,
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF
    };

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
    static const uint8_t data[] = {
        (uint8_t)(RT_INT32) | 0x80, uint8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78
    };

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
    static const uint8_t data[] = {
        (uint8_t)(RT_INT64) | 0x80, uint8_t(0x01) | 0x80,
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    Int64Record *rec = dynamic_cast<Int64Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == (int64_t)0xFFDEBC9A78563412);
}

TEST_CASE ("decode/container/empty", "Test decode of an empty container with explicit length")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE) | 0x80, uint8_t(0x00),
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);
    reader.read_all();

    Container *cont = dynamic_cast<Container*>(node.get());
    REQUIRE(cont != 0);
    REQUIRE(cont->children_begin() == cont->children_end());
}

TEST_CASE ("decode/container/empty_implicit", "Test decode of an empty container without explicit length")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(0x00) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);
    reader.read_all();

    Container *cont = dynamic_cast<Container*>(node.get());
    REQUIRE(cont != 0);
    REQUIRE(cont->children_begin() == cont->children_end());
}

TEST_CASE ("decode/container/with_child", "Test decode of an empty container")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80,
        uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE) | 0x80, uint8_t(0x01) | 0x80,
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x02) | 0x80, 0x12, 0x34, 0x56, 0x78,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);
    reader.read_all();

    Container *cont = dynamic_cast<Container*>(node.get());
    REQUIRE(cont != 0);

    NodeHandle child = *(cont->children_begin());
    REQUIRE(child.get() != 0);

    UInt32Record *rec = dynamic_cast<UInt32Record*>(child.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == 0x78563412);
}

TEST_CASE ("decode/container/complex", "Complex nesting structure")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80,  // start container
        uint8_t(0x01) | 0x80,  // with id 0x01
        uint8_t(0x00),  // and no other information (i.e. indefinite length)

        // add a uint32 node with id 0x02 and value 0x11111111
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x02) | 0x80, 0x11, 0x11, 0x11, 0x11,

        // add a nested container
        (uint8_t)(RT_CONTAINER) | 0x80,
        uint8_t(0x01) | 0x80, // with id 0x01
        uint8_t(CF_WITH_SIZE) | 0x80, uint8_t(0x01) | 0x80, // and length 1
        // add a uint32 node with id 0x03 and value 0x22222222
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x03) | 0x80, 0x22, 0x22, 0x22, 0x22,
        // ~ implicit exit of nested container, length constraint reached

        // add a uint32 node with id 0x04 and valeu 0x33333333
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x04) | 0x80, 0x33, 0x33, 0x33, 0x33,

        // end our explicit outer container with indefinite length
        (uint8_t)(RT_END_OF_CHILDREN) | 0x80,

        // end the implicit global container
        (uint8_t)(RT_END_OF_CHILDREN) | 0x80
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new MemoryIO(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    reader.read_all();

    Container *cont1 = dynamic_cast<Container*>(node.get());
    REQUIRE(cont1 != 0);

    NodeVector::iterator children = cont1->children_begin();

    UInt32Record *rec = dynamic_cast<UInt32Record*>((*children).get());
    REQUIRE(rec != 0);
    REQUIRE(rec->id() == 0x02);
    REQUIRE(rec->get() == 0x11111111);

    children += 1;

    Container *cont2 = dynamic_cast<Container*>((*children).get());
    REQUIRE(cont2 != 0);
    REQUIRE(cont2->id() == 0x01);

    rec = dynamic_cast<UInt32Record*>((*(cont2->children_begin())).get());
    REQUIRE(rec != 0);
    REQUIRE(rec->id() == 0x03);
    REQUIRE(rec->get() == 0x22222222);

    children += 1;

    rec = dynamic_cast<UInt32Record*>((*children).get());
    REQUIRE(rec != 0);
    REQUIRE(rec->id() == 0x04);
    REQUIRE(rec->get() == 0x33333333);

    children += 1;
    REQUIRE(children == cont1->children_end());
}
