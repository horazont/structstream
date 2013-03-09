/**********************************************************************
File name: streaming_tree.hpp
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
#ifndef _STRUCTSTREAM_STREAMING_TREE_H
#define _STRUCTSTREAM_STREAMING_TREE_H

#include <forward_list>

#include "structstream/streaming_base.hpp"

namespace StructStream {

class ToTree: public StreamSinkIntf {
protected:
    struct ParentInfo {
    public:
        ParentInfo(ContainerMeta *meta, ContainerHandle parent):
            meta(meta),
            parent_h(parent),
            parent(parent.get()) {};
        virtual ~ParentInfo() {
            delete meta;
        };
    public:
        ContainerMeta *meta;
        ContainerHandle parent_h;
        Container *parent;
    };

public:
    ToTree();
    ToTree(ContainerHandle root);
    virtual ~ToTree();
private:
    std::forward_list<ParentInfo*> _stack;
    ContainerHandle _root;
    ParentInfo *_curr_parent;
protected:
    void init_root();
    void push_parent(ParentInfo *info);
    void pop_parent();
public:
    virtual bool start_container(ContainerHandle cont, const ContainerMeta *meta);
    virtual bool push_node(NodeHandle node);
    virtual bool end_container(const ContainerFooter *foot);
public:
    inline ContainerHandle root() { return _root; };
};

/* class FromTree {
public:
    FromTree(StreamSink sink, ContainerHandle root);
    FromTree(StreamSink sink, NodeHandle first_child);
    virtual ~FromTree();
private:
    StreamSink _sink_h;
    StreamSinkIntf *_sink;

    ContainerHandle _root;
public:
    void send_all();
    };*/

void FromTree(StreamSink sink, ContainerHandle root);
void FromTree(StreamSink sink, std::initializer_list<NodeHandle> children);

}

#endif
