/**********************************************************************
File name: streaming_sinks.hpp
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
#ifndef _STRUCTSTREAM_STREAMING_SINKS_H
#define _STRUCTSTREAM_STREAMING_SINKS_H

#include <forward_list>

#include "structstream/streaming_base.hpp"

namespace StructStream {

class SinkTree: public StreamSinkIntf {
public:
    SinkTree();
    virtual ~SinkTree();
private:
    StreamSink _nested;
    bool _handling_container;
    intptr_t _depth;
protected:
    void nest(StreamSink other);
protected:
    virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta) = 0;
    virtual bool _push_node(NodeHandle node) = 0;
    virtual bool _end_container(const ContainerFooter *foot) = 0;
    virtual void _end_of_stream() = 0;
public:
    bool start_container(ContainerHandle cont, const ContainerMeta *meta) final override;
    bool push_node(NodeHandle node) final override;
    bool end_container(const ContainerFooter *foot) final override;
    void end_of_stream() final override;
};

class ThrowOnAll: public StreamSinkIntf {
public:
    ThrowOnAll() = default;
    ThrowOnAll(const ThrowOnAll &ref) = default;
    virtual ~ThrowOnAll() {};
    ThrowOnAll &operator= (const ThrowOnAll &ref) = default;
public:
    bool start_container(ContainerHandle cont, const ContainerMeta *meta) override;
    bool push_node(NodeHandle node) override;
    bool end_container(const ContainerFooter *foot) override;
    void end_of_stream() override;
};

class NullSink: public StreamSinkIntf {
public:
    NullSink() = default;
    NullSink(const NullSink &ref) = default;
    virtual ~NullSink() {};
    NullSink &operator= (const NullSink &ref) = default;
public:
    bool start_container(ContainerHandle cont, const ContainerMeta *meta) override;
    bool push_node(NodeHandle node) override;
    bool end_container(const ContainerFooter *foot) override;
    void end_of_stream() override;
};

class SinkChain: public StreamSinkIntf {
public:
    SinkChain(const std::initializer_list<StreamSink> &sinks);
    SinkChain(const SinkChain &ref) = default;
    SinkChain &operator= (const SinkChain &ref) = default;

private:
    std::forward_list<StreamSink> _next_sinks;
    StreamSink _current_sink;

protected:
    bool select_next_sink();

public:
    bool start_container(ContainerHandle cont, const ContainerMeta *meta) override;
    bool push_node(NodeHandle node) override;
    bool end_container(const ContainerFooter *foot) override;
    void end_of_stream() override;

};

}

#endif
