/**********************************************************************
File name: reader.cpp
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
#include "structstream/reader.hpp"

#include "structstream/utils.hpp"
#include "structstream/errors.hpp"

namespace StructStream {

Reader::Reader(IOIntfHandle source, const RegistryHandle node_types):
    _source(),
    _source_h(),
    _node_factory(node_types.get()),
    _node_factory_h(node_types),
    _on_node(),
    _parent_stack(),
    _root(),
    _curr_parent()
{
    open(source);
}

Reader::~Reader()
{
    if (_source)
        close();
}

void Reader::check_end_of_container()
{
    if (!_curr_parent)
        return;

    // printf("%ld out of %ld children found\n",
    //        _curr_parent->read_child_count,
    //        _curr_parent->expected_children);

    if (!_curr_parent->armored
        && _curr_parent->expected_children == _curr_parent->read_child_count)
    {
        end_of_container();
    }
}

bool Reader::fire_on_node(NodeHandle node)
{
    if (_on_node) {
	return _on_node(
            this,
            (_curr_parent ? _curr_parent->parent_node : ContainerHandle()),
            node);
    }
    return true;
}

void Reader::require_open()
{
    if (!_source) {
        throw AlreadyClosed("This operation is not allowed on closed Reader:s.");
    }
}

ContainerInfo* Reader::new_container_info() const
{
    return new ContainerInfo();
}

void Reader::start_of_container(ContainerHandle cont_node)
{
    VarUInt flags_int = Utils::read_varuint(_source);
    ContainerInfo *info = new_container_info();
    info->parent_node = cont_node;
    info->read_child_count = 0;

    proc_container_flags(flags_int, info);

    if (flags_int != 0) {
        delete info;
        throw UnsupportedContainerFlags("Unsupported container flags encountered.");
    }

    try {
        end_of_container_header(info);
    } catch (...) {
        delete info;
        throw;
    }

    _parent_stack.push_front(info);
    _curr_parent = info;

    // printf("push %lx\n", (uint64_t)_curr_parent->parent_node.get());
}

void Reader::proc_container_flags(VarUInt &flags_int, ContainerInfo *info)
{
    info->expected_children = -1;
    info->hash_function = HT_INVALID;
    info->armored = false;

    if ((flags_int & CF_WITH_SIZE) != 0) {
        flags_int ^= CF_WITH_SIZE;
        info->expected_children = Utils::read_varint(_source);
    }
    if ((flags_int & CF_ARMORED) != 0) {
        flags_int ^= CF_ARMORED;
        info->armored = true;
    }
    if (!info->armored && (info->expected_children == -1)) {
        throw IllegalCombinationOfFlags("Illegal combination of container flags: no CF_WITH_SIZE, but no CF_ARMORED either -- how am I supposed to find out the length?");
    }
    if ((flags_int & CF_HASHED) != 0) {
	flags_int ^= CF_HASHED;
        info->hash_function = static_cast<HashType>(Utils::read_varint(_source));
    }
}

void Reader::end_of_container_header(ContainerInfo *info)
{
    if (info->hash_function != HT_INVALID) {
        throw UnsupportedHashFunction("Unsupported hash function.");
    }
}

void Reader::end_of_container_body(ContainerInfo *info)
{

}

void Reader::end_of_container()
{
    ContainerInfo *info = _curr_parent;
    _parent_stack.pop_front();
    if (_parent_stack.empty()) {
        _curr_parent = nullptr;
    } else {
        _curr_parent = _parent_stack.front();
    }

    if (_curr_parent) {
        // Do not call this virtual method for the root node
        end_of_container_body(info);
    }

    if (_curr_parent) {
        _curr_parent->read_child_count += 1;
    }

    bool keep_in_dom = fire_on_node(info->parent_node);
    if (keep_in_dom && _curr_parent) {
        _curr_parent->parent_node->child_add(info->parent_node);
    }

    // printf("pop %lx; new is %lx\n",
    //        (uint64_t)info->parent_node.get(),
    //        (_curr_parent != nullptr ? (uint64_t)_curr_parent->parent_node.get() : 0));

    delete info;

    check_end_of_container();
}

void Reader::close()
{
    require_open();

    _source = nullptr;
    _source_h = IOIntfHandle();

    _on_node = nullptr;
    _root = ContainerHandle();

    for (auto it = _parent_stack.begin();
         it != _parent_stack.end();
         it++)
    {
        delete (*it);
    }
    _parent_stack.clear();
    _curr_parent = nullptr;
}

void Reader::open(IOIntfHandle stream)
{
    if (_source) {
        throw AlreadyOpen("This operation is not allowed on open Reader:s.");
    }

    _source_h = stream;
    _source = _source_h.get();

    _root = std::static_pointer_cast<Container>(
        NodeHandleFactory<Container>::create(TreeRootID)
        );

    ContainerInfo *info = new ContainerInfo();
    info->parent_node = _root;
    info->armored = true;
    info->expected_children = -1;
    info->hash_function = HT_INVALID;

    _parent_stack.push_front(info);
    _curr_parent = info;
}

NodeHandle Reader::read_next() {
    require_open();

    if (_curr_parent == nullptr) {
	return NodeHandle();
    }

    RecordType rt = Utils::read_record_type(_source);
    if (rt == RT_RESERVED) {
	throw UnsupportedRecordType("RT_RESERVED encountered. This stream may have been created with a newer version of structstream.");
    } else if (rt == RT_END_OF_CHILDREN) {
        // printf("end of children encountered\n");

        if (_curr_parent->armored &&
            (_curr_parent->expected_children == -1
             || _curr_parent->expected_children == _curr_parent->read_child_count)
            )
        {
            // EOC is only valid if container has CF_ARMORED flag
            end_of_container();
        } else {
            if (_curr_parent->armored) {
                throw UnexpectedEndOfChildren("Armored container ended unexpectedly (not all announced children found).");
            } else {
                throw UnexpectedEndOfChildren("Non-armored container closed by End-Of-Children tag. This may also imply that some children are missing.");
            }
        }
	return read_next();
    }

    if (_curr_parent->armored
        && _curr_parent->expected_children != -1
        && _curr_parent->read_child_count > _curr_parent->expected_children)
    {
        throw MissingEndOfChildren("CF_ARMORED | CF_WITH_SIZE container without EOC marker.");
    }


    ID id = Utils::read_id(_source);
    if (id == InvalidID) {
	throw InvalidIDError("Invalid object ID encountered.");
    }

    // printf("found 0x%x with id 0x%lx\n", rt, id);

    NodeHandle new_node = _node_factory->node_from_record_type(rt, id);
    if (!new_node.get()) {
	throw UnsupportedRecordType("Unsupported record type.");
    }

    ContainerHandle new_parent = std::dynamic_pointer_cast<Container>(new_node);
    if (new_parent.get() != nullptr) {
        start_of_container(new_parent);
    } else {
	new_node->read(_source);

        _curr_parent->read_child_count++;
        if (fire_on_node(new_node)) {
            _curr_parent->parent_node->child_add(new_node);
        }
    }

    check_end_of_container();

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
