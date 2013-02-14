/**********************************************************************
File name: strstr_utils.cpp
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
#include "include/strstr_utils.hpp"

#include <cassert>
#include <cstdlib>

#include "include/strstr_io.hpp"

namespace StructStream { namespace Utils {

using namespace StructStream;

VarUInt read_varuint_ex(IOIntf *stream, intptr_t *overlen)
{
    VarUInt result = 0;
    uint8_t leading = 0;
    sread(stream, &leading, sizeof(uint8_t));
    if (overlen) {
        *overlen = 0;
    }
    if (leading == 0) {
	return result;
    }

    // GCC rulez
    uint8_t count = __builtin_clz(leading)-24;
    result |= ((uint64_t)(leading & (0xFF >> (count+1))) << count*8);
    if (overlen && (result == 0))  {
        *overlen += 1;
    }

    uint8_t buffer[count];
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
    VarUInt raw = read_varuint_ex(stream, nullptr);
    return *((VarInt*)&raw);
}

VarUInt read_varuint(IOIntf *stream)
{
    return read_varuint_ex(stream, 0);
}

ID read_id(IOIntf *stream)
{
    return read_varuint_ex(stream, 0);
}

RecordType read_record_type(IOIntf *stream)
{
    return (RecordType)read_varuint_ex(stream, 0);
}

void write_varint(IOIntf *stream, VarInt value)
{
    write_varuint(stream, *((VarUInt*)&value));
}

void write_varuint(IOIntf *stream, VarUInt value)
{
    const uint_fast8_t total_bit_count = (sizeof(value)*8);
    const uint_fast8_t bitcount = total_bit_count-__builtin_clzl(value);
    // this gives ceil((float)bitcount / 7)
    const uint_fast8_t bytecount = (bitcount+6) / 7;

    if (bytecount == 0) {
        swritev<uint8_t>(stream, 0x00);
        return;
    }

    uint8_t leading = ((uint8_t)0x80 >> (bytecount-1));
    VarUInt leading_premask = 0xff << ((bytecount-2)*8);
    VarUInt leading_mask = (leading_premask >> bytecount) & leading_premask;

    assert((total_bit_count-__builtin_clzl(leading_mask)) == bytecount);

    leading |= (value & leading_mask) >> ((bytecount-2)*8);

    swritev<uint8_t>(stream, leading);

    for (uint_fast8_t i = bytecount - 1;
         i >= 0;
         i++)
    {
        VarUInt mask = 0xff << (i*8);
        swritev<uint8_t>(stream, (value & mask) >> (i*8));
    }
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
