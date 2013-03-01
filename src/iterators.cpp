/**********************************************************************
File name: iterators.cpp
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
#include "structstream/iterators.hpp"

#include <cassert>

namespace StructStream {

/* StructStream::NodeTreeIterator */

NodeTreeIterator::NodeTreeIterator():
    _cont(),
    _cont_iter(),
    _nested_iter()
{

}

NodeTreeIterator::NodeTreeIterator(ContainerHandle cont):
    _cont(cont),
    _cont_iter(_cont->children_begin()),
    _nested_iter()
{
    if (_cont_iter == _cont->children_end()) {
        _cont = ContainerHandle();
    }
}

NodeTreeIterator::~NodeTreeIterator()
{
    if (_nested_iter != nullptr) {
        delete _nested_iter;
    }
}

NodeTreeIterator& NodeTreeIterator::operator++()
{
    if (!_cont) {
        return *this;
    }

    if (_nested_iter) {
        ++(*_nested_iter);
        if (_nested_iter->valid()) {
            return *this;
        }
        delete _nested_iter;
        _nested_iter = nullptr;
    } else {
        ContainerHandle cont = std::dynamic_pointer_cast<Container>(*_cont_iter);
        if (cont) {
            _nested_iter = new NodeTreeIterator(cont);
            if (_nested_iter->valid()) {
                return *this;
            } else {
                delete _nested_iter;
                _nested_iter = nullptr;
            }
        }
    }

    skip();

    return *this;
}

void NodeTreeIterator::skip()
{
    if (_nested_iter) {
        // printf("nti 0x%lx: delegating skip to 0x%lx\n",
        //        (intptr_t)this,
        //        (intptr_t)_nested_iter);
        _nested_iter->skip();
        if (!_nested_iter->valid()) {
            // printf("nti 0x%lx: nested iter depleted during skip\n", (intptr_t)this);
            delete _nested_iter;
            _nested_iter = nullptr;
        }
        skip();
    } else {
        // printf("nti 0x%lx: local skip\n", (intptr_t)this);
        ++_cont_iter;
        if (_cont_iter == _cont->children_end()) {
            // printf("nti 0x%lx: eoc during skip\n", (intptr_t)this);
            kill();
        }
    }
}

void NodeTreeIterator::kill()
{
    _cont = ContainerHandle();
}

bool NodeTreeIterator::operator==(const NodeTreeIterator &other) const
{
    if ((other._cont == _cont) &&
        (other._cont_iter == _cont_iter))
    {
        if (_nested_iter && other._nested_iter) {
            return *_nested_iter == *other._nested_iter;
        } else if (!_nested_iter && !other._nested_iter) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

/* StructStream::FindMostShallow */

FindMostShallow::FindMostShallow():
    _iter(),
    _filter()
{

}

FindMostShallow::FindMostShallow(ContainerHandle cont, NodeFilter filter):
    _iter(cont),
    _filter(filter)
{
    if (_iter.valid()) {
        advance();
    }
}

FindMostShallow::~FindMostShallow()
{

}

void FindMostShallow::advance()
{
    // printf("fms 0x%lx: advance() \n", (intptr_t)this);
    while (_iter.valid()) {
        // printf("fms 0x%lx: advancing -- currently at node 0x%lx \n",
        //        (intptr_t)this,
        //        (intptr_t)(*_iter).get());
        NodeHandle item = *_iter;
        assert(item.get());
        if (_filter(*item.get())) {
            // printf("fms 0x%lx: match \n", (intptr_t)this);
            return;
        }
        ++_iter;
    }
}

FindMostShallow& FindMostShallow::operator++()
{
    ContainerHandle cont = std::dynamic_pointer_cast<Container>(*_iter);
    if (cont.get()) {
        // printf("fms 0x%lx: ++: is a container (skipping over)\n", (intptr_t)this);
        _iter.skip();
    } else {
        // printf("fms 0x%lx: ++: not a container\n", (intptr_t)this);
        ++_iter;
    }
    advance();
    return *this;
}

/* StructStream::FindByID */

FindByID::FindByID(const ID id):
    _id(id)
{

}

bool FindByID::operator() (const Node &node) const
{
    return node.id() == _id;
}

}
