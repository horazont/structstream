/**********************************************************************
File name: writer.hpp
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
#ifndef _STRUCTSTREAM_WRITER_H
#define _STRUCTSTREAM_WRITER_H

#include <unordered_map>

#include "structstream/io.hpp"
#include "structstream/node_base.hpp"
#include "structstream/node_container.hpp"

namespace StructStream {

/**
 * Implement writing of nodes to a stream.
 */
class Writer
{
public:
    struct ContainerInfo
    {
        const Container *node;
        HashType hashfunc;
        bool armored;
    };
public:
    Writer();
    virtual ~Writer();
private:
    IOIntf *_dest;
    IOIntfHandle _dest_h;

    std::unordered_map<ID, HashType> _hashed_containers;
    HashType _default_hash;
    bool _armor_containers;
protected:
    HashType get_container_hash(const Container* node) const;
    bool get_container_armored(const Container* node) const;
    void require_open() const;
protected:
    virtual ContainerInfo *new_container_info() const;
    virtual void write_header();
    virtual void write_container(const Container *node);
    virtual void setup_container_header(ContainerInfo *info);
    virtual void write_container_header(ContainerInfo *info);
    virtual void write_container_footer(ContainerInfo *info);
    virtual void write_footer();
public:
    /**
     * Close a currently open stream.
     *
     * You must call this (or delete the Writer instance) to
     * completely finish writing.
     */
    void close();

    /**
     * Return whether a stream is currently opened.
     */
    inline bool is_open() const { return _dest != nullptr; };

    /**
     * Open a stream.
     *
     * It is an error to call this while is_open returns true.
     */
    void open(IOIntfHandle stream);

    /**
     * Write a node to the stream.
     *
     * @param subtree A node to write. Does not need to be a
     * container, but containers are supported.
     */
    void write(const NodeHandle subtree);
public:
    /**
     * Whether to use armoring for containers.
     *
     * See set_use_armor for details.
     */
    inline bool get_use_armor() const { return _armor_containers; };

    /**
     * Control whether armoring is used for containers.
     *
     * If armoring is enabled, containers are written with the
     * CF_ARMORED flag, regardless of the amount of children they have
     * or whether this amount is known.
     *
     * This means that an END_OF_CHILDREN marker is written after the
     * last child of each container, which gives the reader an
     * additional check that the correct amount of children was read.
     *
     * It is safe to change this value in the middle of a writing
     * operation. It's use inside of the class is atomic.
     *
     * @param value true to enable armoring, false to disable.
     */
    inline void set_use_armor(bool value) { _armor_containers = value; };
};

}

#endif
