/**********************************************************************
File name: node_varint.cpp
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
#include "structstream/node_varint.hpp"

namespace StructStream {

/* StructStream::VarIntRecord */

VarIntRecord::VarIntRecord(ID id):
    PrimitiveDataRecord<VarInt, RT_VARINT>(id)
{

}

NodeHandle VarIntRecord::copy() const
{
    return NodeHandleFactory<VarIntRecord>::copy(*this);
}

void VarIntRecord::read(IOIntf *stream)
{
    _data = Utils::read_varint(stream);
}

void VarIntRecord::write(IOIntf *stream) const
{
    write_header(stream);
    Utils::write_varint(stream, _data);
}

/* StructStream::VarUIntRecord */

VarUIntRecord::VarUIntRecord(ID id):
    PrimitiveDataRecord<VarUInt, RT_VARUINT>(id)
{

}

NodeHandle VarUIntRecord::copy() const
{
    return NodeHandleFactory<VarUIntRecord>::copy(*this);
}

void VarUIntRecord::read(IOIntf *stream)
{
    _data = Utils::read_varuint(stream);
}

void VarUIntRecord::write(IOIntf *stream) const
{
    write_header(stream);
    Utils::write_varuint(stream, _data);
}


}
