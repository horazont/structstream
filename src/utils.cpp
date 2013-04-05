/**********************************************************************
File name: utils.cpp
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
#include "structstream/utils.hpp"

#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "structstream/errors.hpp"

namespace StructStream { namespace Utils {

template <typename int_type>
struct platform_utils
{
};

template <>
struct platform_utils<long unsigned int>
{
    typedef long unsigned int int_type;

    static constexpr uint_fast8_t total_bit_count = sizeof(int_type) * 8;

    static inline uint_fast8_t significant_bits(int_type value)
    {
        return total_bit_count - __builtin_clzl(value);
    };
};

template <>
struct platform_utils<long long unsigned int>
{
    typedef long long unsigned int int_type;

    static constexpr uint_fast8_t total_bit_count = sizeof(int_type) * 8;

    static inline uint_fast8_t significant_bits(int_type value)
    {
        return total_bit_count - __builtin_clzll(value);
    };
};


using namespace StructStream;

VarUInt read_varuint_ex(IOIntf *stream, intptr_t *overlen, uint_fast8_t *bytecount)
{
    VarUInt result = 0;
    uint8_t leading = 0;
    sread(stream, &leading, sizeof(uint8_t));
    if (overlen) {
        *overlen = 0;
    }
    if (leading == 0x00) {
        throw InvalidVarIntError("0x00 is not a valid Var(U)Int.");
    }
    if (leading == 0x80) {
        if (bytecount) {
            *bytecount = 1;
        }
        return 0;
    }

    // GCC rulez
    uint8_t count = __builtin_clz(leading)-24;
    result |= ((uint64_t)(leading & (0xFF >> (count+1))) << count*8);
    if (bytecount) {
        *bytecount = count+1;
    }
    if (overlen && (result == 0))  {
        *overlen += 1;
    }
    if (count == 0) {
        return result;
    }

    // this size must be increased if we ever support more than 8-byte
    // varuints.
    uint8_t buffer[7];
    assert(count <= 7);
    sread(stream, buffer, count);

    for (int idx = 0; idx < count; idx++) {
        if (overlen) {
            if (buffer[idx] == 0) {
                *overlen += 1;
            } else {
                *overlen = 0;
            }
        }
        result |= ((uint64_t)(buffer[idx]) << ((count-idx)-1)*8);
    }

    return result;
}

VarInt read_varint(IOIntf *stream)
{
    uint_fast8_t bytecount = 0;
    VarUInt raw = read_varuint_ex(stream, nullptr, &bytecount);
    assert(bytecount != 0);

    VarUInt mask = ((VarUInt)1 << (7*bytecount-1));
    if ((raw & mask) != 0) {
        raw ^= mask;
        return -(VarInt)(raw);
    }
    return raw;
}

VarUInt read_varuint(IOIntf *stream)
{
    return read_varuint_ex(stream, nullptr, nullptr);
}

ID read_id(IOIntf *stream)
{
    return read_varuint_ex(stream, nullptr, nullptr);
}

RecordType read_record_type(IOIntf *stream)
{
    return (RecordType)read_varuint_ex(stream, nullptr, nullptr);
}

inline uint_fast8_t bytecount_from_varuint(VarUInt value)
{
    const uint_fast8_t bitcount = platform_utils<VarUInt>::significant_bits(value);
    // this gives ceil((float)bitcount / 7)
    const uint_fast8_t bytecount = (bitcount+6) / 7;

    return bytecount;
}

void write_varbuf_ex(IOIntf *stream, VarUInt buf, uint_fast8_t bytecount)
{
    if (buf == 0) {
        swritev<uint8_t>(stream, 0x80);
        return;
    }

    // printf("serializing varint 0x%lx; maxbit = %d; bytes = %d\n", value, bitcount, bytecount);

    assert(bytecount > 0);

    uint8_t leading = ((uint8_t)0x80 >> (bytecount-1));
    const VarUInt leading_premask = (VarUInt)0xff << ((bytecount-1)*8);
    const VarUInt leading_mask = (leading_premask >> bytecount) & leading_premask;

    // printf("  premask = %lx\n", leading_premask);
    // printf("  mask = %lx\n", leading_mask);

    //assert((total_bit_count-__builtin_clzl(leading_mask)) == bytecount);

    leading |= (buf & leading_mask) >> ((bytecount-1)*8);

    // printf("  => leading = 0x%x\n", leading);

    swritev<uint8_t>(stream, leading);

    for (int_fast8_t i = bytecount - 2;
         i >= 0;
         i--)
    {
        const VarUInt mask = (VarUInt)0xff << (i*8);
        // printf("  i = %d\n", i);
        // printf("    mask = %lx\n", mask);
        const uint8_t masked = (buf & mask) >> (i*8);
        // printf("    masked = %x\n", masked);
        swritev<uint8_t>(stream, masked);
    }
}

void write_varint(IOIntf *stream, VarInt value)
{
    VarUInt enc_value = 0;
    if (value < 0) {
        enc_value = (VarUInt)(-value);
    } else {
        enc_value = (VarUInt)(value);
    }

    const uint_fast8_t bitcount = platform_utils<VarUInt>::significant_bits(
        enc_value);
    // intentionally one bit more than needed to encode the value
    const uint_fast8_t bytecount = (bitcount+7)/7;

    if (value < 0) {
        assert((enc_value | ((VarUInt)1 << (bytecount*7-1))) != enc_value);
        // embed sign-bit into output
        enc_value |= (VarUInt(1) << (bytecount*7-1));
    }

    write_varbuf_ex(stream, enc_value, bytecount);
}

void write_varuint(IOIntf *stream, VarUInt value)
{
    write_varbuf_ex(stream, value, bytecount_from_varuint(value));
}

void write_id(IOIntf *stream, ID value)
{
    write_varuint(stream, value);
}

void write_record_type(IOIntf *stream, RecordType value)
{
    write_varuint(stream, static_cast<RecordType>(value));
}

}
}
