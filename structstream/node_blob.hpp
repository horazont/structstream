/**********************************************************************
File name: node_blob.hpp
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
#ifndef _STRUCTSTREAM_NODE_BLOB_H
#define _STRUCTSTREAM_NODE_BLOB_H

#include <string>

#include <cstdlib>
#include <cstring>

#include "structstream/node_primitive.hpp"
#include "structstream/utils.hpp"
#include "structstream/errors.hpp"

namespace StructStream {

/**
 * Base template to store a dynamic-length blob.
 *
 * Adds the set methods which allow reading from a buffer and a
 * std::string instance.
 */
template <class _IntfT>
class BlobDataRecord: public DataRecord {
protected:
    explicit BlobDataRecord(ID id):
        DataRecord::DataRecord(id),
        _buf(),
        _len(0) {};
    BlobDataRecord(const BlobDataRecord<_IntfT> &ref):
        DataRecord::DataRecord(ref),
        _buf(malloc(ref.size())),
        _len(ref._len)
        {
            memcpy(_buf, ref._buf, size());
        }
public:
    virtual ~BlobDataRecord() {
        if (_buf) {
            free(_buf);
            _buf = nullptr;
        }
        _len = 0;
    }
protected:
    void *_buf;
    intptr_t _len;
protected:
    inline intptr_t size() const {
        return _len * sizeof(_IntfT);
    };
protected:
    inline static VarInt read_and_check_length(IOIntf *stream) {
        VarInt length = Utils::read_varint(stream);
        if (length < 0) {
            throw IllegalData("Negative-length blob record.");
        }
        return length;
    };

    inline void allocate_length(VarInt length) {
	if (length != _len) {
	    _len = length;
            void *newbuf = realloc(_buf, size());
            if (!newbuf) {
                free(_buf);
                throw std::runtime_error("out of memory");
            }
            _buf = newbuf;
	}
    };
public:
    virtual void raw_get(void *to) const {
        memcpy(to, _buf, size());
    };

    virtual intptr_t raw_size() const {
        return size();
    };

    virtual void raw_set(const void *from) {
        memcpy(_buf, from, size());
    };

public:
    const _IntfT *dataptr() const {
        return (_IntfT*)(_buf);
    };

    intptr_t datalen() const {
        return _len;
    };

    void set(const _IntfT *from, const intptr_t len) {
        allocate_length(len);
        memcpy(_buf, from, size());
    };

    void set(const std::string &str) {
        set((_IntfT*)str.c_str(), str.size()+1);
    };

    virtual std::string datastr() const {
        return std::string((const char*)_buf, size());
    };
};

/**
 * Implement a UTF8 string record.
 */
class UTF8Record: public BlobDataRecord<char> {
protected:
    explicit UTF8Record(ID id):
        BlobDataRecord<char>::BlobDataRecord(id) {};
    UTF8Record(const UTF8Record &ref):
        BlobDataRecord<char>::BlobDataRecord(ref) {};
public:
    virtual ~UTF8Record();
public:
    virtual NodeHandle copy() const;
    virtual void read(IOIntf *stream);
    virtual void write(IOIntf *stream) const;

    std::string get() const {
        return datastr();
    };

    virtual std::string datastr() const {
        return std::string((const char*)_buf);
    };

    virtual RecordType record_type() const {
        return RT_UTF8STRING;
    };

    friend struct NodeHandleFactory<UTF8Record>;
};

/**
 * Implement a binary blob of arbitrary length with arbitrary
 * contents.
 */
class BlobRecord: public BlobDataRecord<char> {
protected:
    explicit BlobRecord(ID id):
        BlobDataRecord<char>::BlobDataRecord(id) {};
    BlobRecord(const BlobRecord &ref):
        BlobDataRecord<char>::BlobDataRecord(ref) {};
public:
    virtual ~BlobRecord();
public:
    virtual NodeHandle copy() const;
    virtual RecordType record_type() const {
        return RT_BLOB;
    };

    virtual void read(IOIntf *stream);
    virtual void write(IOIntf *stream) const;

    friend struct NodeHandleFactory<BlobRecord>;
};

}

#endif
