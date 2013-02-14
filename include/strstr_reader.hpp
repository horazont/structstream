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

struct ContainerInfo {
    ContainerHandle parent_node;
    intptr_t read_child_count;
    intptr_t expected_children;
    bool armored;
    HashType hash_function;
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

    std::forward_list<ContainerInfo*> _parent_stack;
    ContainerHandle _root;
    ContainerInfo *_curr_parent;
protected:
    void check_end_of_container();
    bool fire_on_node(NodeHandle node);
    void require_open();
protected:
    virtual ContainerInfo* new_container_info() const;
    void start_of_container(ContainerHandle cont_node);
    virtual void proc_container_flags(VarUInt &flags_int, ContainerInfo *info);
    virtual void end_of_container_header(ContainerInfo *info);
    virtual void end_of_container_body(ContainerInfo *info);
    void end_of_container();
public:
    void close();
    void open(IOIntfHandle stream);
public:
    void read_all();
    NodeHandle read_next();
    ContainerHandle root();
};

}

#endif
