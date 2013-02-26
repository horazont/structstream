/**********************************************************************
File name: streaming_base.cpp
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
#include "structstream/streaming_base.hpp"

#include <cassert>
#include <stdexcept>

#include "structstream/node_container.hpp"

namespace StructStream {

ContainerMeta::ContainerMeta():
    child_count(-1)
{

}

ContainerMeta::ContainerMeta(const Container &cont):
    child_count(cont.child_count())
{

}

ContainerMeta::ContainerMeta(const ContainerMeta &ref):
    child_count(ref.child_count)
{

}

ContainerMeta::~ContainerMeta()
{

}

ContainerMeta* ContainerMeta::copy() const
{
    return new ContainerMeta(*this);
}

/* StructStream::ContainerFooter */

ContainerFooter::ContainerFooter():
    validated(false),
    hash_function(HT_INVALID)
{

}

ContainerFooter::ContainerFooter(const ContainerFooter &ref):
    validated(ref.validated),
    hash_function(ref.hash_function)
{

}

ContainerFooter::~ContainerFooter()
{

}

ContainerFooter *ContainerFooter::copy() const
{
    return new ContainerFooter(*this);
}

/* StructStream::StreamSinkIntf */

void StreamSinkIntf::end_of_stream()
{

}

/* StructStream::SinkTree */

SinkTree::SinkTree():
    _nested(),
    _handling_container(false),
    _depth(0)
{

}

SinkTree::~SinkTree()
{

}

void SinkTree::nest(StreamSink &other)
{
    assert(_nested.get() == nullptr);
    if (!_handling_container) {
        throw std::logic_error("nest() can only be called inside of start_container() handler.");
    }

    _nested = other;
    _depth = 1;
}

void SinkTree::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    if (_nested.get() != nullptr) {
        _depth += 1;
        _nested->start_container(cont, meta);
    } else {
        _handling_container = true;
        _start_container(cont, meta);
        _handling_container = false;
    }
}

void SinkTree::push_node(NodeHandle node)
{
    if (_nested.get() != nullptr) {
        _nested->push_node(node);
    } else {
        _push_node(node);
    }
}

void SinkTree::end_container(const ContainerFooter *foot)
{
    if (_nested.get() != nullptr) {
        _depth -= 1;
        if (_depth == 0) {
            _nested = StreamSink();
            _end_container(foot);
        } else {
            _nested->end_container(foot);
        }
    } else {
        _end_container(foot);
    }
}

void SinkTree::end_of_stream()
{
    // this always goes to the top-level item.
    _end_of_stream();
}

/* StructStream::ThrowOnAll */

void ThrowOnAll::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    throw std::logic_error("This sink should not recieve start_container().");
}

void ThrowOnAll::push_node(NodeHandle node)
{
    throw std::logic_error("This sink should not recieve push_node().");
}

void ThrowOnAll::end_container(const ContainerFooter *foot)
{
    throw std::logic_error("This sink should not recieve end_container().");
}

void ThrowOnAll::end_of_stream()
{
    throw std::logic_error("This sink should not recieve end_of_stream().");
}

}
