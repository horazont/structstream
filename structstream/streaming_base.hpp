/**********************************************************************
File name: streaming_base.hpp
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
#ifndef _STRUCTSTREAM_STREAMING_BASE_H
#define _STRUCTSTREAM_STREAMING_BASE_H

#include <memory>

#include "structstream/static.hpp"
#include "structstream/node_base.hpp"

namespace StructStream {

struct ContainerMeta {
public:
    ContainerMeta();
    ContainerMeta(const Container &cont);
    ContainerMeta(const ContainerMeta &ref);
    virtual ~ContainerMeta();
public:
    int32_t child_count;
public:
    virtual ContainerMeta *copy() const;
};

struct ContainerFooter {
public:
    ContainerFooter();
    ContainerFooter(const ContainerFooter &ref);
    virtual ~ContainerFooter();
public:
    bool validated;
    HashType hash_function;
public:
    virtual ContainerFooter *copy() const;
};

/**
 * The minimum interface to be implemented by a stream sink.
 */
class StreamSinkIntf {
public:
    StreamSinkIntf() = default;
    StreamSinkIntf(const StreamSinkIntf &ref) = default;
    StreamSinkIntf& operator= (const StreamSinkIntf &ref) = default;
public:
    /**
     * Start a new container in the current container. Only the
     * metadata of the container is required; it not neccessary or
     * supported to feed all children here at once.
     *
     * Some endpoints may support subclasses of ``ContainerMeta``
     * which offer more information, but all sinks *MUST* be able to
     * work with only this set of data and all sources *MUST* at least
     * emit this set of data.
     *
     * The *meta* object is owned by the emitter of the stream event
     * and as such will be deleted after the call. If an
     * implementation needs a copy, it shall create one.
     */
    virtual bool start_container(ContainerHandle cont, const ContainerMeta *meta) = 0;

    /**
     * Push a node to the current container. This requires the full node,
     * as nodes do not share a very common format, except for their record
     * type and the ID.
     *
     * An implemantation MAY assume that the *node* is a private copy
     * to it. Although users may use the streaming API differently,
     * they have to care about the possible side effects.
     */
    virtual bool push_node(NodeHandle node) = 0;

    /**
     * End the current innermost container and return to the upper
     * context.
     */
    virtual bool end_container(const ContainerFooter *foot) = 0;

    /**
     * Mark the end of the stream. Not all sinks may need this.
     */
    virtual void end_of_stream();
};

typedef std::shared_ptr<StreamSinkIntf> StreamSink;
typedef std::weak_ptr<StreamSinkIntf> StreamSinkW;

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
    virtual bool start_container(ContainerHandle cont, const ContainerMeta *meta) final;
    virtual bool push_node(NodeHandle node) final;
    virtual bool end_container(const ContainerFooter *foot) final;
    virtual void end_of_stream() final;
};

class ThrowOnAll: public StreamSinkIntf {
public:
    ThrowOnAll() = default;
    ThrowOnAll(const ThrowOnAll &ref) = default;
    virtual ~ThrowOnAll() {};
    ThrowOnAll &operator= (const ThrowOnAll &ref) = default;
public:
    virtual bool start_container(ContainerHandle cont, const ContainerMeta *meta);
    virtual bool push_node(NodeHandle node);
    virtual bool end_container(const ContainerFooter *foot);
    virtual void end_of_stream();
};

class NullSink: public StreamSinkIntf {
public:
    NullSink() = default;
    NullSink(const NullSink &ref) = default;
    virtual ~NullSink() {};
    NullSink &operator= (const NullSink &ref) = default;
public:
    virtual bool start_container(ContainerHandle cont, const ContainerMeta *meta);
    virtual bool push_node(NodeHandle node);
    virtual bool end_container(const ContainerFooter *foot);
    virtual void end_of_stream();
};

/**
 * Push a complete basic container to a stream sink. This does not
 * support any fancy ContainerMeta subclass and is just a convenience
 * method.
 */
void push_container(ContainerHandle container);

}

#endif
