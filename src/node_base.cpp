/**********************************************************************
File name: node_base.cpp
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
#include "structstream/node_base.hpp"

#include <cassert>

#include "structstream/node_container.hpp"
#include "structstream/utils.hpp"

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

}
