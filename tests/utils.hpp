/**********************************************************************
File name: utils.hpp
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
#ifndef _STRUCTSTREAM_TESTS_UTILS_H
#define _STRUCTSTREAM_TESTS_UTILS_H

#include "structstream/node_primitive.hpp"
#include "structstream/node_container.hpp"
#include "structstream/node_blob.hpp"
#include "structstream/streaming.hpp"

using namespace StructStream;

inline ContainerHandle blob_to_tree(const uint8_t *data, intptr_t data_len)
{
    RegistryHandle registry = RegistryHandle(new Registry());
    IOIntfHandle io = IOIntfHandle(new ReadableMemory(data, data_len));
    return bitstream_to_tree(io, registry);
}

inline intptr_t tree_to_blob(uint8_t *output, intptr_t output_len, std::initializer_list<NodeHandle> children, bool armor = true)
{
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, output_len));
    tree_to_bitstream(children, io, armor);
    return static_cast<WritableMemory*>(io.get())->size();
}

inline intptr_t tree_to_blob(uint8_t *output, intptr_t output_len, ContainerHandle root, bool armor = true)
{
    IOIntfHandle io = IOIntfHandle(new WritableMemory(output, output_len));
    tree_to_bitstream(root, io, armor);
    return static_cast<WritableMemory*>(io.get())->size();
}

inline void hexdump(const uint8_t *buf, intptr_t buf_len)
{
    const uint8_t *const end = buf + buf_len;
    for (; buf != end; buf++)
    {
        const uint8_t lo = (*buf) & 0x0F;
        const uint8_t hi = ((*buf) & 0xF0) >> 4;
        printf("%x%x ", hi, lo);
    }
}

#endif
