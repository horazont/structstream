/**********************************************************************
File name: streaming.hpp
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
#ifndef _STRUCTSTREAM_STREAMING_H
#define _STRUCTSTREAM_STREAMING_H

#include "structstream/streaming_tree.hpp"
#include "structstream/streaming_bitstream.hpp"

namespace StructStream {

ContainerHandle bitstream_to_tree(IOIntfHandle in,
                                  RegistryHandle registry = RegistryHandle());

void tree_to_bitstream(ContainerHandle root, IOIntfHandle out,
                       bool armor = true);
void tree_to_bitstream(std::initializer_list<NodeHandle> nodes, IOIntfHandle out,
                       bool armor = true);

}

#endif
