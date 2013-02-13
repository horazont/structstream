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

}
}
