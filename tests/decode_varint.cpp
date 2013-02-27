/**********************************************************************
File name: decode_varint.cpp
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

inline void check_varint_invariant(VarInt v)
{
    static uint8_t buffer[32];
    memset(buffer, 0, sizeof(buffer));

    WritableMemory *writer_io = new WritableMemory(buffer, sizeof(buffer));

    Utils::write_varint(writer_io, v);

    ReadableMemory *reader_io = new ReadableMemory(*writer_io);

    VarInt ref = Utils::read_varint(reader_io);

    // hexdump(buffer, writer_io->size());
    // printf("\n");

    CHECK(v == ref);

    delete reader_io;
    delete writer_io;
}

inline void check_varuint_invariant(VarUInt v)
{
    static uint8_t buffer[32];
    memset(buffer, 0, sizeof(buffer));

    WritableMemory *writer_io = new WritableMemory(buffer, sizeof(buffer));

    Utils::write_varuint(writer_io, v);

    ReadableMemory *reader_io = new ReadableMemory(*writer_io);

    VarUInt ref = Utils::read_varuint(reader_io);

    // hexdump(buffer, writer_io->size());
    // printf("\n");

    CHECK(v == ref);

    delete writer_io;
    delete reader_io;
}

TEST_CASE ("decode/varint", "Decode some varints")
{
    check_varint_invariant(-1);
    check_varint_invariant(-0x7f);
    check_varint_invariant(-((VarInt)1 << 32));
    check_varint_invariant(((VarInt)1 << 32));
    check_varint_invariant(0x7f);
    check_varint_invariant(1);
    check_varint_invariant(0);
    check_varint_invariant(MinVarInt);
    check_varint_invariant(MaxVarInt);
}

TEST_CASE ("decode/varuint", "Decode some varuints")
{
    check_varuint_invariant(((VarInt)1 << 32));
    check_varuint_invariant(0x7f);
    check_varuint_invariant(1);
    check_varuint_invariant(0);
    check_varuint_invariant(MaxVarUInt);
}
