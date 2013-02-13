#include "include/strstr_registry.hpp"

namespace StructStream {

Registry::Registry():
    _record_types()
{
    register_defaults();
}

Registry::Registry(const Registry &ref):
    _record_types(ref._record_types)
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
    // _record_types[RT_BOOLEAN_FALSE] = NodeHandleFactory<
    _record_types[RT_CONTAINER] = NodeHandleFactory<Container>::create;
}

NodeHandle Registry::node_from_record_type(RecordType rt, ID id) const
{
    auto found = _record_types.find(rt);
    if (found == _record_types.end()) {
        return NodeHandle();
    }

    return (found->second)(id);
}

}
