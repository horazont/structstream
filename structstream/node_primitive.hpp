/**********************************************************************
File name: node_primitive.hpp
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
#ifndef _STRUCTSTREAM_NODE_PRIMITIVE_H
#define _STRUCTSTREAM_NODE_PRIMITIVE_H

#include "structstream/node_base.hpp"

#include <cstring>

namespace StructStream {

/**
 * Base class for primitive data records.
 */
class DataRecord: public Node {
protected:
    DataRecord(ID id);
    DataRecord(const DataRecord &ref);
public:
    virtual ~DataRecord();
public:
    /**
     * Copy the contents to a buffer.
     *
     * Copy the contents of the record to a buffer given by the
     * user. It is the users responsibility to assure that the buffer
     * is large enough.
     *
     * @param to Buffer to copy the data to.
     *
     * @seealso raw_size
     */
    virtual void raw_get(void *to) const = 0;

    /**
     * Return the size of the data
     *
     * Return the amount of bytes needed to store a copy of the data
     * held by this node.
     *
     * @return Amount of bytes the data takes
     */
    virtual intptr_t raw_size() const = 0;

    /**
     * Set the contents from a buffer
     *
     * Copy the contents of the given buffer and take it as a
     * value. Will only read the amount of bytes needed to fully
     * specify the value. It is the callers responsibility to assure
     * that the buffer handed in to this function is long enough.
     *
     * @param from Buffer to read from
     *
     * @seealso raw_size
     */
    virtual void raw_set(const void *from) = 0;
};

/**
 * Template for primitive C types. Specializations of this template
 * support additional get() and set() methods, which allow compile
 * time type checking by returning/accepting respectively the type
 * stored by the specialization.
 */
template <class _T, RecordType rt>
class PrimitiveDataRecord: public DataRecord {
protected:
    PrimitiveDataRecord(ID id):
        DataRecord::DataRecord(id),
        _data() {}
    PrimitiveDataRecord(const PrimitiveDataRecord<_T, rt> &ref):
        DataRecord::DataRecord(ref),
        _data(ref._data) {}
public:
    virtual ~PrimitiveDataRecord() {}
private:
    _T _data;
public:
    virtual NodeHandle copy() const {
        return NodeHandleFactory< PrimitiveDataRecord<_T, rt> >::copy(*this);
    };

    virtual void raw_get(void *to) const {
        memcpy(to, &_data, sizeof(_T));
    };

    virtual intptr_t raw_size() const {
        return sizeof(_T);
    };

    virtual void raw_set(const void *from) {
        memcpy(&_data, from, sizeof(_T));
    };

    virtual void read(IOIntf *stream) {
	sread(stream, &_data, sizeof(_T));
    };

    virtual void write(IOIntf *stream) const {
        write_header(stream);
        swrite(stream, &_data, sizeof(_T));
    };

    virtual RecordType record_type() const {
        return rt;
    };
public:
    inline _T get() const {
        return _data;
    };

    inline void set(const _T &value) {
        _data = value;
    };

    friend class NodeHandleFactory< PrimitiveDataRecord<_T, rt> >;
};

typedef PrimitiveDataRecord<int32_t, RT_INT32> Int32Record;
typedef PrimitiveDataRecord<uint32_t, RT_UINT32> UInt32Record;
typedef PrimitiveDataRecord<int64_t, RT_INT64> Int64Record;
typedef PrimitiveDataRecord<uint64_t, RT_UINT64> UInt64Record;
typedef PrimitiveDataRecord<float, RT_FLOAT32> Float32Record;
typedef PrimitiveDataRecord<double, RT_FLOAT64> Float64Record;


}

#endif
