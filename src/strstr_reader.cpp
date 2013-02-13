/**********************************************************************
File name: strstr_reader.cpp
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
#include "include/strstr_reader.hpp"

#include "include/strstr_utils.hpp"

namespace StructStream {

Reader::Reader(IOIntfHandle source, const RegistryHandle node_types):
    _source(source.get()),
    _source_h(source),
    _node_factory(node_types.get()),
    _node_factory_h(node_types),
    _on_node(),
    _parent_stack(),
    _root(std::static_pointer_cast<Container>(NodeHandleFactory<Container>::create(TreeRootID))),
    _curr_parent()
{
    _push_parent_info(_root, -1);
}

Reader::~Reader() {

}

bool Reader::_fire_on_node(NodeHandle node) {
    if (_on_node) {
	return _on_node(this, (_curr_parent ? _curr_parent->parent_node : ContainerHandle()), node);
    }
    return true;
}

void Reader::_pop_parent_info(bool notify) {
    ParentInfo old = ParentInfo(*_curr_parent);
    _parent_stack.pop_front();

    if (_parent_stack.empty()) {
        _curr_parent = nullptr;
    } else {
        _curr_parent = &_parent_stack.front();
    }

    if (notify) {
        if (_curr_parent) {
            _curr_parent->read_child_count++;
        }
	if (_fire_on_node(old.parent_node)) {
            if (_curr_parent) {
                _curr_parent->parent_node->child_add(old.parent_node);
            }
        }
    }

    // printf("pop %lx; new is %lx\n",
    //        (uint64_t)old.parent_node.get(),
    //        (_curr_parent != nullptr ? (uint64_t)_curr_parent->parent_node.get() : 0));
}

void Reader::_push_parent_info(ContainerHandle new_parent, intptr_t expected_children) {
    _parent_stack.push_front(ParentInfo{new_parent, 0, expected_children});
    _curr_parent = &_parent_stack.front();
    // printf("push %lx\n", (uint64_t)_curr_parent->parent_node.get());
}

NodeHandle Reader::read_next() {
    if (_curr_parent == nullptr) {
	return NodeHandle();
    }

    RecordType rt = Utils::read_record_type(_source);
    if (rt == RT_RESERVED) {
	throw new std::exception();
    } else if (rt == RT_END_OF_CHILDREN) {
        // printf("end of children encountered\n");
        if (_curr_parent) {
            if (_curr_parent->expected_children != -1) {
                // we MUST NOT end up here if the parent specified a size
                throw new std::exception();
            }
        }
	_pop_parent_info(true);
	return read_next();
    }

    ID id = Utils::read_id(_source);
    if (id == InvalidID) {
	throw new std::exception();
    }

    // printf("found 0x%x with id 0x%lx\n", rt, id);

    NodeHandle new_node = _node_factory->node_from_record_type(rt, id);
    if (!new_node.get()) {
	throw new std::exception();
    }

    ContainerHandle new_parent = std::dynamic_pointer_cast<Container>(new_node);
    if (new_parent.get() != nullptr) {
	uintptr_t expected_children = new_parent->read_header(_source);

	_push_parent_info(new_parent, expected_children);
    } else {
	new_node->read(_source);

        _curr_parent->read_child_count++;
        if (_fire_on_node(new_node)) {
            _curr_parent->parent_node->child_add(new_node);
        }
    }

    // printf("%ld out of %ld children found\n",
    //        _curr_parent->read_child_count,
    //        _curr_parent->expected_children);
    if (_curr_parent->expected_children != -1
        && _curr_parent->expected_children <= _curr_parent->read_child_count)
    {
        _pop_parent_info(true);
    }

    return new_node;
}

void Reader::read_all()
{
    NodeHandle node = read_next();
    while (node.get() != nullptr) {
        node = read_next();
    }
}

}
