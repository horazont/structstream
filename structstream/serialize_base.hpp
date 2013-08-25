/**********************************************************************
File name: serialize_base.hpp
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
#ifndef _STRUCTSTREAM_SERIALIZE_BASE_H
#define _STRUCTSTREAM_SERIALIZE_BASE_H

#include <memory>

namespace StructStream {

class Node;
class Container;
typedef std::shared_ptr<Node> NodeHandle;
typedef std::shared_ptr<Container> ContainerHandle;

struct deserializer_base
{
    virtual ~deserializer_base();
    virtual bool end_container();
    virtual void finalize();
    virtual bool node(const NodeHandle &node);
    virtual bool start_container(const ContainerHandle &cont);
};

struct deserializer_nesting: public deserializer_base
{
    deserializer_nesting();
    virtual ~deserializer_nesting();

private:
    size_t _depth;
    deserializer_base *_nested;

    void unnest();

protected:
    void nest(deserializer_base *obj);
    virtual bool _end_container();
    virtual bool _node(const NodeHandle &node);
    virtual bool _start_container(const ContainerHandle &cont);

public:
    bool end_container() override final;
    void finalize() override;
    bool node(const NodeHandle &node) override final;
    bool start_container(const ContainerHandle &cont) override final;
};

struct deserializer_null: public deserializer_base
{
public:
    bool end_container() override;
    bool node(const NodeHandle &node) override;
    bool start_container(const ContainerHandle &cont) override;

};

}

#endif
