#include "structstream/node_varint.hpp"

namespace StructStream {

/* StructStream::VarIntRecord */

VarIntRecord::VarIntRecord(ID id):
    PrimitiveDataRecord<VarInt, RT_VARINT>(id)
{

}

NodeHandle VarIntRecord::copy() const
{
    return NodeHandleFactory<VarIntRecord>::copy(*this);
}

void VarIntRecord::read(IOIntf *stream)
{
    _data = Utils::read_varint(stream);
}

void VarIntRecord::write(IOIntf *stream) const
{
    Utils::write_varint(stream, _data);
}

/* StructStream::VarUIntRecord */

VarUIntRecord::VarUIntRecord(ID id):
    PrimitiveDataRecord<VarUInt, RT_VARUINT>(id)
{

}

NodeHandle VarUIntRecord::copy() const
{
    return NodeHandleFactory<VarUIntRecord>::copy(*this);
}

void VarUIntRecord::read(IOIntf *stream)
{
    _data = Utils::read_varuint(stream);
}

void VarUIntRecord::write(IOIntf *stream) const
{
    Utils::write_varuint(stream, _data);
}


}
