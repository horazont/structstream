#include "include/strstr_reader.hpp"

#include "include/strstr_utils.hpp"

namespace StructStream {

Reader::Reader(IOIntf *source, const RegistryHandle node_types):
    _source(source),
    _node_factory(node_types.get()),
    _node_factory_handle(node_types),
    _on_node(),
    _parent_stack(),
    _root(std::static_pointer_cast<Container>(NodeHandleFactory<Container>::create(TreeRootID))),
    _curr_parent()
{
    _push_parent_info(_root, -1);
}

Reader::~Reader() {
    delete _source;
}

bool Reader::_fire_on_node(NodeHandle node) {
    if (_on_node) {
	return _on_node(this, _curr_parent->parent_node, node);
    }
    return true;
}

void Reader::_pop_parent_info(bool notify) {
    ParentInfo old = ParentInfo(*_curr_parent);
    _parent_stack.pop_front();
    _curr_parent = &_parent_stack.front();

    if (notify) {
	_fire_on_node(old.parent_node);
    }
}

void Reader::_push_parent_info(ContainerHandle new_parent, intptr_t expected_children) {
    _parent_stack.push_front(ParentInfo{new_parent, 0, expected_children});
    _curr_parent = &_parent_stack.front();
}

NodeHandle Reader::read_next() {
    if (_curr_parent == nullptr) {
	return NodeHandle();
    }

    RecordType rt = Utils::read_record_type(_source);
    if (rt == RT_RESERVED) {
	throw new std::exception();
    } else if (rt == RT_END_OF_CHILDREN) {
	_pop_parent_info(true);
	return read_next();
    }

    ID id = Utils::read_id(_source);
    if (id == InvalidID) {
	throw new std::exception();
    }
    

    NodeHandle new_node = _node_factory->node_from_record_type(rt, id);
    if (!new_node.get()) {
	throw new std::exception();
    }

    if (rt == RT_CONTAINER) {
	ContainerHandle new_parent = std::static_pointer_cast<Container>(new_node);
	uintptr_t expected_children = new_parent->read_header(_source);
	
	_push_parent_info(new_parent, expected_children);
    } else {
	new_node->read(_source);
	_curr_parent->read_child_count++;
	if (_fire_on_node(new_node)) {
	    _curr_parent->parent_node->child_add(new_node);
	}

	if (_curr_parent->expected_children != 0 
	    && _curr_parent->expected_children <= _curr_parent->read_child_count) 
	{
	    _pop_parent_info(true);
	}
    }

    return new_node;
}

}
