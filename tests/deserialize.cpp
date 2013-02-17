#include "catch.hpp"

#include <cstddef>

#include "structstream/serializer.hpp"
#include "structstream/node_primitive.hpp"
#include "structstream/node_blob.hpp"
#include "structstream/reader.hpp"
#include "structstream/writer.hpp"

using namespace StructStream;

TEST_CASE ("deserialize/pod", "Deserialization of a plain-old-data type")
{
    struct pod_t {
        uint32_t v1;
        uint8_t v3;
        double v2;
    };

    NodeHandle pod_root_node = NodeHandleFactory<Container>::create(0x01);
    Container *pod_root = static_cast<Container*>(pod_root_node.get());

    NodeHandle node = NodeHandleFactory<UInt32Record>::create(0x02);
    static_cast<UInt32Record*>(node.get())->set(0x2342dead);
    pod_root->child_add(node);

    node = NodeHandleFactory<UInt32Record>::create(0x04);
    static_cast<UInt32Record*>(node.get())->set(0xffffff12);
    pod_root->child_add(node);

    node = NodeHandleFactory<Float64Record>::create(0x03);
    static_cast<Float64Record*>(node.get())->set(23.42);
    pod_root->child_add(node);

    pod_t pod;

    serializer<
        0x01,
        serialize_block<pod_t,
                        serialize_primitive<0x02, UInt32Record, uint32_t, offsetof(pod_t, v1)>,
                        serialize_primitive<0x04, UInt32Record, uint8_t, offsetof(pod_t, v3)>,
                        serialize_primitive<0x03, Float64Record, double, offsetof(pod_t, v2)>
                        >
        >::deserialize(pod_root, &pod);

    CHECK(pod.v1 == 0x2342dead);
    CHECK(pod.v2 == 23.42);
    CHECK(pod.v3 == 0x12);
}

TEST_CASE ("deserialize/str_callback", "Deserialization of a string")
{
    struct block_t {
        std::string value;

        void set_str(const std::string &ref) {
            value = ref;
        };
    };

    NodeHandle pod_root_node = NodeHandleFactory<Container>::create(0x01);
    Container *pod_root = static_cast<Container*>(pod_root_node.get());

    NodeHandle node = NodeHandleFactory<UTF8Record>::create(0x02);
    static_cast<UTF8Record*>(node.get())->set("Hello World!");
    pod_root->child_add(node);

    block_t block;

    serializer<
        0x01,
        serialize_block<block_t,
                        deserialize_string<0x02, UTF8Record, block_t, &block_t::set_str>
                        >
        >::deserialize(pod_root, &block);

    CHECK(block.value == "Hello World!");

}

TEST_CASE ("deserialize/blob_callback", "Deserialization of a blob")
{
    struct block_t {
    public:
        block_t(): _buf(), _len(0) {};
        virtual ~block_t() { if (_buf) { free(_buf); } };
    private:
        char* _buf;
        intptr_t _len;
    public:
        void set_str(const char* value, const intptr_t len) {
            _buf = (char*)realloc(_buf, len);
            memcpy(_buf, value, len);
        };

        const char* get_str() const {
            return _buf;
        };
    };

    static const char* text = "Hello World!";

    NodeHandle pod_root_node = NodeHandleFactory<Container>::create(0x01);
    Container *pod_root = static_cast<Container*>(pod_root_node.get());

    NodeHandle node = NodeHandleFactory<UTF8Record>::create(0x02);
    static_cast<UTF8Record*>(node.get())->set(text);
    pod_root->child_add(node);

    block_t block;

    serializer<
        0x01,
        serialize_block<block_t,
                        deserialize_buffer<0x02, UTF8Record, block_t, char, &block_t::set_str>
                        >
        >::deserialize(pod_root, &block);

    CHECK(strcmp(block.get_str(), text) == 0);
}
