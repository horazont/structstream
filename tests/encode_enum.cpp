/**********************************************************************
File name: encode_enum.cpp
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
#include "structstream/node_primitive.hpp"
#include "structstream/node_varint.hpp"

#define COMMON_HEADER
#define COMMON_FOOTER (uint8_t)(RT_END_OF_CHILDREN) | 0x80

using namespace StructStream;

enum Foo {
    Foo_A = 0,
    Foo_B = 1,
    Foo_C = 2
};

TEST_CASE ("encode/record/enum_uint32", "Encode a uint32 enum record")
{
    typedef EnumRecordTpl<Foo, 0x60, UInt32Record> EnumRecord;

    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(0x60) | 0x80, uint8_t(0x01) | 0x80, 0x01, 0x00, 0x00, 0x00,
        COMMON_FOOTER
    };


    std::shared_ptr<EnumRecord> tree = NodeHandleFactory<EnumRecord>::create(0x01);
    tree->set(Foo_B);

    uint8_t output[sizeof(expected)];

    intptr_t size = tree_to_blob(output, sizeof(output), {tree});
    REQUIRE(size == sizeof(expected));

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/record/enum_varuint", "Encode a uint32 enum record")
{
    typedef EnumRecordTpl<Foo, 0x60, VarUIntRecord> EnumRecord;

    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(0x60) | 0x80, uint8_t(0x01) | 0x80, 0x81,
        COMMON_FOOTER
    };


    std::shared_ptr<EnumRecord> tree = NodeHandleFactory<EnumRecord>::create(0x01);
    tree->set(Foo_B);

    uint8_t output[sizeof(expected)];

    intptr_t size = tree_to_blob(output, sizeof(output), {tree});
    REQUIRE(size == sizeof(expected));

    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}
