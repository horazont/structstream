/**********************************************************************
File name: node_container.cpp
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
#include "structstream/node_container.hpp"

#include <cassert>

#include "structstream/errors.hpp"

namespace StructStream {

/* StructStream::Container */

Container::Container(ID id):
    Node::Node(id),
    _validated(false),
    _hash_function(HT_INVALID),
    _children(),
    _id_lut()
{

}

Container::Container(const Container &ref):
    Node::Node(ref),
    _validated(ref._validated),
    _hash_function(ref._hash_function),
    _children(),
    _id_lut()
{
    for (auto it = ref.children_cbegin(); it != ref.children_cend(); it++) {
        child_add((*it)->copy());
    }
}

Container::~Container()
{

}

void Container::check_valid_child(NodeHandle child) const
{
    if (child->parent().get() != nullptr) {
        throw ParentAlreadySet("Node cannot have multiple parents.");
    }
}

void Container::checkin_child(NodeHandle child)
{
    // _id_lut.emplace(
    //     std::piecewise_construct,
    //     std::forward_as_tuple(child->id()),
    //     std::forward_as_tuple(child)
    //     );
    _id_lut.insert(std::make_pair(child->id(), child));
}

void Container::checkout_child(NodeHandle child)
{
    std::pair<NodeByIDIterator, NodeByIDIterator> range = _id_lut.equal_range(child->id());
    for (NodeByIDIterator it = range.first;
         it != range.second;
         it++)
    {
        if ((*it).second == child) {
            _id_lut.erase(it);
            return;
        }
    }
}

void Container::child_add(NodeHandle child)
{
    check_valid_child(child);
    _children.push_back(child);
    child->set_parent(std::static_pointer_cast<Container>(_self.lock()));
    checkin_child(child);
}

intptr_t Container::child_count() const
{
    return _children.size();
}

void Container::child_erase(NodeVector::iterator &to_remove)
{
    NodeHandle child = *to_remove;
    if ((*to_remove)->parent().get() != this) {
        throw NotMyChild("Cannot erase a child which is not mine.");
    }
    _children.erase(to_remove);
    child->set_parent(nullptr);
    checkout_child(child);
}

NodeVector::iterator Container::child_find(NodeHandle child)
{
    NodeVector::iterator it = _children.begin();
    for (;
         it != _children.end();
         it++)
    {
        if (*it == child) {
            break;
        }
    }
    return it;
}

void Container::child_insert_before(NodeVector::iterator &ref, NodeHandle child)
{
    check_valid_child(child);
    _children.insert(ref, child);
    child->set_parent(std::static_pointer_cast<Container>(_self.lock()));
    checkin_child(child);
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

Container::NodeRangeByID Container::children_by_id(const ID id) const
{
    return _id_lut.equal_range(id);
}

NodeHandle Container::first_child_by_id(const ID id) const
{
    NodeRangeByID range = children_by_id(id);
    if (range.first == range.second) {
        return NodeHandle();
    }
    return (*range.first).second;
}

void Container::set_hashed(bool validated, HashType hash_function)
{
    assert(!validated || hash_function != HT_INVALID);

    _validated = validated;
    _hash_function = (validated ? hash_function : HT_INVALID);
}

NodeHandle Container::copy() const
{
    return NodeHandleFactory<Container>::copy(*this);
}

}
