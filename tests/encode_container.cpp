/**********************************************************************
File name: encode_container.cpp
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

#define COMMON_HEADER
#define COMMON_FOOTER (uint8_t)(RT_END_OF_CHILDREN) | 0x80

using namespace StructStream;

TEST_CASE ("encode/container/empty", "Encode an empty container, without armor")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_CONTAINER) | 0x80,
        (uint8_t)(0x01) | 0x80,
        (uint8_t)(CF_WITH_SIZE) | 0x80,
        (uint8_t)(0x00),
        COMMON_FOOTER
    };

    NodeHandle tree = NodeHandleFactory<Container>::create(0x01);

    uint8_t output[sizeof(expected)];

    intptr_t size = tree_to_blob(output, sizeof(output), {tree}, false);
    REQUIRE(size == sizeof(expected));

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/container/empty_with_armor", "Encode an empty container, with armor")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_CONTAINER) | 0x80,
        (uint8_t)(0x01) | 0x80,
        (uint8_t)(CF_WITH_SIZE | CF_ARMORED) | 0x80,
        (uint8_t)(0x00),
        (uint8_t)(RT_END_OF_CHILDREN) | 0x80,
        COMMON_FOOTER
    };

    NodeHandle tree = NodeHandleFactory<Container>::create(0x01);

    uint8_t output[sizeof(expected)];

    intptr_t size = tree_to_blob(output, sizeof(output), {tree}, true);
    REQUIRE(size == sizeof(expected));

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/container/with_child", "Encode a container with one child")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_CONTAINER) | 0x80,
        (uint8_t)(0x01) | 0x80,
        (uint8_t)(CF_WITH_SIZE) | 0x80,
        (uint8_t)(0x01) | 0x80,
        (uint8_t)(RT_UINT32) | 0x80,
        (uint8_t)(0x02) | 0x80,
        0x11, 0x22, 0x33, 0x44,
        COMMON_FOOTER
    };

    NodeHandle tree = NodeHandleFactory<Container>::create(0x01);
    Container *cont = dynamic_cast<Container*>(tree.get());

    NodeHandle rec_node = NodeHandleFactory<UInt32Record>::create(0x02);
    UInt32Record *rec = dynamic_cast<UInt32Record*>(rec_node.get());
    rec->set(0x44332211);
    cont->child_add(rec_node);

    uint8_t output[sizeof(expected)];

    intptr_t size = tree_to_blob(output, sizeof(output), {tree}, false);
    REQUIRE(size == sizeof(expected));

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}
