/**********************************************************************
File name: serialize.cpp
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

#include "structstream/serialize.hpp"

#include "structstream/streaming_tree.hpp"
#include "structstream/node_primitive.hpp"
#include "structstream/node_container.hpp"

using namespace StructStream;

TEST_CASE ("serialize/primitive/int", "Serialize a single integer")
{
    static const uint32_t value = 0x12345678;
    typedef value_decl<UInt32Record, 0x01, uint32_t> serializer;

    ContainerHandle root = NodeHandleFactory<Container>::create(0x00);

    serialize_to_sink<serializer>(value, StreamSink(new ToTree(root)));

    NodeHandle result = *root->children_begin();
    UInt32Record *rec = dynamic_cast<UInt32Record*>(result.get());

    REQUIRE(result.get() != 0);
    REQUIRE(rec != 0);

    CHECK(rec->get() == value);
}

TEST_CASE ("serialize/block/simple", "Serialize a block with some primitive elements")
{
    struct block_t {
        uint32_t v1;
        uint8_t v2;
        float v3;
    };

    block_t block{0x12345678, 0x00, 0.2f};

    typedef struct_decl<
        Container,
        0x01,
        struct_members<
            member<UInt32Record, 0x11, block_t, uint32_t, &block_t::v1>,
            member<Float64Record, 0x13, block_t, float, &block_t::v3>,
            member<UInt32Record, 0x12, block_t, uint8_t, &block_t::v2>
            >
        > serializer;

    ContainerHandle root = NodeHandleFactory<Container>::create(0x00);
    serialize_to_sink<serializer>(block, StreamSink(new ToTree(root)));

    NodeHandle result = *root->children_begin();

    REQUIRE(result.get() != 0);

    Container *parent = dynamic_cast<Container*>(result.get());

    REQUIRE(parent != 0);

    NodeHandle curr_child = parent->first_child_by_id(0x11);
    REQUIRE(curr_child.get() != 0);
    UInt32Record *v1_rec = dynamic_cast<UInt32Record*>(curr_child.get());
    REQUIRE(v1_rec != 0);

    curr_child = parent->first_child_by_id(0x12);
    UInt32Record *v2_rec = dynamic_cast<UInt32Record*>(curr_child.get());
    REQUIRE(v2_rec != 0);

    curr_child = parent->first_child_by_id(0x13);
    Float64Record *v3_rec = dynamic_cast<Float64Record*>(curr_child.get());
    REQUIRE(v3_rec != 0);

    CHECK(v1_rec->get() == block.v1);
    CHECK(v2_rec->get() == block.v2);
    CHECK(v3_rec->get() == block.v3);
}


TEST_CASE ("serialize/block/bool", "Serialize a block with a boolean")
{
    struct block_t {
        bool v1;
        uint8_t v2;
    };

    block_t block{true, 0xff};

    typedef struct_decl<
        Container,
        0x01,
        struct_members<
            member<UInt32Record, 0x11, block_t, uint8_t, &block_t::v2>,
            member<BoolRecord, 0x13, block_t, bool, &block_t::v1>
            >
        > serializer;

    ContainerHandle root = NodeHandleFactory<Container>::create(0x00);
    serialize_to_sink<serializer>(block, StreamSink(new ToTree(root)));

    NodeHandle result = *root->children_begin();

    REQUIRE(result.get() != 0);

    Container *parent = dynamic_cast<Container*>(result.get());

    REQUIRE(parent != 0);

    NodeHandle curr_child = parent->first_child_by_id(0x13);
    REQUIRE(curr_child.get() != 0);
    BoolRecord *v1_rec = dynamic_cast<BoolRecord*>(curr_child.get());
    REQUIRE(v1_rec != 0);

    curr_child = parent->first_child_by_id(0x11);
    UInt32Record *v2_rec = dynamic_cast<UInt32Record*>(curr_child.get());
    REQUIRE(v2_rec != 0);

    CHECK(v1_rec->get() == block.v1);
    CHECK(v2_rec->get() == block.v2);
}

TEST_CASE ("serialize/array/int", "Serialize an array of integer")
{
    static const std::vector<uint32_t> values{1, 2, 3, 4, 5};

    ContainerHandle result = NodeHandleFactory<Container>::create(0x01);
    Container *cont = static_cast<Container*>(result.get());

    typedef container<
        value_decl<UInt32Record, 0x02, uint32_t>,
        std::back_insert_iterator<decltype(values)>
        > serializer;

    serialize_to_sink<serializer>(values, StreamSink(new ToTree(result)));

    REQUIRE((intptr_t)values.size() == (intptr_t)cont->child_count());

    auto intit = values.cbegin();

    for (auto nodeit = cont->children_cbegin();
         nodeit != cont->children_cend();
         nodeit++, intit++)
    {
        UInt32Record *rec = dynamic_cast<UInt32Record*>((*nodeit).get());
        REQUIRE(rec != 0);
        REQUIRE(rec->get() == *intit);
    }
}
