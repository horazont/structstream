/**********************************************************************
File name: node_primitive.cpp
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
#include "structstream/node_primitive.hpp"

namespace StructStream {

/* StructStream::DataRecord */

DataRecord::DataRecord(ID id):
    Node::Node(id)
{

}

DataRecord::DataRecord(const DataRecord &ref):
    Node::Node(ref)
{

}

DataRecord::~DataRecord()
{

}

/* StructStream::BoolRecord */

BoolRecord::BoolRecord(ID id):
    BoolRecordBase(id)
{

}

BoolRecord::BoolRecord(const BoolRecord &ref):
    BoolRecordBase(ref)
{

}

void BoolRecord::read(IOIntf *stream)
{

}

void BoolRecord::write(IOIntf *stream) const
{
    write_header(stream);
}

RecordType BoolRecord::record_type() const
{
    return (_data ? RT_BOOL_TRUE : RT_BOOL_FALSE);
}

}
