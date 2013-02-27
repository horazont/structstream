/**********************************************************************
File name: io_std.cpp
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
#include "structstream/io_std.hpp"

namespace StructStream {

/* StructStream::StandardInputStream */

StandardInputStream::StandardInputStream(std::istream &in):
    _in(in)
{

}

intptr_t StandardInputStream::read(void *buf, const intptr_t len)
{
    _in.read((char*)buf, len);
    if (!_in.good()) {
        return 0;
    } else {
        return len;
    }
}

intptr_t StandardInputStream::write(const void*, const intptr_t)
{
    return 0;
}

/* StructStream::StandardOutputStream */

StandardOutputStream::StandardOutputStream(std::ostream &out):
    _out(out)
{

}

intptr_t StandardOutputStream::read(void*, const intptr_t)
{
    return 0;
}

intptr_t StandardOutputStream::write(const void *buf, const intptr_t len)
{
    _out.write((const char*)buf, len);
    if (!_out.good()) {
        return 0;
    } else {
        return len;
    }
}

}
