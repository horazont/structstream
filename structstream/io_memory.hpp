/**********************************************************************
File name: io_memory.hpp
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
#ifndef _STRUCTSTREAM_IO_MEMORY_H
#define _STRUCTSTREAM_IO_MEMORY_H

#include "structstream/io_base.hpp"

namespace StructStream {

struct WritableMemory;

struct ReadableMemory: public IOIntf {
public:
    ReadableMemory(const uint8_t *srcbuf, const intptr_t len);
    ReadableMemory(const ReadableMemory &ref);
    ReadableMemory(const WritableMemory &ref);
    virtual ~ReadableMemory();

private:
    uint8_t *_buf;
    intptr_t _len;
    intptr_t _offs;

public:
    ReadableMemory& operator=(const ReadableMemory &ref);

    inline const uint8_t *buffer() const { return _buf; };
    inline intptr_t size() const { return _len; };

    virtual intptr_t read(void *buf, const intptr_t len);
    virtual intptr_t write(const void *buf, const intptr_t len);
};

struct WritableMemory: public IOIntf {
public:
    WritableMemory();
    WritableMemory(uint8_t *buf, const intptr_t len);
    WritableMemory(const uint32_t blank_pattern);
    WritableMemory(const ReadableMemory &ref);
    WritableMemory(const WritableMemory &ref);
    virtual ~WritableMemory();
private:
    uint8_t *_buf;
    intptr_t _buf_size;
    intptr_t _outward_size;
    intptr_t _offs;
    uint32_t _blank_pattern;
    bool _may_grow;
private:
    void grow();
public:
    WritableMemory& operator=(const WritableMemory &ref);

    inline const uint8_t *buffer() const { return _buf; };
    inline intptr_t size() const { return _outward_size; };

    virtual intptr_t read(void *buf, const intptr_t len);
    virtual intptr_t write(const void *buf, const intptr_t len);

    uint8_t *release_buffer(intptr_t &len);
};

}

#endif
