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
#include <cassert>

#include "structstream/utils.hpp"

namespace StructStream {

/**
 * Base class for primitive data records.
 */
class DataRecord: public Node {
protected:
    explicit DataRecord(ID id);
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
    static_assert(
        (sizeof(_T) == 1) ||
        (sizeof(_T) == 4) ||
        (sizeof(_T) == 8),
        "PrimitiveDataRecord only supports 1, 4 or 8 byte wide types.");

protected:
    explicit PrimitiveDataRecord(ID id):
        DataRecord::DataRecord(id),
        _data() {}
    PrimitiveDataRecord(const PrimitiveDataRecord<_T, rt> &ref):
        DataRecord::DataRecord(ref),
        _data(ref._data) {}
public:
    virtual ~PrimitiveDataRecord() {}
protected:
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
        if (Utils::is_big_endian && (sizeof(_T) > 1)) {
            uint8_t buf[sizeof(_T)];
            sread(stream, buf, sizeof(_T));

            switch (sizeof(_T)) {
            case 4:
                _data = (_T)(__builtin_bswap32(*reinterpret_cast<uint32_t*>(buf)));
                break;
            case 8:
                _data = (_T)(__builtin_bswap64(*reinterpret_cast<uint64_t*>(buf)));
                break;
            default:
                assert(false);
            };
        } else {
            sreadv<_T>(stream, &_data);
        }
    };

    virtual void write(IOIntf *stream) const {
        write_header(stream);
        if (Utils::is_big_endian) {
            uint8_t buf[sizeof(_T)];

            switch (sizeof(_T)) {
            case 4:
                *((_T*)buf) = (_T)(__builtin_bswap32(*reinterpret_cast<const uint32_t*>(&_data)));
                break;
            case 8:
                *((_T*)buf) = (_T)(__builtin_bswap64(*reinterpret_cast<const uint64_t*>(&_data)));
                break;
            default:
                assert(false);
            };

            swrite(stream, buf, sizeof(_T));
        } else {
            swritev<_T>(stream, _data);
        }
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

    friend struct NodeHandleFactory< PrimitiveDataRecord<_T, rt> >;
};

template <typename enum_t, RecordType rt,
          typename underlying_record_type =
              PrimitiveDataRecord<typename std::underlying_type<enum_t>::type, rt>>
class EnumRecordTpl: public underlying_record_type
{
public:
    typedef decltype(std::declval<underlying_record_type>().get()) int_t;
public:
    EnumRecordTpl(ID id):
        underlying_record_type(id)
    {

    };

    EnumRecordTpl(const EnumRecordTpl& ref)
    {
    };
    EnumRecordTpl& operator=(const EnumRecordTpl& ref);
public:
    inline enum_t get() const {
        return (enum_t)this->_data;
    };

    inline void set(const enum_t& value) {
        this->_data = (int_t)value;
    };

    friend struct NodeHandleFactory< EnumRecordTpl<enum_t, rt, underlying_record_type> >;
};

typedef PrimitiveDataRecord<int32_t, RT_INT32> Int32Record;
typedef PrimitiveDataRecord<uint32_t, RT_UINT32> UInt32Record;
typedef PrimitiveDataRecord<int64_t, RT_INT64> Int64Record;
typedef PrimitiveDataRecord<uint64_t, RT_UINT64> UInt64Record;
typedef PrimitiveDataRecord<float, RT_FLOAT32> Float32Record;
typedef PrimitiveDataRecord<double, RT_FLOAT64> Float64Record;
typedef PrimitiveDataRecord<bool, RT_BOOL_FALSE> BoolRecordBase;

class BoolRecord: public BoolRecordBase
{
protected:
    explicit BoolRecord(ID id);
    BoolRecord(const BoolRecord &ref);
public:
    virtual void read(IOIntf *stream);
    virtual void write(IOIntf *stream) const;
    virtual RecordType record_type() const;
public:
    friend struct NodeHandleFactory< BoolRecord >;
};

}

#endif
