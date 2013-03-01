/**********************************************************************
File name: node_factory.hpp
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
#ifndef _STRUCTSTREAM_NODE_FACTORY_H
#define _STRUCTSTREAM_NODE_FACTORY_H

#include <memory>

#include "structstream/static.hpp"

namespace StructStream {

class Node;
typedef std::shared_ptr<Node> NodeHandle;
typedef std::weak_ptr<Node> NodeWeakHandle;

template <class NodeT>
struct NodeHandleFactory {
    typedef std::shared_ptr<NodeT> NodeTHandle;
    typedef std::weak_ptr<NodeT> NodeTWeakHandle;

    template <typename ... ArgTs>
    inline static NodeTHandle createv(ID id, ArgTs... args) {
        NodeT *node = new NodeT(id, args...);
        NodeTHandle handle = NodeTHandle(node);
        node->_self = NodeTWeakHandle(handle);
        return handle;
    };

    inline static NodeTHandle create(ID id) {
        return createv<>(id);
    };

    inline static NodeTHandle create_with_children(
        ID id,
        std::initializer_list<NodeHandle> children)
    {
        return createv(id, children);
    };

    inline static NodeTHandle copy(const NodeT &ref) {
        NodeT *node = new NodeT(ref);
        NodeTHandle handle = NodeTHandle(node);
        node->_self = NodeTWeakHandle(handle);
        return handle;
    };
private:
    NodeHandleFactory();
    NodeHandleFactory(const NodeHandleFactory &ref);
};


}

#endif
