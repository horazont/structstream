#ifndef _STRUCTSTREAM_STATIC_H
#define _STRUCTSTREAM_STATIC_H

#include <cstdint>

namespace StructStream {

typedef uint64_t VarUInt;
typedef int64_t VarInt;
typedef uint64_t ID;

enum RecordType {
    RT_RESERVED = 0x00,  // may indicate a larger typefield
    RT_NULL = 0x01,
    RT_UINT = 0x02,
    RT_INT = 0x03,
    RT_BOOL_TRUE = 0x04,
    RT_BOOL_FALSE = 0x05,
    RT_FLOAT32 = 0x06,
    RT_FLOAT64 = 0x07,
    RT_FLOAT80 = 0x08,
    RT_FLOAT128 = 0x09,
    RT_HUGEINT = 0x0A,
    RT_HUGEUINT = 0x0B,
    RT_HUGEFLOAT = 0x0C,
    RT_UTF8STRING = 0x0D,
    RT_BLOB = 0x0E,
    RT_CONTAINER = 0x0F,
    RT_UTCDATE = 0x10,
    RT_TZDATE = 0x11,  // unused
    RT_APPDEF_MIN = 0x40,
    RT_APPDEF_MAX = 0x7F,
};

}

#endif
