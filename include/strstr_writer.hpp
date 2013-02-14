/**********************************************************************
File name: strstr_writer.hpp
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
#ifndef _STRSTR_WRITER_H
#define _STRSTR_WRITER_H

#include <unordered_map>

#include "include/strstr_io.hpp"
#include "include/strstr_nodes.hpp"

namespace StructStream {

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
    void close();
    inline bool is_open() const { return _dest != nullptr; };
    void open(IOIntfHandle stream);
    void write(const NodeHandle subtree);
public:
    inline bool get_use_armor() const { return _armor_containers; };
    inline void set_use_armor(bool value) { _armor_containers = value; };
};

}

#endif
