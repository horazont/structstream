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

namespace {

template <typename int_t>
struct bswap_impl;

template <>
struct bswap_impl<int32_t>
{
    static inline int32_t bswap(const int32_t value)
    {
        return __builtin_bswap32(value);
    };
};

template <>
struct bswap_impl<int64_t>
{
    static inline int64_t bswap(const int64_t value)
    {
        return __builtin_bswap64(value);
    };
};

template <typename A, typename enable = void>
struct endianess;

template<typename A>
struct endianess<A, typename std::enable_if<sizeof(A) != sizeof(int8_t)>::type>
{
    typedef typename std::conditional<
        sizeof(A) == sizeof(int32_t), int32_t,
        typename std::conditional<
            sizeof(A) == sizeof(int64_t), int64_t,
            void
            >::type
        >::type int_t;

    static_assert(!std::is_same<int_t, void>::value,
                  "Can only deal with 8, 32 or 64-bit values.");

    static inline void bswap(A &value)
    {
        union {
            A orig;
            int_t buf;
        } converter;
        converter.orig = value;
        converter.buf = bswap_impl<int_t>::bswap(converter.buf);
        value = converter.orig;
    };
};

template<typename A>
struct endianess<A, typename std::enable_if<sizeof(A) == sizeof(int8_t)>::type>
{
    static inline void bswap(A &value)
    {
    };
};

}

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

    typedef endianess<_T> endian_helper;

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
            sread(stream, &_data, sizeof(_T));
            endian_helper::bswap(_data);
        } else {
            sreadv<_T>(stream, &_data);
        }
    };

    virtual void write(IOIntf *stream) const {
        write_header(stream);
        if (Utils::is_big_endian) {
            _T tmp = _data;
            endian_helper::bswap(tmp);
            swrite(stream, &tmp, sizeof(_T));
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

    EnumRecordTpl(const EnumRecordTpl& ref):
        underlying_record_type(ref)
    {

    };

    EnumRecordTpl& operator=(const EnumRecordTpl& ref)
    {
        underlying_record_type::operator=(ref);
        return *this;
    };
public:
    virtual NodeHandle copy() const {
        return NodeHandleFactory<EnumRecordTpl<enum_t, rt, underlying_record_type>>::copy(*this);
    };

    inline enum_t get() const {
        return (enum_t)this->_data;
    };

    inline void set(const enum_t& value) {
        this->_data = (int_t)value;
    };

    virtual RecordType record_type() const {
        return rt;
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

template <size_t len, RecordType rt, typename char_t = uint8_t>
class StaticByteArrayRecord: public DataRecord
{
    static_assert(
        sizeof(char_t) == 1,
        "Character type for static byte array record must have size 1");

protected:
    explicit StaticByteArrayRecord(ID id):
        DataRecord::DataRecord(id),
        _data()
    {

    }

    StaticByteArrayRecord(const StaticByteArrayRecord &ref):
        DataRecord::DataRecord(ref),
        _data()
    {
        memcpy(&_data[0], &ref._data[0], len);
    }

protected:
    char_t _data[len];

public:
    NodeHandle copy() const override {
        return NodeHandleFactory<StaticByteArrayRecord>::copy(*this);
    };

    void raw_get(void *to) const override {
        memcpy(to, &_data[0], len);
    };

    intptr_t raw_size() const override {
        return len;
    };

    void raw_set(const void *from) override {
        memcpy(&_data[0], from, len);
    };

    void read(IOIntf *stream) override {
        sread(stream, &_data[0], len);
    };

    void write(IOIntf *stream) const override {
        write_header(stream);
        swrite(stream, &_data[0], len);
    };

    RecordType record_type() const override {
        return rt;
    };

    friend struct NodeHandleFactory<StaticByteArrayRecord>;

};

typedef StaticByteArrayRecord<16, RT_RAW128> Raw128Record;

class BoolRecord: public BoolRecordBase
{
protected:
    explicit BoolRecord(ID id);
    BoolRecord(const BoolRecord &ref);
public:
    virtual ~BoolRecord() {};
public:
    virtual NodeHandle copy() const;
    virtual void read(IOIntf *stream);
    virtual void write(IOIntf *stream) const;
    virtual RecordType record_type() const;
public:
    friend struct NodeHandleFactory< BoolRecord >;
};

}

#endif
