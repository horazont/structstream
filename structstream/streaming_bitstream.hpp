/**********************************************************************
File name: streaming_bitstream.hpp
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
#ifndef _STRUCTSTREAM_STREAMING_BITSTREAM_H
#define _STRUCTSTREAM_STREAMING_BITSTREAM_H

#include <forward_list>

#include "structstream/streaming_base.hpp"
#include "structstream/io.hpp"
#include "structstream/registry.hpp"

namespace StructStream {

class FromBitstream {
protected:
    struct ParentInfo {
        ParentInfo():
            cont(),
            meta(new ContainerMeta()),
            footer(new ContainerFooter()) {};
        virtual ~ParentInfo() {
            if (meta) {
                delete meta;
            }
            if (footer) {
                delete footer;
            }
        };

        ContainerHandle cont;
        ContainerMeta *meta;
        ContainerFooter *footer;

        int32_t read_child_count;
        bool armored;
    };
public:
    FromBitstream(IOIntfHandle source,
             const RegistryHandle nodetypes,
             StreamSink sink);
    virtual ~FromBitstream();
private:
    IOIntfHandle _source_h;
    IOIntf *_source;

    const RegistryHandle _node_factory_h;
    const Registry *_node_factory;

    StreamSink _sink_h;
    StreamSinkIntf *_sink;

    std::forward_list<ParentInfo*> _parent_stack;
    ParentInfo *_curr_parent;
protected:
    void check_end_of_container();
    void push_root();
protected:
    virtual ParentInfo *new_parent_info() const;
    void start_of_container(ContainerHandle cont_h);
    virtual void proc_container_flags(VarUInt &flags_int,
                                      ParentInfo *info);
    virtual void end_of_container_header(ParentInfo *info);
    virtual void end_of_container_body(ContainerFooter *foot);
    void end_of_container();
protected:
    NodeHandle read_next();
public:
    void read_all();
};

class ToBitstream: public StreamSinkIntf {
protected:
    struct ParentInfo {
        ContainerHandle cont;
        int32_t child_count;
        bool armored;
        HashType hash_function;
    };
public:
    ToBitstream(IOIntfHandle dest);
    virtual ~ToBitstream();
private:
    IOIntfHandle _dest_h;
    IOIntf *_dest;

    std::forward_list<ParentInfo*> _parent_stack;
    ParentInfo *_curr_parent;

    bool _default_armor;
protected:
    void require_open() const;
protected:
    virtual ParentInfo *new_parent_info() const;
    virtual VarUInt get_container_flags(ParentInfo *info);
    virtual ParentInfo *setup_container(ContainerHandle cont, const ContainerMeta *meta);
    virtual void write_container_header(VarUInt flags, ParentInfo *info);
    virtual void write_container_footer(ParentInfo *info);
    virtual void write_footer();
public:
    virtual void start_container(ContainerHandle cont, const ContainerMeta *meta);
    virtual void push_node(NodeHandle node);
    virtual void end_container(const ContainerFooter *foot);
    virtual void end_of_stream();
public:
    void close();
public:
    inline bool get_armor_default() const {
        return _default_armor;
    };
    void set_armor_default(bool armor) {
        _default_armor = armor;
    };
};

}

#endif
