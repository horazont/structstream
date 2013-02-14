/**********************************************************************
File name: node_base.hpp
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
#ifndef _STRUCTSTREAM_NODE_BASE_H
#define _STRUCTSTREAM_NODE_BASE_H

#include "structstream/io.hpp"
#include "structstream/node_factory.hpp"

namespace StructStream {

class Container;
typedef std::shared_ptr<Container> ContainerHandle;
typedef std::weak_ptr<Container> ContainerWeakHandle;

/**
 * Base class for all nodes which can written into a structstream
 * file. You may derive from this or DataRecord or Container depending
 * on your needs if you want to define a new record type.
 *
 * You must /never/ create nodes by hand, which is why the
 * constructors are protected. Use
 * NodeHandleFactory<NodeClass>::create(id) to create nodes.
 *
 * @param id structstream ID of the node
 */
class Node {
protected:
    Node(ID id);
    Node(const Node &ref);
public:
    virtual ~Node();
protected:
    NodeWeakHandle _self;
    const ID _id;
    ContainerWeakHandle _parent;
protected:
    void set_parent(ContainerHandle parent);
public:
    inline ID id() const {
        return  _id;
    };

    inline ContainerHandle parent() const {
        return _parent.lock();
    };

    /**
     * Detach the node from it's parent, do nothing if no parent is
     * assigned. This also removes the node from the parents child
     * list.
     */
    void detach_from_parent();

    /**
     * Create and return a deep copy of the node.
     */
    virtual NodeHandle copy() const = 0;

    /**
     * Return the record type of the node.
     */
    virtual RecordType record_type() const = 0;

    /**
     * If the node has contents, read them from the stream. This is
     * /not/ how containers are read. Containers do nothing in read()
     * and it's also not called by the Reader class.
     *
     * @param stream The stream to read from.
     */
    virtual void read(IOIntf *stream) = 0;

    /**
     * Write the nodes header (record type and id) and it's contents
     * to the given stream.
     *
     * This is not how containers are written. Containers do nothing
     * in write().
     *
     * @param stream The stream to write to.
     */
    virtual void write(IOIntf *stream) const = 0;

    /**
     * Helper function to write the record type and the id to a
     * stream.
     *
     * @param stream Stream to write the data to.
     */
    void write_header(IOIntf *stream) const;

    // If you want to make your node constructible using the
    // NodeHandleFactory, include this line and adapt it
    // appropriately.
    friend class NodeHandleFactory<Node>;
    friend class Container;
};

}

#endif
