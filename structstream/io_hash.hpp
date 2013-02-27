/**********************************************************************
File name: io_hash.hpp
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
#ifndef _STRUCTSTREAM_IO_HASH_H
#define _STRUCTSTREAM_IO_HASH_H

#include "structstream/io_base.hpp"
#include "structstream/hashing_base.hpp"

namespace StructStream {

enum HashPipeDirection {
    HP_READ,
    HP_WRITE
};

template <HashPipeDirection dir>
struct HashPipe: public IOIntf {
public:
    HashPipe(IncrementalHash *hash, IOIntfHandle underlying_io):
        _hash(hash),
        _io_h(underlying_io),
        _io(_io_h.get())
    {
    };

    virtual ~HashPipe()
    {
        if (_hash) {
            delete _hash;
        }
    };
private:
    IncrementalHash *_hash;
    IOIntfHandle _io_h;
    IOIntf *_io;
public:
    virtual intptr_t read(void *buf, const intptr_t len) {
        if (_hash == nullptr) {
            return 0;
        }
        if (dir == HP_READ) {
            intptr_t bytes = _io->read(buf, len);
            _hash->feed(buf, bytes);
            return bytes;
        } else {
            return 0;
        }
    };

    virtual intptr_t write(const void *buf, const intptr_t len) {
        if (_hash == nullptr) {
            return 0;
        }
        if (dir == HP_WRITE) {
            intptr_t bytes = _io->write(buf, len);
            _hash->feed(buf, bytes);
            return bytes;
        } else {
            return 0;
        }
    };

    IncrementalHash *get_hash() {
        return _hash;
    };

    IncrementalHash *reclaim_hash() {
        IncrementalHash *hash = _hash;
        _hash = nullptr;
        return hash;
    };

    IOIntfHandle underlying_io() const {
        return _io_h;
    };
};

}

#endif
