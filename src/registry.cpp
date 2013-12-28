/**********************************************************************
File name: registry.cpp
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
#include "structstream/registry.hpp"

#include "structstream/node_container.hpp"
#include "structstream/node_primitive.hpp"
#include "structstream/node_blob.hpp"
#include "structstream/node_varint.hpp"

namespace StructStream {

template <bool value>
NodeHandle create_boolean(ID id)
{
    NodeHandle node = NodeHandleFactory<BoolRecord>::create(id);
    static_cast<BoolRecord*>(node.get())->set(value);
    return node;
}

Registry::Registry():
    _record_types()
{
    register_defaults();
}

Registry::Registry(const Registry &ref):
    _record_types(ref._record_types)
{

}

Registry::Registry(
        const std::initializer_list<std::pair<RecordType, NodeConstructor>> &initial,
        bool add_defaults):
    _record_types()
{
    if (add_defaults) {
        register_defaults();
    }
    for (auto &to_add: initial)
    {
        register_record_type(to_add.first, to_add.second);
    }
}

Registry::~Registry()
{

}

void Registry::register_defaults() {
    _record_types[RT_UINT32] = NodeHandleFactory<UInt32Record>::create;
    _record_types[RT_INT32] = NodeHandleFactory<Int32Record>::create;
    _record_types[RT_UINT64] = NodeHandleFactory<UInt64Record>::create;
    _record_types[RT_INT64] = NodeHandleFactory<Int64Record>::create;
    _record_types[RT_FLOAT32] = NodeHandleFactory<Float32Record>::create;
    _record_types[RT_FLOAT64] = NodeHandleFactory<Float64Record>::create;
    _record_types[RT_UTF8STRING] = NodeHandleFactory<UTF8Record>::create;
    _record_types[RT_BLOB] = NodeHandleFactory<BlobRecord>::create;
    _record_types[RT_BOOL_FALSE] = create_boolean<false>;
    _record_types[RT_BOOL_TRUE] = create_boolean<true>;
    _record_types[RT_CONTAINER] = NodeHandleFactory<Container>::create;
    _record_types[RT_VARINT] = NodeHandleFactory<VarIntRecord>::create;
    _record_types[RT_VARUINT] = NodeHandleFactory<VarUIntRecord>::create;
    _record_types[RT_RAW128] = NodeHandleFactory<Raw128Record>::create;
}

NodeHandle Registry::node_from_record_type(RecordType rt, ID id) const
{
    auto found = _record_types.find(rt);
    if (found == _record_types.end()) {
        return NodeHandle();
    }

    return (found->second)(id);
}

void Registry::register_record_type(
    RecordType rt,
    const NodeConstructor &constructor)
{
    _record_types[rt] = constructor;
}

}
