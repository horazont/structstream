/**********************************************************************
File name: strstr_nodes.cpp
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
#include "include/strstr_nodes.hpp"

namespace StructStream {

/* StructStream::Node */

Node::Node(ID id):
    _self(),
    _id(id),
    _parent()
{

}

Node::Node(const Node &ref):
    _self(),
    _id(ref._id),
    _parent()
{

}

void detach_from_parent() {

}

/* StructStream::Container */

Container::Container(ID id):
    Node::Node(id),
    _children()
{

}

Container::Container(const Container &ref):
    Node::Node(ref),
    _children()
{
    for (auto it = ref.children_cbegin(); it != ref.children_cend(); it++) {
        child_add((*it)->copy());
    }
}

Container::~Container()
{

}

void Container::_check_valid_child(NodeHandle child)
{
    if (child->parent().get() != nullptr) {
        throw new std::exception(); //("node cannot have multiple parents. un-parent first!");
    }
}

void Container::child_add(NodeHandle child)
{
    _check_valid_child(child);
    _children.push_back(child);
    child->set_parent(std::static_pointer_cast<Container>(_self.lock()));
}

void Container::child_insert_before(NodeVector::iterator &ref, NodeHandle child)
{
    _check_valid_child(child);
    _children.insert(ref, child);
    child->set_parent(std::static_pointer_cast<Container>(_self.lock()));
}

NodeVector::iterator Container::children_begin()
{
    return _children.begin();
}

NodeVector::const_iterator Container::children_cbegin() const
{
    return _children.cbegin();
}

NodeVector::iterator Container::children_end()
{
    return _children.end();
}

NodeVector::const_iterator Container::children_cend() const
{
    return _children.cend();
}

void Container::child_erase(NodeVector::iterator &to_remove)
{
    NodeHandle child = *to_remove;
    if ((*to_remove)->parent().get() != this) {
        throw std::exception();
    }
    _children.erase(to_remove);
    child->set_parent(nullptr);
}

NodeHandle Container::copy() const
{
    return NodeHandleFactory<Container>::copy(*this);
}

intptr_t Container::read_header(IOIntf *stream)
{
    VarUInt flags_int = Utils::read_varuint(stream);
    intptr_t expected_children = 0;
    if ((flags_int & CF_WITH_SIZE) != 0) {
	expected_children = Utils::read_varint(stream);
    }
    if ((flags_int & CF_SIGNED) != 0) {
	throw new std::exception();
    }
    if ((flags_int & CF_ENCRYPTED) != 0) {
	throw new std::exception();
    }
    return expected_children;
}

// void Container::read_footer(IOIntf *stream)
// {
//     if (_expect_signature) {
// 	VarUInt hash_type_int = Utils::read_varuint(stream);
// 	uint32_t hash_length = 0;
// 	// FIXME: Endianess!
// 	sread(stream, &hash_length, sizeof(uint32_t));

// 	HashType hash_type = static_cast<HashType>(hash_type_int);
	
// 	throw new std::exception();
//     }
// }

}
