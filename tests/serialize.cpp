/**********************************************************************
File name: deserialize.cpp
This file is part of: structstream++

LICENSE

The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations under
the License.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public license (the  "GPL License"), in which case  the
provisions of GPL License are applicable instead of those above.

FEEDBACK & QUESTIONS

For feedback and questions about structstream++ please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#include "catch.hpp"

#include <cstddef>

#include "structstream/serialize.hpp"
#include "structstream/node_primitive.hpp"
#include "structstream/node_blob.hpp"
#include "structstream/streaming_tree.hpp"

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

    typedef struct_decl<
        Container,
        0x01,
        struct_members<
            member_raw<UInt32Record, 0x02, pod_t, uint32_t, offsetof(pod_t, v1)>,
            member<UInt32Record, 0x04, pod_t, uint8_t, &pod_t::v3>,
            member_raw<Float64Record, 0x03, pod_t, double, offsetof(pod_t, v2)>
            >
        > deserializer;

    FromTree(deserialize<deserializer>(pod),
             std::dynamic_pointer_cast<Container>(pod_root_node));

    CHECK(pod.v1 == 0x2342dead);
    CHECK(pod.v2 == 23.42);
    CHECK(pod.v3 == 0x12);
}

TEST_CASE ("deserialize/str/member_ptr", "Deserialization of a string")
{
    struct block_t {
        std::string value;
    };

    NodeHandle pod_root_node = NodeHandleFactory<Container>::create(0x01);
    Container *pod_root = static_cast<Container*>(pod_root_node.get());

    NodeHandle node = NodeHandleFactory<UTF8Record>::create(0x02);
    static_cast<UTF8Record*>(node.get())->set("Hello World!");
    pod_root->child_add(node);

    block_t block;

    typedef struct_decl<
        Container,
        0x01,
        struct_members<
            member_string<UTF8Record, 0x02, block_t, &block_t::value>
            >
        > deserializer;

    FromTree(deserialize<deserializer>(block),
             std::dynamic_pointer_cast<Container>(pod_root_node));

    CHECK(block.value == "Hello World!");
}

TEST_CASE ("deserialize/str/callback", "Deserialization of a string")
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

    typedef struct_decl<
        Container,
        0x01,
        struct_members<
            member_string_cb<UTF8Record, 0x02, block_t, &block_t::set_str>
            >
        > deserializer;

    FromTree(deserialize<deserializer>(block),
             std::dynamic_pointer_cast<Container>(pod_root_node));

    CHECK(block.value == "Hello World!");

}

TEST_CASE ("deserialize/blob/callback", "Deserialization of a blob")
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

    typedef struct_decl<
        Container,
        0x01,
        struct_members<
            member_cb_len<UTF8Record, 0x02, block_t, &block_t::set_str>
            >
        > deserializer;

    FromTree(deserialize<deserializer>(block),
             std::dynamic_pointer_cast<Container>(pod_root_node));

    CHECK(strcmp(block.get_str(), text) == 0);
}

TEST_CASE ("deserialize/iterator/simple", "Deserialization of an integer array")
{
    static const uint32_t values[] = {1, 2, 3, 4, 5};

    NodeHandle pod_root_node = NodeHandleFactory<Container>::create(0x01);
    Container *pod_root = static_cast<Container*>(pod_root_node.get());

    for (auto &value: values)
    {
        NodeHandle rec_node = NodeHandleFactory<UInt32Record>::create(0x02);
        UInt32Record *rec = static_cast<UInt32Record*>(rec_node.get());
        rec->set(value);
        pod_root->child_add(rec_node);
    }

    std::vector<uint32_t> dest;

    typedef iterator<
        value<UInt32Record, 0x02, uint32_t>,
        std::back_insert_iterator<decltype(dest)>,
        uint32_t
        > deserializer;

    FromTree(deserialize<deserializer>(std::back_inserter(dest)),
             std::dynamic_pointer_cast<Container>(pod_root_node));

    REQUIRE((sizeof(values) / sizeof(uint32_t)) == dest.size());

    int value_idx = 0;
    for (auto it = dest.begin();
         it != dest.end();
         it++, value_idx++)
    {
        CHECK(values[value_idx] == *it);
    }
}

TEST_CASE ("deserialize/container/simple", "Deserialization of an integer array")
{
    static const uint32_t values[] = {1, 2, 3, 4, 5};

    NodeHandle pod_root_node = NodeHandleFactory<Container>::create(0x01);
    Container *pod_root = static_cast<Container*>(pod_root_node.get());

    for (auto &value: values)
    {
        NodeHandle rec_node = NodeHandleFactory<UInt32Record>::create(0x02);
        UInt32Record *rec = static_cast<UInt32Record*>(rec_node.get());
        rec->set(value);
        pod_root->child_add(rec_node);
    }

    std::vector<uint32_t> dest;

    typedef only<container<
        value<UInt32Record, 0x02, uint32_t>,
        std::back_insert_iterator<decltype(dest)>
        >> deserializer;

    FromTree(deserialize<deserializer>(dest),
             std::dynamic_pointer_cast<Container>(pod_root_node));

    REQUIRE((sizeof(values) / sizeof(uint32_t)) == dest.size());

    int value_idx = 0;
    for (auto it = dest.begin();
         it != dest.end();
         it++, value_idx++)
    {
        CHECK(values[value_idx] == *it);
    }
}
