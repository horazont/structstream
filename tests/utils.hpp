#ifndef _STRUCTSTREAM_TESTS_UTILS_H
#define _STRUCTSTREAM_TESTS_UTILS_H

#include "structstream/node_primitive.hpp"
#include "structstream/node_container.hpp"
#include "structstream/node_blob.hpp"
#include "structstream/streaming_tree.hpp"
#include "structstream/streaming_bitstream.hpp"

using namespace StructStream;

inline ContainerHandle blob_to_tree(const uint8_t *data, intptr_t data_len)
{
    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, data_len));
    ToTree *tree = new ToTree();

    StreamSink sink_h(tree);
    FromFile reader(io, registry, sink_h);
    reader.read_all();

    return tree->root();
}


#endif
