/**********************************************************************
File name: streaming_sinks.cpp
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
#include "structstream/streaming_sinks.hpp"

#include <stdexcept>

#include <cassert>

#include "structstream/node_container.hpp"

namespace StructStream {

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

void SinkTree::nest(StreamSink other)
{
    assert(_nested.get() == nullptr);
    if (!_handling_container) {
        throw std::logic_error("nest() can only be called inside of start_container() handler.");
    }

    _nested = other;
    // printf("sinktree: nested %lx\n", (uint64_t)_nested.get());
    _depth = 1;
}

bool SinkTree::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    if (_nested.get() != nullptr) {
        _depth += 1;
        // printf("sinktree: forwarding to nested %lx\n", (uint64_t)_nested.get());
        return _nested->start_container(cont, meta);
    } else {
        _handling_container = true;
        // printf("sinktree: handling start_container\n");
        bool result = _start_container(cont, meta);
        _handling_container = false;
        return result;
    }
}

bool SinkTree::push_node(NodeHandle node)
{
    if (_nested.get() != nullptr) {
        // printf("sinktree: forwarding to nested %lx\n", (uint64_t)_nested.get());
        return _nested->push_node(node);
    } else {
        // printf("sinktree: handling push_node\n");
        return _push_node(node);
    }
}

bool SinkTree::end_container(const ContainerFooter *foot)
{
    if (_nested.get() != nullptr) {
        _depth -= 1;
        if (_depth == 0) {
            // printf("sinktree: ended nested %lx\n", (uint64_t)_nested.get());
            _nested = StreamSink();
            return _end_container(foot);
        } else {
            // printf("sinktree: forwarding to nested %lx\n", (uint64_t)_nested.get());
            return _nested->end_container(foot);
        }
    } else {
        // printf("sinktree: handling end_of_container\n");
        return _end_container(foot);
    }
}

void SinkTree::end_of_stream()
{
    // this always goes to the top-level item.
    _end_of_stream();
}

/* StructStream::ThrowOnAll */

bool ThrowOnAll::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    throw std::logic_error("This sink should not recieve start_container().");
}

bool ThrowOnAll::push_node(NodeHandle node)
{
    throw std::logic_error("This sink should not recieve push_node().");
}

bool ThrowOnAll::end_container(const ContainerFooter *foot)
{
    throw std::logic_error("This sink should not recieve end_container().");
}

void ThrowOnAll::end_of_stream()
{
    throw std::logic_error("This sink should not recieve end_of_stream().");
}

/* StructStream::NullSink */

bool NullSink::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    return true;
}

bool NullSink::push_node(NodeHandle node)
{
    return true;
}

bool NullSink::end_container(const ContainerFooter *foot)
{
    return true;
}

void NullSink::end_of_stream()
{

}

/* StructStream::SinkChain */

SinkChain::SinkChain(const std::initializer_list<StreamSink> &sinks):
    _next_sinks(sinks),
    _current_sink()
{
    assert(select_next_sink());
}

bool SinkChain::select_next_sink()
{
    if (_next_sinks.empty()) {
        return false;
    }
    std::swap(_current_sink, _next_sinks.front());
    _next_sinks.pop_front();
    return true;
}

bool SinkChain::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    if (!_current_sink->start_container(cont, meta)) {
        return select_next_sink();
    }
    return true;
}

bool SinkChain::push_node(NodeHandle node)
{
    if (!_current_sink->push_node(node)) {
        return select_next_sink();
    }
    return true;
}

bool SinkChain::end_container(const ContainerFooter *foot)
{
    if (!_current_sink->end_container(foot)) {
        return select_next_sink();
    }
    return true;
}

void SinkChain::end_of_stream()
{
    _current_sink->end_of_stream();
}

/* StructStream::SinkDebug */

SinkDebug::SinkDebug(std::ostream &dest):
    _dest(dest),
    _indent("")
{

}

void SinkDebug::node_info(const NodeHandle &node)
{
    _dest << "(rt=0x" << std::hex << node->record_type()
          << "; id=0x" << std::hex << node->id()
          << ")";
}

bool SinkDebug::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    _dest << _indent << "|- cont ";
    node_info(std::dynamic_pointer_cast<Node>(cont));
    _dest << std::endl;
    _indent += "|   ";
    return true;
}

bool SinkDebug::push_node(NodeHandle node)
{
    _dest << _indent << "|- node ";
    node_info(node);
    _dest << std::endl;
    return true;
}

bool SinkDebug::end_container(const ContainerFooter *foot)
{
    _indent = _indent.substr(0, _indent.size()-4);
    return true;
}

void SinkDebug::end_of_stream()
{
    _dest << "end of stream" << std::endl;
}


}
