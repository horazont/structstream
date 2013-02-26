/**********************************************************************
File name: decode_container.cpp
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

#include "tests/utils.hpp"

using namespace StructStream;

TEST_CASE ("decode/container/empty", "Test decode of an empty container with explicit length")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE) | 0x80, uint8_t(0x00),
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    ContainerHandle root = blob_to_tree(data, sizeof(data));

    NodeHandle node = *(root->children_begin());
    REQUIRE(node.get() != 0);

    Container *cont = dynamic_cast<Container*>(node.get());
    REQUIRE(cont != 0);
    CHECK(cont->id() == 0x01);
    CHECK(cont->children_begin() == cont->children_end());
}

TEST_CASE ("decode/container/empty_implicit", "Test decode of an empty container without explicit length")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_ARMORED) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    ContainerHandle root = blob_to_tree(data, sizeof(data));

    NodeHandle node = *(root->children_begin());
    REQUIRE(node.get() != 0);

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

    ContainerHandle root = blob_to_tree(data, sizeof(data));

    NodeHandle node = *(root->children_begin());
    REQUIRE(node.get() != 0);

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
        uint8_t(CF_ARMORED) | 0x80,  // and no other information (i.e. indefinite length)

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

    ContainerHandle root = blob_to_tree(data, sizeof(data));

    NodeHandle node = *(root->children_begin());
    REQUIRE(node.get() != 0);

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

TEST_CASE ("decode/container/surplus_eoc", "Detect errornous End-Of-Children / lost child")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    REQUIRE_THROWS_AS(blob_to_tree(data, sizeof(data)), UnexpectedEndOfChildren);
}

TEST_CASE ("decode/container/early_eoc", "Detect early End-Of-Children")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    REQUIRE_THROWS_AS(blob_to_tree(data, sizeof(data)), UnexpectedEndOfChildren);
}

TEST_CASE ("decode/container/missing_eoc", "Detect too many children / missing EOC")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED) | 0x80, uint8_t(0x00),
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x02) | 0x80, 0x00, 0x00, 0x00, 0x00,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    REQUIRE_THROWS_AS(blob_to_tree(data, sizeof(data)), MissingEndOfChildren);
}
