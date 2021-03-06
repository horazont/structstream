/**********************************************************************
File name: encode_varint.cpp
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

#include <cstring>

#include "structstream/io.hpp"
#include "structstream/utils.hpp"

#include "tests/utils.hpp"

using namespace StructStream;

TEST_CASE ("encode/varuint/0", "Encode a 1 byte long varuint (zero)")
{
    static const uint8_t expected[] = {
        0x80
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x00);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/1", "Encode a 1 byte long varuint")
{
    static const uint8_t expected[] = {
        0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x7f);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/2", "Encode a 2 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x7f, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x3fff);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/3", "Encode a 3 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x3f, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x1fffff);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/4", "Encode a 4 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x1f, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x0fffffff);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/5", "Encode a 5 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x0f, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x07ffffffff);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/6", "Encode a 6 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x07, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x03ffffffffff);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/7", "Encode a 7 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0x01ffffffffffff);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varuint/8", "Encode a 8 bytes long varuint")
{
    static const uint8_t expected[] = {
        0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varuint(io.get(), 0xffffffffffffff);

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varint/-1", "Encode a negative one as varint")
{
    static const uint8_t expected[] = {
        (uint8_t)(0x80 | 0x41)
    };

    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varint(io.get(), -1);

    // hexdump(output, static_cast<WritableMemory*>(io.get())->size());
    // printf("\n");

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}

TEST_CASE ("encode/varint/-0x7f", "Encode a negative varint at the byte count border")
{
    static const uint8_t expected[] = {
        (uint8_t)(0x40 | 0x20),
        (uint8_t)(0x7f)
    };

    uint8_t output[sizeof(expected)];
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, sizeof(expected)));

    Utils::write_varint(io.get(), -0x7f);

    // hexdump(output, static_cast<WritableMemory*>(io.get())->size());
    // printf("\n");

    REQUIRE(static_cast<WritableMemory*>(io.get())->size() == sizeof(expected));
    REQUIRE(memcmp(expected, output, sizeof(expected)) == 0);
}
