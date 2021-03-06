/**********************************************************************
File name: static.hpp
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
#ifndef _STRUCTSTREAM_STATIC_H
#define _STRUCTSTREAM_STATIC_H

#include <cstdint>

namespace StructStream {

typedef uint64_t VarUInt;
typedef int64_t VarInt;
typedef uint64_t ID;
typedef VarUInt RecordType;

const VarUInt MaxVarUInt = 0xffffffffffffff;
const VarUInt MinVarUInt = 0x0;

const VarInt MinVarInt = -0x7fffffffffffff;
const VarInt MaxVarInt = 0x7fffffffffffff;

const ID TreeRootID = 0x00000000;
const ID MaxID = MaxVarUInt;
const ID InvalidID = MaxID;

const RecordType RT_RESERVED = 0x00;  // may indicate a larger typefield
const RecordType RT_CONTAINER = 0x01;
const RecordType RT_UINT32 = 0x02;
const RecordType RT_INT32 = 0x03;
const RecordType RT_UINT64 = 0x04;
const RecordType RT_INT64 = 0x05;
const RecordType RT_BOOL_TRUE = 0x06;
const RecordType RT_BOOL_FALSE = 0x07;
const RecordType RT_FLOAT32 = 0x08;
const RecordType RT_FLOAT64 = 0x09;
const RecordType RT_UTF8STRING = 0x0A;
const RecordType RT_BLOB = 0x0B;
const RecordType RT_END_OF_CHILDREN = 0x0C;
const RecordType RT_VARUINT = 0x0D;
const RecordType RT_VARINT = 0x0E;
const RecordType RT_RAW128 = 0x0F;

const RecordType RT_APPBLOB_MIN = 0x40;
const RecordType RT_APPBLOB_MAX = 0x5f;
const RecordType RT_APP_NOSIZE_MIN = 0x60;
const RecordType RT_APP_NOSIZE_MAX = 0x7f;

// up to here, record type doesn't require a second byte


enum ContainerFlags {
    CF_WITH_SIZE = 0x0001,
    CF_HASHED = 0x0002,
    CF_ARMORED = 0x0004,
    CF_RESERVED0 = 0x0008,

    CF_APP0 = 0x0010,
    CF_APP1 = 0x0020,
    CF_APP2 = 0x0040,

    // up to here, flags don't require a second byte.

    CF_RESERVED1 = 0x0080,
    CF_RESERVED2 = 0x0100,
    CF_RESERVED3 = 0x0200,
    CF_RESERVED4 = 0x0400,
    CF_RESERVED5 = 0x0800,

    CF_APP3 = 0x1000,
    CF_APP4 = 0x2000

    // up to here, flags don't require a third byte.
};

enum HashType {
    HT_NONE = 0x00,
    HT_SHA1 = 0x01,
    HT_SHA256 = 0x02,
    HT_SHA512 = 0x03,
    HT_CRC32 = 0x04,
    HT_MD5 = 0x05,
    HT_APP0 = 0x40,
    HT_INVALID = 0x7f,
};

}

#endif
