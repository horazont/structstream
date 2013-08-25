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
#include "structstream/node_container.hpp"
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

    ContainerHandle pod_root = NodeHandleFactory<Container>::create(0x01);

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
    memset(&pod, 0, sizeof(pod));

    typedef struct_decl<
        Container,
        id_selector<0x01>,
        struct_members<
            member<UInt32Record, id_selector<0x02>, pod_t, uint32_t, &pod_t::v1>,
            member<UInt32Record, id_selector<0x04>, pod_t, uint8_t, &pod_t::v3>,
            member<Float64Record, id_selector<0x03>, pod_t, double, &pod_t::v2>
            >
        > deserializer;

    FromTree(deserialize<only<deserializer, true, true>>(pod),
             {pod_root});

    CHECK(pod.v1 == 0x2342dead);
    CHECK(pod.v2 == 23.42);
    CHECK(pod.v3 == 0x12);
}


TEST_CASE ("deserialize/pod/nested", "Deserialization of nested plain-old-data types")
{
    struct pod1_t {
        uint32_t v1;
        double v2;
    };

    struct pod2_t {
        pod1_t v1;
        uint8_t v2;
    };

    ContainerHandle pod1_cont = NodeHandleFactory<Container>::create(0x01);

    std::shared_ptr<UInt32Record> uint32_node = NodeHandleFactory<UInt32Record>::create(0x02);
    uint32_node->set(0x2342dead);
    pod1_cont->child_add(uint32_node);

    std::shared_ptr<Float64Record> float64_node = NodeHandleFactory<Float64Record>::create(0x03);
    float64_node->set(23.42);
    pod1_cont->child_add(float64_node);

    ContainerHandle pod2_cont = NodeHandleFactory<Container>::create(0x05);

    uint32_node = NodeHandleFactory<UInt32Record>::create(0x04);
    uint32_node->set(0xffffff12);
    pod2_cont->child_add(uint32_node);

    pod2_cont->child_add(pod1_cont);

    pod2_t pod;

    typedef struct_decl<
        Container,
        id_selector<0x05>,
        struct_members<
            member_struct<
                pod2_t,
                struct_decl<
                    Container,
                    id_selector<0x01>,
                    struct_members<
                        member<UInt32Record, id_selector<0x02>, pod1_t, uint32_t, &pod1_t::v1>,
                        member<Float64Record, id_selector<0x03>, pod1_t, double, &pod1_t::v2>
                        >
                    >,
                &pod2_t::v1
                >,
            member<UInt32Record, id_selector<0x04>, pod2_t, uint8_t, &pod2_t::v2>
            >
        > deserializer;

    FromTree(deserialize<only<deserializer, true, true>>(pod),
             {pod2_cont});

    CHECK(pod.v1.v1 == 0x2342dead);
    CHECK(pod.v1.v2 == 23.42);
    CHECK(pod.v2 == 0x12);
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
        id_selector<0x01>,
        struct_members<
            member<UTF8Record, id_selector<0x02>, block_t, std::string, &block_t::value>
            >
        > deserializer;

    FromTree(deserialize<deserializer>(block),
             std::dynamic_pointer_cast<Container>(pod_root_node));

    CHECK(block.value == "Hello World!");
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
        value_decl<UInt32Record, id_selector<0x02>, uint32_t>,
        std::back_insert_iterator<decltype(dest)>
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

    ContainerHandle pod_root = NodeHandleFactory<Container>::create(0x01);

    for (auto &value: values)
    {
        std::shared_ptr<UInt32Record> rec = NodeHandleFactory<UInt32Record>::create(0x02);
        rec->set(value);
        pod_root->child_add(rec);
    }

    std::vector<uint32_t> dest;

    typedef container<
        value_decl<UInt32Record, id_selector<0x02>, uint32_t>,
        id_selector<0x01>,
        std::back_insert_iterator<decltype(dest)>
        > deserializer;

    FromTree(deserialize<deserializer>(dest),
             std::dynamic_pointer_cast<Container>(pod_root));

    REQUIRE((sizeof(values) / sizeof(uint32_t)) == dest.size());

    int value_idx = 0;
    for (auto it = dest.begin();
         it != dest.end();
         it++, value_idx++)
    {
        CHECK(values[value_idx] == *it);
    }
}

TEST_CASE ("deserialize/only/detect_missing", "Detect missing object with only<> deserializer")
{
    ContainerHandle result = NodeHandleFactory<Container>::create(0x00);
    NodeHandle rec_node = NodeHandleFactory<UInt32Record>::create(0x02);
    result->child_add(rec_node);

    uint32_t dest;
    typedef only<value_decl<UInt32Record, id_selector<0x01>, uint32_t>, true> deserializer;


    CHECK_THROWS_AS(
        FromTree(deserialize<deserializer>(dest), result),
        RecordNotFound);
}

TEST_CASE ("deserialize/only/require_first/err", "Require an object to be first with only<>")
{
    ContainerHandle result = NodeHandleFactory<Container>::create(0x00);
    std::shared_ptr<UInt32Record> rec_node = NodeHandleFactory<UInt32Record>::create(0x02);
    result->child_add(rec_node);
    rec_node = NodeHandleFactory<UInt32Record>::create(0x01);
    result->child_add(rec_node);

    uint32_t dest;
    typedef only<value_decl<UInt32Record, id_selector<0x01>, uint32_t>, true, true> deserializer;

    CHECK_THROWS_AS(
        FromTree(deserialize<deserializer>(dest), result),
        RecordNotFound);
}

TEST_CASE ("deserialize/only/require_first/ok", "Require an object to be first with only<>")
{
    ContainerHandle result = NodeHandleFactory<Container>::create(0x00);
    std::shared_ptr<UInt32Record> rec_node = NodeHandleFactory<UInt32Record>::create(0x01);
    rec_node->set(2342);
    result->child_add(rec_node);
    rec_node = NodeHandleFactory<UInt32Record>::create(0x02);
    result->child_add(rec_node);

    uint32_t dest = 0;
    typedef only<value_decl<UInt32Record, id_selector<0x01>, uint32_t>, false, true> deserializer;

    FromTree(deserialize<deserializer>(dest), result);

    CHECK(dest == 2342);
}
