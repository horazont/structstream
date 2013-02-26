#include "structstream/streaming.hpp"

namespace StructStream {

ContainerHandle bitstream_to_tree(IOIntfHandle in, RegistryHandle registry)
{
    ToTree *sink = new ToTree();
    StreamSink sink_h(sink);

    FromFile reader(in, registry, sink_h);
    reader.read_all();
    return sink->root();
}

void tree_to_bitstream(ContainerHandle root, IOIntfHandle out)
{
    FromTree(StreamSink(new ToFile(out)), root);
}

void tree_to_bitstream(std::initializer_list<NodeHandle> nodes, IOIntfHandle out)
{
    FromTree(StreamSink(new ToFile(out)), nodes);
}

}
