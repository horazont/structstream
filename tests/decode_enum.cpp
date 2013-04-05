/**********************************************************************
File name: decode_enum.cpp
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

using namespace StructStream;

enum Foo {
    Foo_A = 0,
    Foo_B = 1,
    Foo_C = 2
};

TEST_CASE ("decode/records/enum_uint32", "Test decode of a uint32 enum record")
{
    typedef EnumRecordTpl<Foo, 0x60, UInt32Record> EnumRecord;

    static const uint8_t data[] = {
        uint8_t(0x60 | 0x80), uint8_t(0x01 | 0x80), 0x01, 0x00, 0x00, 0x00,
        uint8_t(RT_END_OF_CHILDREN | 0x80)
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    registry->register_record_class<EnumRecord>(0x60);

    NodeHandle node = *(blob_to_tree(data, sizeof(data), 0, registry)->children_begin());
    REQUIRE(node.get() != 0);

    EnumRecord *rec = dynamic_cast<EnumRecord*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == Foo_B);
}

TEST_CASE ("decode/records/enum_varuint", "Test decode of a varuint enum record")
{
    typedef EnumRecordTpl<Foo, 0x60, VarUIntRecord> EnumRecord;

    static const uint8_t data[] = {
        uint8_t(0x60 | 0x80), uint8_t(0x01 | 0x80), uint8_t(0x01 | 0x80),
        uint8_t(RT_END_OF_CHILDREN | 0x80)
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    registry->register_record_class<EnumRecord>(0x60);

    NodeHandle node = *(blob_to_tree(data, sizeof(data), 0, registry)->children_begin());
    REQUIRE(node.get() != 0);

    EnumRecord *rec = dynamic_cast<EnumRecord*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == Foo_B);
}
