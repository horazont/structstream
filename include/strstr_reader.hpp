/**********************************************************************
File name: strstr_reader.hpp
This file is part of: ebml++

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

For feedback and questions about ebml++ please e-mail one of the authors
named in the AUTHORS file.
**********************************************************************/
#ifndef _STRSTR_READER_H
#define _STRSTR_READER_H

#include <functional>
#include <forward_list>

#include "include/strstr_nodes.hpp"
#include "include/strstr_registry.hpp"
#include "include/strstr_io.hpp"

namespace StructStream {

class Reader;

typedef std::function<bool (Reader *reader, ContainerHandle parent, NodeHandle node)> ReaderOnNode;

struct ParentInfo {
    ContainerHandle parent_node;
    intptr_t read_child_count;
    intptr_t expected_children;
};

class Reader {
public:
    Reader(IOIntfHandle source, const RegistryHandle node_types);
    virtual ~Reader();
private:
    IOIntf *_source;
    IOIntfHandle _source_h;
    const Registry *_node_factory;
    const RegistryHandle _node_factory_h;

    ReaderOnNode _on_node;

    std::forward_list<ParentInfo> _parent_stack;
    ContainerHandle _root;
    ParentInfo *_curr_parent;
protected:
    bool _fire_on_node(NodeHandle node);
    void _pop_parent_info(bool notify);
    void _push_parent_info(ContainerHandle new_parent, intptr_t expected_children);
public:
    void read_all();
    NodeHandle read_next();
    ContainerHandle root();
};

}

#endif
