/**********************************************************************
File name: node_blob.cpp
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
#include "structstream/node_blob.hpp"

namespace StructStream {

/* StructStream::UTF8Record */

UTF8Record::~UTF8Record()
{

}

NodeHandle UTF8Record::copy() const
{
    return NodeHandleFactory<UTF8Record>::copy(*this);
}

void UTF8Record::read(IOIntf *stream)
{
    // \0 is implied!
    VarInt length = read_and_check_length(stream) + 1;

    allocate_length(length);

    sread(stream, _buf, size()-1);
    ((char*)_buf)[length-1] = 0;
}

void UTF8Record::write(IOIntf *stream) const
{
    write_header(stream);
    Utils::write_varint(stream, _len-1);
    swrite(stream, _buf, _len-1);
}

/* StructStream::BlobRecord */

BlobRecord::~BlobRecord()
{

}

NodeHandle BlobRecord::copy() const
{
    return NodeHandleFactory<BlobRecord>::copy(*this);
}

void BlobRecord::read(IOIntf *stream) {
    VarInt length = read_and_check_length(stream);
    allocate_length(length);
    sread(stream, _buf, size());
};

void BlobRecord::write(IOIntf *stream) const
{
    write_header(stream);
    Utils::write_varint(stream, _len);
    swrite(stream, _buf, _len);
}

}
