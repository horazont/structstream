/**********************************************************************
File name: strstr_utils.hpp
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
#ifndef _STRSTR_UTILS_H
#define _STRSTR_UTILS_H

#include "include/strstr_static.hpp"
#include "include/strstr_io.hpp"

namespace StructStream { namespace Utils {

/**
 * Read a signed EBML varint.
 */
StructStream::VarInt read_varint(StructStream::IOIntf *stream);

/**
 * Read an unsigned EBML varint.
 */
StructStream::VarUInt read_varuint(StructStream::IOIntf *stream);

/**
 * Read a structstream ID.
 */
StructStream::ID read_id(StructStream::IOIntf *stream);

/**
 * Read a structstream RecordType.
 */
StructStream::RecordType read_record_type(StructStream::IOIntf *stream);

/**
 * Write a signed EBML varint.
 *
 * No range-checking is done whether the value can actually be
 * represented as a varint. Surplus bits will be cut off.
 */
void write_varint(StructStream::IOIntf *stream, StructStream::VarInt value);

/**
 * Write an unsigned EBML varint.
 *
 * No range-checking is done whether the value can actually be
 * represented as a varint. Surplus bits will be cut off.
 */
void write_varuint(StructStream::IOIntf *stream, StructStream::VarUInt value);

/**
 * Write a structstream ID.
 */
void write_id(StructStream::IOIntf *stream, StructStream::ID value);

/**
 * Write a structstream RecordType.
 */
void write_record_type(StructStream::IOIntf *stream, StructStream::RecordType value);

}
}

#endif
