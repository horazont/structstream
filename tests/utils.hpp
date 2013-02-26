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

inline intptr_t tree_to_blob(uint8_t *output, intptr_t output_len, std::initializer_list<NodeHandle> children, bool armor = true)
{
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, output_len));
    ToFile *out = new ToFile(io);
    out->set_armor_default(armor);
    FromTree(StreamSink(out), children);
    return static_cast<WritableMemory*>(io.get())->size();
}

inline intptr_t tree_to_blob(uint8_t *output, intptr_t output_len, ContainerHandle root, bool armor = true)
{
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, output_len));
    ToFile *out = new ToFile(io);
    out->set_armor_default(armor);
    FromTree(StreamSink(out), root);
    return static_cast<WritableMemory*>(io.get())->size();
}

#endif
