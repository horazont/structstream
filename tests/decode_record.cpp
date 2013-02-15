/**********************************************************************
File name: decode_record.cpp
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

#include "structstream/node_primitive.hpp"
#include "structstream/node_blob.hpp"
#include "structstream/reader.hpp"

using namespace StructStream;

TEST_CASE ("decode/records/uint32", "Test decode of a uint32 record")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    UInt32Record *rec = dynamic_cast<UInt32Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == 0x78563412);
}

TEST_CASE ("decode/records/uint64", "Test decode of a uint64 record")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_UINT64) | 0x80, uint8_t(0x01) | 0x80,
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    UInt64Record *rec = dynamic_cast<UInt64Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == 0xFFDEBC9A78563412);
}

TEST_CASE ("decode/records/int32", "Test decode of a int32 record")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_INT32) | 0x80, uint8_t(0x01) | 0x80, 0x12, 0x34, 0x56, 0x78
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    Int32Record *rec = dynamic_cast<Int32Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == (int32_t)0x78563412);
}

TEST_CASE ("decode/records/int64", "Test decode of a int64 record")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_INT64) | 0x80, uint8_t(0x01) | 0x80,
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF
    };

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    Int64Record *rec = dynamic_cast<Int64Record*>(node.get());
    REQUIRE(rec != 0);
    REQUIRE(rec->get() == (int64_t)0xFFDEBC9A78563412);
}

TEST_CASE ("decode/records/blob", "Test decode of a blob record")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_BLOB) | 0x80, uint8_t(0x01) | 0x80,
        // length
        uint8_t(0x0D) | 0x80,
        0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x00
    };
    static const char reference[] = "Hello World!";

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    BlobRecord *rec = dynamic_cast<BlobRecord*>(node.get());
    REQUIRE(rec != 0);

    char* buf = (char*)malloc(rec->raw_size());
    rec->raw_get(buf);

    REQUIRE(strlen(reference) == rec->raw_size()-1);
    REQUIRE(strcmp(buf, reference) == 0);
}

TEST_CASE ("decode/records/utf8", "Test decode of a utf8 record")
{
    // note the subtle difference that utf8 records don't require the
    // trailing 0x00 byte.
    static const uint8_t data[] = {
        (uint8_t)(RT_UTF8STRING) | 0x80, uint8_t(0x01) | 0x80,
        // length
        uint8_t(0x0C) | 0x80,
        0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21
    };
    static const char reference[] = "Hello World!";

    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, sizeof(data)));
    Reader reader(io, registry);

    NodeHandle node = reader.read_next();
    REQUIRE(node.get() != 0);

    UTF8Record *rec = dynamic_cast<UTF8Record*>(node.get());
    REQUIRE(rec != 0);

    char* buf = (char*)malloc(rec->raw_size());
    rec->raw_get(buf);

    REQUIRE(strlen(reference) == rec->raw_size()-1);
    REQUIRE(strcmp(buf, reference) == 0);
}
