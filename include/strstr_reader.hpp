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
    Reader(IOIntf *source, const RegistryHandle node_types);
    virtual ~Reader();
private:
    IOIntf *_source;
    const Registry *_node_factory;
    const RegistryHandle _node_factory_handle;

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
};

}

#endif
