#ifndef _STRUCTSTREAM_STREAMING_H
#define _STRUCTSTREAM_STREAMING_H

#include "structstream/streaming_tree.hpp"
#include "structstream/streaming_bitstream.hpp"

namespace StructStream {

ContainerHandle bitstream_to_tree(IOIntfHandle in, RegistryHandle registry);

void tree_to_bitstream(ContainerHandle root, IOIntfHandle out);
void tree_to_bitstream(std::initializer_list<NodeHandle> nodes, IOIntfHandle out);

}

#endif
