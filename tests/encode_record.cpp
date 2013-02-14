/**********************************************************************
File name: encode_record.cpp
This file is part of: ebml++

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

For feedback and questions about ebml++ please e-mail one of the authors
named in the AUTHORS file.
**********************************************************************/
#include "catch.hpp"

#include "include/strstr_nodes.hpp"
#include "include/strstr_writer.hpp"

#define COMMON_HEADER
#define COMMON_FOOTER (uint8_t)(RT_END_OF_CHILDREN) | 0x80

using namespace StructStream;

TEST_CASE ("encode/record/uint32", "Encode a RT_UINT32 record")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78,
        COMMON_FOOTER
    };

    NodeHandle tree = NodeHandleFactory<UInt32Record>::create(0x01);
    UInt32Record *rec = static_cast<UInt32Record*>(tree.get());
    rec->set(0x78563412);

    uint8_t output[sizeof(expected)];

    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));
    Writer writer;
    writer.open(io);
    writer.write(tree);
    writer.close();

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/record/uint64", "Encode a RT_UINT64 record")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_UINT64) | 0x80, uint8_t(0x01) | 0x80,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        COMMON_FOOTER
    };

    NodeHandle tree = NodeHandleFactory<UInt64Record>::create(0x01);
    UInt64Record *rec = static_cast<UInt64Record*>(tree.get());
    rec->set(0xF0DEBC9A78563412);

    uint8_t output[sizeof(expected)];

    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));
    Writer writer;
    writer.open(io);
    writer.write(tree);
    writer.close();

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/record/int32", "Encode a RT_INT32 record")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_INT32) | 0x80, uint8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78,
        COMMON_FOOTER
    };

    NodeHandle tree = NodeHandleFactory<Int32Record>::create(0x01);
    Int32Record *rec = static_cast<Int32Record*>(tree.get());
    rec->set(0x78563412);

    uint8_t output[sizeof(expected)];

    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));
    Writer writer;
    writer.open(io);
    writer.write(tree);
    writer.close();

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/record/int64", "Encode a RT_INT64 record")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_INT64) | 0x80, uint8_t(0x01) | 0x80,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        COMMON_FOOTER
    };

    NodeHandle tree = NodeHandleFactory<Int64Record>::create(0x01);
    Int64Record *rec = static_cast<Int64Record*>(tree.get());
    rec->set(0xF0DEBC9A78563412);

    uint8_t output[sizeof(expected)];

    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));
    Writer writer;
    writer.open(io);
    writer.write(tree);
    writer.close();

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/record/blob", "Encode a blob")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_BLOB) | 0x80, uint8_t(0x01) | 0x80,
        // length
        uint8_t(0x0D) | 0x80,
        0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x00,
        COMMON_FOOTER
    };
    static const char reference[] = "Hello World!";

    NodeHandle tree = NodeHandleFactory<BlobRecord>::create(0x01);
    BlobRecord *rec = static_cast<BlobRecord*>(tree.get());
    rec->set(reference, strlen(reference)+1);

    uint8_t output[sizeof(expected)];

    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));
    Writer writer;
    writer.open(io);
    writer.write(tree);
    writer.close();

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/record/utf8", "Encode a utf8 string")
{
    static const uint8_t expected[] = {
        COMMON_HEADER
        (uint8_t)(RT_BLOB) | 0x80, uint8_t(0x01) | 0x80,
        // length
        uint8_t(0x0C) | 0x80,
        0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21,
        COMMON_FOOTER
    };
    static const char reference[] = "Hello World!";

    NodeHandle tree = NodeHandleFactory<BlobRecord>::create(0x01);
    BlobRecord *rec = static_cast<BlobRecord*>(tree.get());
    rec->set(reference, strlen(reference));

    uint8_t output[sizeof(expected)];

    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));
    Writer writer;
    writer.open(io);
    writer.write(tree);
    writer.close();

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}
