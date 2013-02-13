#ifndef _STRSTR_UTILS_H
#define _STRSTR_UTILS_H

#include "include/strstr_static.hpp"
#include "include/strstr_io.hpp"

namespace StructStream { namespace Utils {

StructStream::VarInt read_varint(StructStream::IOIntf *stream);
StructStream::VarUInt read_varuint(StructStream::IOIntf *stream);
StructStream::ID read_id(StructStream::IOIntf *stream);
StructStream::RecordType read_record_type(StructStream::IOIntf *stream);

}
}

#endif
