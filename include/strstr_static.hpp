#ifndef _STRUCTSTREAM_STATIC_H
#define _STRUCTSTREAM_STATIC_H

#include <cstdint>

namespace StructStream {

typedef uint64_t VarUInt;
typedef int64_t VarInt;
typedef uint64_t ID;

const ID TreeRootID = 0x00000000;
const ID MaxID = 0xFFFFFFFFFFFFFFFF;
const ID InvalidID = MaxID;

enum RecordType {
    RT_RESERVED = 0x00,  // may indicate a larger typefield
    RT_CONTAINER = 0x01,
    RT_UINT32 = 0x02,
    RT_INT32 = 0x03,
    RT_UINT64 = 0x04,
    RT_INT64 = 0x05,
    RT_BOOL_TRUE = 0x06,
    RT_BOOL_FALSE = 0x07,
    RT_FLOAT32 = 0x08,
    RT_FLOAT64 = 0x09,
    RT_UTF8STRING = 0x0A,
    RT_BLOB = 0x0B,
    RT_END_OF_CHILDREN = 0x0C,
    RT_APPDEF_MIN = 0x40,
    RT_APPDEF_MAX = 0x7F,

    RT_APPDEF_NOSIZE_MIN = 0x100,
};

enum ContainerFlags {
    CF_WITH_SIZE = 0x01,
    CF_SIGNED = 0x02,
    CF_ENCRYPTED = 0x04,
};

enum HashType {
    HT_INVALID = 0x00,
    HT_SHA1 = 0x01,
    HT_SHA256 = 0x02,
};

}

#endif
