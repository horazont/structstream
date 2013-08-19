/**********************************************************************
File name: decode_misc.cpp
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

TEST_CASE ("decode/regression/eoc_node_in_root", "Test decode of several nodes in the root node")
{
    static const uint8_t data[] = {
        uint8_t(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_ARMORED) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_CONTAINER) | 0x80, uint8_t(0x02) | 0x80,
        uint8_t(CF_ARMORED) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    ContainerHandle root = blob_to_tree(data, sizeof(data));

    CHECK(root->child_count() == 2);

    auto it = root->children_begin();

    NodeHandle node = *it;
    REQUIRE(node.get() != 0);

    Container *cont = dynamic_cast<Container*>(node.get());
    REQUIRE(cont != 0);
    CHECK(cont->id() == 0x01U);
    CHECK(cont->children_begin() == cont->children_end());

    it++;

    node = *it;
    REQUIRE(node.get() != 0);

    cont = dynamic_cast<Container*>(node.get());
    REQUIRE(cont != 0);
    CHECK(cont->id() == 0x02U);
    CHECK(cont->children_begin() == cont->children_end());
}

