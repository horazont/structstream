/**********************************************************************
File name: decode_failures.cpp
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
#include "structstream/hashing.hpp"

using namespace StructStream;

inline VarInt blob_to_varint(const uint8_t *data, const intptr_t len)
{
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, len));
    return Utils::read_varint(io.get());
}

inline VarInt blob_to_varuint(const uint8_t *data, const intptr_t len)
{
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, len));
    return Utils::read_varuint(io.get());
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
        uint8_t(CF_WITH_SIZE | CF_ARMORED) | 0x80, uint8_t(0x00) | 0x80,
        (uint8_t)(RT_UINT32) | 0x80, uint8_t(0x02) | 0x80, 0x00, 0x00, 0x00, 0x00,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    REQUIRE_THROWS_AS(blob_to_tree(data, sizeof(data)), MissingEndOfChildren);
}

TEST_CASE ("decode/container/unknown_flags", "Detect unknown container flags")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED | CF_APP0) | 0x80,
        uint8_t(0x00) | 0x80, // length
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    REQUIRE_THROWS_AS(blob_to_tree(data, sizeof(data)), UnsupportedContainerFlags);
}

TEST_CASE ("decode/container/unknown_hash_function", "Detect unknown hash function")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED | CF_HASHED) | 0x80,
        uint8_t(0x00) | 0x80, // length
        uint8_t(HT_INVALID) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    REQUIRE_THROWS_AS(blob_to_tree(data, sizeof(data)), UnsupportedHashFunction);
}

#ifdef WITH_GNUTLS
TEST_CASE ("decode/container/wrong_hash", "Detect wrong hash")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED | CF_HASHED) | 0x80,
        uint8_t(0x00) | 0x80, // length
        uint8_t(HT_SHA1) | 0x80, // hash function
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(0x14) | 0x80, // hash length
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (wrong) hash
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    load_all_hashes();

    REQUIRE_THROWS_AS(blob_to_tree(data, sizeof(data)), HashCheckError);
}
#endif

TEST_CASE ("decode/records/unknown_appblob", "Abort on unknown app blobs")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_APPBLOB_MIN) | 0x80, uint8_t(0x01) | 0x80,
        (uint8_t)(RT_END_OF_CHILDREN) | 0x80,
    };

    REQUIRE_THROWS_AS(blob_to_tree(data, sizeof(data)), UnsupportedRecordType);
}

TEST_CASE ("decode/varuint/invalid", "Abort on invalid varints")
{
    static const uint8_t data[] = {
        0x00
    };

    REQUIRE_THROWS_AS(blob_to_varuint(data, sizeof(data)), InvalidVarIntError);
}

TEST_CASE ("decode/varuint/missing_payload", "Abort on too short varints")
{
    static const uint8_t data[] = {
        0x40
    };

    REQUIRE_THROWS_AS(blob_to_varuint(data, sizeof(data)), EndOfStreamError);
}
