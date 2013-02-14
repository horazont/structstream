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
#include "structstream/nodes.hpp"

#include <cassert>

#include "structstream/errors.hpp"

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

Node::~Node()
{

}

void Node::set_parent(ContainerHandle parent) {
    assert(_parent.lock().get() == nullptr);
    _parent = parent;
}

void Node::detach_from_parent() {
    Container *parent = _parent.lock().get();
    if (parent == nullptr) {
        return;
    }

    NodeVector::iterator me = parent->child_find(_self.lock());
    assert(me != parent->children_end());

    parent->child_erase(me);
}

void Node::write_header(IOIntf *stream) const
{
    Utils::write_record_type(stream, record_type());
    Utils::write_id(stream, _id);
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

void Container::check_valid_child(NodeHandle child) const
{
    if (child->parent().get() != nullptr) {
        throw ParentAlreadySet("Node cannot have multiple parents.");
    }
}

void Container::child_add(NodeHandle child)
{
    check_valid_child(child);
    _children.push_back(child);
    child->set_parent(std::static_pointer_cast<Container>(_self.lock()));
}

intptr_t Container::child_count() const
{
    return _children.size();
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

/* StructStream::DataRecord */

DataRecord::DataRecord(ID id):
    Node::Node(id)
{

}

DataRecord::DataRecord(const DataRecord &ref):
    Node::Node(ref)
{

}

DataRecord::~DataRecord()
{

}

/** StructStream::UTF8Record */

UTF8Record::~UTF8Record()
{

}

NodeHandle UTF8Record::copy() const
{
    return NodeHandleFactory<UTF8Record>::copy(*this);
}

void UTF8Record::read(IOIntf *stream)
{
    // \0 is implied!
    VarInt length = Utils::read_varint(stream) + 1;
    if (length < 0) {
        throw std::exception();
    }
    if (length != _len) {
        _len = length;
        _buf = realloc(_buf, size());
    }
    sread(stream, _buf, size()-1);
    ((char*)_buf)[length-1] = 0;
}

void UTF8Record::write(IOIntf *stream) const
{
    write_header(stream);
    Utils::write_varint(stream, _len-1);
    swrite(stream, _buf, _len-1);
}

/** StructStream::BlobRecord */

BlobRecord::~BlobRecord()
{

}

NodeHandle BlobRecord::copy() const
{
    return NodeHandleFactory<BlobRecord>::copy(*this);
}

void BlobRecord::write(IOIntf *stream) const
{
    write_header(stream);
    Utils::write_varint(stream, _len);
    swrite(stream, _buf, _len);
}

}
