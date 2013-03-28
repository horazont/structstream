/**********************************************************************
File name: streaming_tree.cpp
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
#include "structstream/streaming_tree.hpp"

#include <cassert>
#include <cstdio>

#include "structstream/node_container.hpp"

namespace StructStream {

/* StructStream::ToTree */

ToTree::ToTree():
    _stack(),
    _root(std::static_pointer_cast<Container>(NodeHandleFactory<Container>::create(TreeRootID))),
    _curr_parent()
{
    init_root();
}

ToTree::ToTree(ContainerHandle root):
    _stack(),
    _root(root),
    _curr_parent()
{
    init_root();
}

ToTree::~ToTree()
{
    for (auto it: _stack) {
        delete it;
    }
    _stack.clear();
}

void ToTree::init_root()
{
    push_parent(
        new ParentInfo(
            new ContainerMeta(*(_root.get())),
            _root
            )
        );
}

void ToTree::push_parent(ParentInfo *info)
{
    _stack.push_front(info);
    _curr_parent = _stack.front();
}

void ToTree::pop_parent()
{
    _stack.pop_front();
    delete _curr_parent;
    _curr_parent = _stack.front();
}

bool ToTree::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    assert(_curr_parent != nullptr);

    // printf("tree: starting container with id 0x%lx\n", cont->id());

    _curr_parent->parent->child_add(cont);
    push_parent(new ParentInfo(meta->copy(), cont));
    return true;
}

bool ToTree::push_node(NodeHandle node)
{
    assert(_curr_parent != nullptr);

    // printf("tree: new child with id 0x%lx\n", node->id());

    _curr_parent->parent->child_add(node);
    return true;
}

bool ToTree::end_container(const ContainerFooter *foot)
{
    assert(_curr_parent != nullptr);

    // printf("tree: finish container with id 0x%lx\n", _curr_parent->parent_h->id());

    pop_parent();
    return true;
}

/* StructStream::FromTree */

bool subtree_to_sink(StreamSink sink, NodeHandle subtree)
{
    Container *cont = dynamic_cast<Container*>(subtree.get());
    if (cont != nullptr) {
        ContainerMeta meta;
        meta.child_count = cont->child_count();
        if (!sink->start_container(
                std::static_pointer_cast<Container>(subtree),
                &meta
                ))
        {
            return false;
        }
        for (auto it = cont->children_begin();
             it != cont->children_end();
             it++)
        {
            if (!subtree_to_sink(sink, *it)) {
                return false;
            }
        }
        ContainerFooter foot;
        foot.validated = false;
        foot.hash_function = HT_NONE;
        return sink->end_container(&foot);
    } else {
        return sink->push_node(subtree);
    }
}

void FromTree(StreamSink sink, ContainerHandle root, bool send_end_of_stream)
{
    for (auto it = root->children_begin();
         it != root->children_end();
         it++)
    {
        if (!subtree_to_sink(sink, *it)) {
            break;
        };
    }
    if (send_end_of_stream)
        sink->end_of_stream();
}

void FromTree(StreamSink sink, std::initializer_list<NodeHandle> children, bool send_end_of_stream)
{
    for (auto child: children) {
        if (!subtree_to_sink(sink, child)) {
            break;
        };
    }
    if (send_end_of_stream)
        sink->end_of_stream();
}

}
