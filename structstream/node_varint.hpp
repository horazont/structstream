#ifndef _STRUCTSTREAM_NODE_VARINT_H
#define _STRUCTSTREAM_NODE_VARINT_H

#include "structstream/node_primitive.hpp"

namespace StructStream {

class VarIntRecord: public PrimitiveDataRecord<VarInt, RT_VARINT> {
protected:
    explicit VarIntRecord(ID id);
    VarIntRecord(const VarIntRecord &ref) = default;
public:
    virtual ~VarIntRecord() = default;
public:
    virtual NodeHandle copy() const;
    virtual void read(IOIntf *stream);
    virtual void write(IOIntf *stream) const;

    friend struct NodeHandleFactory<VarIntRecord>;
};

class VarUIntRecord: public PrimitiveDataRecord<VarUInt, RT_VARUINT> {
protected:
    explicit VarUIntRecord(ID id);
    VarUIntRecord(const VarUIntRecord &ref) = default;
public:
    virtual ~VarUIntRecord() = default;
public:
    virtual NodeHandle copy() const;
    virtual void read(IOIntf *stream);
    virtual void write(IOIntf *stream) const;

    friend struct NodeHandleFactory<VarUIntRecord>;
};

}

#endif
