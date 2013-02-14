/**********************************************************************
File name: node_factory.hpp
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
    inline static NodeHandle create(ID id) {
        NodeT *node = new NodeT(id);
        NodeHandle handle = NodeHandle(node);
        node->_self = NodeWeakHandle(handle);
        return handle;
    };

    inline static NodeHandle copy(const NodeT &ref) {
        NodeT *node = new NodeT(ref);
        NodeHandle handle = NodeHandle(node);
        node->_self = NodeWeakHandle(handle);
        return handle;
    };
private:
    NodeHandleFactory();
    NodeHandleFactory(const NodeHandleFactory &ref);
};


}

#endif
