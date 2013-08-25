/**********************************************************************
File name: serialize.cpp
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
#include "structstream/serialize.hpp"

namespace StructStream {

// test the compile-time helpers defined in the header

struct A
{
};

struct B: public A
{
};

static_assert(std::is_class<A>::value, "A is class");
static_assert(!std::is_same<A, void>::value, "A != void");
static_assert(!std::is_same<B, void>::value, "B != void");
static_assert(!std::is_same<void*, void>::value, "void* != void");
static_assert(!std::is_same<B, void>::value, "B != void");
static_assert(std::is_base_of<A, B>::value, "A is base of B");
static_assert(!std::is_base_of<B, A>::value, "B is not a base of A");
static_assert(std::is_same<typename most_subclassed<B, B>::type, B>::value, "B is most-advanced type of B and B");
static_assert(std::is_same<typename most_subclassed<A, B>::type, B>::value, "B is most-advanced type of A and B");
static_assert(std::is_same<typename common_struct_type<A, B>::type, B>::value, "B is common_struct_type of A and B");
static_assert(std::is_same<typename common_struct_type<A, void>::type, A>::value, "A is common_struct_type of A and void");
static_assert(std::is_same<typename common_struct_type<B, void>::type, B>::value, "B is common_struct_type of B and void");

/* StructStream::DeserializerSink */

DeserializerSink::DeserializerSink(deserializer_base *child):
    _child(child)
{

}

DeserializerSink::~DeserializerSink()
{
    _finalize_child();
}

void DeserializerSink::_finalize_child()
{
    if (!_child) {
        return;
    }

    std::unique_ptr<deserializer_base> guard(_child);
    _child = nullptr;

    guard->finalize();
}

bool DeserializerSink::end_container(const ContainerFooter *foot)
{
    return _child->end_container();
}

void DeserializerSink::end_of_stream()
{
    _finalize_child();
}

bool DeserializerSink::push_node(NodeHandle node)
{
    return _child->node(node);
}

bool DeserializerSink::start_container(
    ContainerHandle cont,
    const ContainerMeta *meta)
{
    return _child->start_container(cont);
}

}
