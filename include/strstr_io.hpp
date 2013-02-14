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

#include <memory>

#include <cstdint>

namespace StructStream {

struct IOIntf {
public:
    virtual ~IOIntf() {};
    virtual intptr_t read(void *buf, const intptr_t len) = 0;
    virtual intptr_t write(const void *buf, const intptr_t len) = 0;
};

typedef std::shared_ptr<IOIntf> IOIntfHandle;

struct WritableMemory;

struct ReadableMemory: public IOIntf {
public:
    ReadableMemory(const void *srcbuf, const intptr_t len);
    ReadableMemory(const ReadableMemory &ref);
    ReadableMemory(const WritableMemory &ref);
    virtual ~ReadableMemory();
private:
    void* _buf;
    intptr_t _len;
    intptr_t _offs;
public:
    ReadableMemory& operator=(const ReadableMemory &ref);

    inline const void *buffer() const { return _buf; };
    inline const intptr_t size() const { return _len; };

    virtual intptr_t read(void *buf, const intptr_t len);
    virtual intptr_t write(const void *buf, const intptr_t len);
};

struct WritableMemory: public IOIntf {
public:
    WritableMemory();
    WritableMemory(const uint32_t blank_pattern);
    WritableMemory(const ReadableMemory &ref);
    WritableMemory(const WritableMemory &ref);
    virtual ~WritableMemory();
private:
    void *_buf;
    intptr_t _buf_size;
    intptr_t _outward_size;
    intptr_t _offs;
    uint32_t _blank_pattern;
private:
    void grow();
public:
    WritableMemory& operator=(const WritableMemory &ref);

    inline const void *buffer() const { return _buf; };
    inline const intptr_t size() const { return _outward_size; };

    virtual intptr_t read(void *buf, const intptr_t len);
    virtual intptr_t write(const void *buf, const intptr_t len);
};

void sread(IOIntf *io, void *buf, const intptr_t len);
void swrite(IOIntf *io, const void *buf, const intptr_t len);

template <class _T>
inline void swritev(IOIntf *io, const _T value)
{
    swrite(io, &value, sizeof(_T));
}

}

#endif
