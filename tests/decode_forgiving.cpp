/**********************************************************************
File name: decode_forgiving.cpp
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

TEST_CASE ("decode/forgiving/early_eoc", "Allow forgiving early end-of-children")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    ContainerHandle root = blob_to_tree(
        data,
        sizeof(data),
        FromBitstream::PrematureEndOfContainer);
    REQUIRE(root.get() != 0);
    REQUIRE((*root->children_begin()).get() != 0);
}

TEST_CASE ("decode/forgiving/unknown_hash", "Allow forgiving unknown hash functions")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED | CF_HASHED) | 0x80,
        uint8_t(0x00) | 0x80, // length
        uint8_t(HT_INVALID) | 0x80, // hash function
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(0x04) | 0x80, 0x00, 0x00, 0x00, 0x00, // hash length and hash
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    ContainerHandle root = blob_to_tree(
        data,
        sizeof(data),
        FromBitstream::UnknownHashFunction);
    REQUIRE(root.get() != 0);
    REQUIRE((*root->children_begin()).get() != 0);
}

#ifdef WITH_GNUTLS
TEST_CASE ("decode/forgiving/wrong_hash", "Allow forgiving mismatching hashes")
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

    ContainerHandle root = blob_to_tree(
        data,
        sizeof(data),
        FromBitstream::ChecksumErrors);
    REQUIRE(root.get() != 0);

    NodeHandle child = *root->children_begin();
    REQUIRE(child.get() != 0);

    Container *cont = dynamic_cast<Container*>(child.get());
    REQUIRE(cont != 0);
    CHECK(cont->get_hashed() == HT_NONE);
}
#endif

TEST_CASE ("decode/forgiving/unknown_container_flags", "Allow forgiving unknown container flags")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED | CF_APP0) | 0x80,
        uint8_t(0x00) | 0x80, // length
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    ContainerHandle root = blob_to_tree(
        data,
        sizeof(data),
        FromBitstream::UnknownContainerFlags);
    REQUIRE(root.get() != 0);
    REQUIRE((*root->children_begin()).get() != 0);
}

TEST_CASE ("decode/forgiving/unknown_appblob", "Allow forgiving unknown appblobs")
{
    static const uint8_t data[] = {
        (uint8_t)(RT_CONTAINER) | 0x80, uint8_t(0x01) | 0x80,
        uint8_t(CF_WITH_SIZE | CF_ARMORED) | 0x80,
        uint8_t(0x01) | 0x80, // length
        uint8_t(RT_APPBLOB_MIN) | 0x80, uint8_t(0x02) | 0x80, uint8_t(0x04) | 0x80, 0x00, 0x00, 0x00, 0x00,
        uint8_t(RT_END_OF_CHILDREN) | 0x80,
        uint8_t(RT_END_OF_CHILDREN) | 0x80
    };

    ContainerHandle root = blob_to_tree(
        data,
        sizeof(data),
        FromBitstream::UnknownAppblobs);
    REQUIRE(root.get() != 0);
    REQUIRE((*root->children_begin()).get() != 0);
}
