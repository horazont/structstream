/**********************************************************************
File name: strstr_io.hpp
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
#ifndef _STRSTR_IO_H
#define _STRSTR_IO_H

#include <cstdint>

namespace StructStream {

struct IOIntf {
public:
    virtual ~IOIntf() {};
    virtual intptr_t read(void *buf, const intptr_t len) = 0;
    virtual intptr_t write(const void *buf, const intptr_t len) = 0;
};

struct MemoryIO: public IOIntf {
public:
    MemoryIO(const void *srcbuf, const intptr_t len);
    MemoryIO(const MemoryIO &ref);
    virtual ~MemoryIO();
private:
    void* _buf;
    intptr_t _len;
    intptr_t _offs;
public:
    MemoryIO& operator=(const MemoryIO &ref);
    virtual intptr_t read(void *buf, const intptr_t len);
    virtual intptr_t write(const void *buf, const intptr_t len);
};

void sread(IOIntf *io, void *buf, const intptr_t len);
void swrite(IOIntf *io, const void *buf, const intptr_t len);

}

#endif
