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
#ifndef _STRUCTSTREAM_STREAMING_H
#define _STRUCTSTREAM_STREAMING_H

#include <memory>

#include "structstream/static.hpp"
#include "structstream/node_base.hpp"

namespace StructStream {

/**
 * It is intended that all attributes are mutable.
 */
struct ContainerMeta {
public:
    ContainerMeta();
    ContainerMeta(const ContainerMeta &ref);
    virtual ~ContainerMeta() {};
public:
    ID id;
    RecordType record_type;
    int32_t child_count;
};

/**
 * The minimum interface to be implemented by a stream sink.
 */
class StreamSinkIntf {
public:
    /**
     * Start a new container in the current container. Only the
     * metadata of the container is required; it not neccessary or
     * supported to feed all children here at once.
     *
     * Some endpoints may support subclasses of ``container_meta``
     * which offer more information, but all sinks *MUST* be able to
     * work with only this set of data and all sources *MUST* at least
     * emit this set of data.
     *
     * The *meta* object is owned by the stream which sent it and will
     * be freed by it after the call. If an implementation needs the
     * data, it is supposed to create a copy of the object.
     */
    virtual void start_container(ContainerMeta *meta) = 0;

    /**
     * Push a node to the current container. This requires the full node,
     * as nodes do not share a very common format, except for their record
     * type and the ID.
     */
    virtual void push_node(NodeHandle node) = 0;

    /**
     * End the current innermost container and return to the upper
     * context.
     */
    virtual void end_container() = 0;
};

typedef std::shared_ptr<StreamSinkIntf> StreamSink;
typedef std::weak_ptr<StreamSinkIntf> StreamSinkW;

/**
 * Push a complete basic container to a stream sink. This does not
 * support any fancy ContainerMeta subclass and is just a convenience
 * method.
 */
void push_container(ContainerHandle container);

}

#endif
