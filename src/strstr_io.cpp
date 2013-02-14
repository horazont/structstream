/**********************************************************************
File name: strstr_io.cpp
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
#include "include/strstr_io.hpp"

#include <cstring>
#include <cstdlib>
#include <cassert>

namespace StructStream {

ReadableMemory::ReadableMemory(const void *srcbuf, const intptr_t len):
    _buf(malloc(len)),
    _len(len),
    _offs(0)
{
    memcpy(_buf, srcbuf, len);
}

ReadableMemory::ReadableMemory(const ReadableMemory &ref):
    _buf(malloc(ref._len)),
    _len(ref._len),
    _offs(0)
{
    memcpy(_buf, ref._buf, _len);
}

ReadableMemory::~ReadableMemory()
{
    free(_buf);
}

ReadableMemory& ReadableMemory::operator=(const ReadableMemory &ref)
{
    free(_buf);
    _buf = malloc(ref._len);
    _len = ref._len;
    _offs = ref._offs;
    memcpy(_buf, ref._buf, _len);
    return *this;
}

intptr_t ReadableMemory::read(void *buf, const intptr_t len)
{
    intptr_t to_read = len;
    if (_offs + len > _len) {
	to_read = _len - _offs;
	if (to_read == 0) {
	    return 0;
	}
    }
    memmove(buf, &((unsigned char*)_buf)[_offs], to_read);
    _offs += to_read;
    return to_read;
}

intptr_t ReadableMemory::write(const void *buf, const intptr_t len)
{
    return 0;
}

/* StructStream::WritableMemory */

WritableMemory::WritableMemory():
    _buf(),
    _buf_size(0),
    _outward_size(0),
    _offs(0),
    _blank_pattern(0xdeadbeef)
{

}

WritableMemory::WritableMemory(const uint32_t blank_pattern):
    _buf(),
    _buf_size(0),
    _outward_size(0),
    _offs(0),
    _blank_pattern(blank_pattern)
{

}

WritableMemory::~WritableMemory()
{
    if (_buf) {
        free(_buf);
    }
}

void WritableMemory::grow()
{
    assert(_buf_size % sizeof(_blank_pattern) == 0);

    const intptr_t new_size = _buf_size + 1024*sizeof(_blank_pattern);
    _buf = realloc(_buf, new_size);

    uint32_t *fillat = (uint32_t*)(&((uint8_t*)_buf)[_buf_size]);
    const uint32_t *end = (uint32_t*)(&((uint8_t*)_buf)[new_size]);
    while (fillat < end) {
        *fillat = _blank_pattern;
    }

    _buf_size = new_size;
}

intptr_t WritableMemory::read(void*, const intptr_t)
{
    return 0;
}

intptr_t WritableMemory::write(const void *buf, const intptr_t len)
{
    while (len + _offs >= _buf_size)
    {
        grow();
    }

    memcpy((uint8_t*)_buf + _outward_size, buf, len);
    _outward_size += len;
    _offs += len;
    return len;
}

void sread(IOIntf *io, void *buf, const intptr_t len)
{
    intptr_t read_bytes = io->read(buf, len);
    if (read_bytes < len) {
	// FIXME: throw an error
	assert(false);
    }
}

void swrite(IOIntf *io, void *buf, const intptr_t len)
{
    intptr_t written_bytes = io->write(buf, len);
    if (written_bytes < len) {
	// FIXME: throw an error
	assert(false);
    }
}

}
