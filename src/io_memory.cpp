/**********************************************************************
File name: io_memory.cpp
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
#include "structstream/io_memory.hpp"

#include <stdexcept>

#include <cstdlib>
#include <cstring>
#include <cassert>

namespace StructStream {

ReadableMemory::ReadableMemory(const uint8_t *srcbuf, const intptr_t len):
    _buf((uint8_t*)malloc(len)),
    _len(len),
    _offs(0)
{
    memcpy(_buf, srcbuf, len);
}

ReadableMemory::ReadableMemory(const ReadableMemory &ref):
    _buf((uint8_t*)malloc(ref._len)),
    _len(ref._len),
    _offs(0)
{
    memcpy(_buf, ref._buf, _len);
}

ReadableMemory::ReadableMemory(const WritableMemory &ref):
    _buf((uint8_t*)malloc(ref.size())),
    _len(ref.size()),
    _offs(0)
{
    memcpy(_buf, ref.buffer(), _len);
}

ReadableMemory::~ReadableMemory()
{
    free(_buf);
}

ReadableMemory& ReadableMemory::operator=(const ReadableMemory &ref)
{
    free(_buf);
    _buf = (uint8_t*)malloc(ref._len);
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
    _blank_pattern(0xdeadbeef),
    _may_grow(true)
{

}

WritableMemory::WritableMemory(uint8_t *buf, const intptr_t len):
    _buf(buf),
    _buf_size(len),
    _outward_size(0),
    _offs(0),
    _blank_pattern(0),
    _may_grow(false)
{

}

WritableMemory::WritableMemory(const uint32_t blank_pattern):
    _buf(),
    _buf_size(0),
    _outward_size(0),
    _offs(0),
    _blank_pattern(blank_pattern),
    _may_grow(true)
{

}

WritableMemory::~WritableMemory()
{
    if (_buf && _may_grow) {
        free(_buf);
    }
}

void WritableMemory::grow()
{
    assert(_may_grow);
    assert(_buf_size % sizeof(_blank_pattern) == 0);

    const intptr_t new_size = _buf_size + 1024*sizeof(_blank_pattern);
    uint8_t *new_buf = (uint8_t*)realloc(_buf, new_size);
    if (!new_buf) {
        throw std::runtime_error("Out of memory during grow().");
    }

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
    intptr_t to_write = len;
    while (to_write + _offs >= _buf_size)
    {
        if (!_may_grow) {
            to_write = _buf_size - _offs;
            break;
        }
        grow();
    }

    if (to_write == 0) {
        return to_write;
    }

    memcpy((uint8_t*)_buf + _outward_size, buf, to_write);
    _outward_size += to_write;
    _offs += to_write;
    return to_write;
}

}
