/**********************************************************************
File name: serializer.hpp
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
#ifndef _STRUCTSTREAM_SERIALIZER_H
#define _STRUCTSTREAM_SERIALIZER_H

#include "structstream/static.hpp"
#include "structstream/node_container.hpp"

namespace StructStream {

template <ID _record_id, class _record_t, typename _src_t, intptr_t src_offs=0>
struct serialize_primitive {
    static_assert(std::is_standard_layout<_src_t>::value, "primitive serialization target must be standard layout type.");

    static constexpr ID record_id = _record_id;
    typedef _src_t src_t;
    typedef _record_t record_t;

    static inline NodeHandle serialize(const void *src)
    {
        NodeHandle node = NodeHandleFactory<record_t>::create(record_id);

        static_cast<record_t*>(node.get())->set(
            *reinterpret_cast<src_t*>(reinterpret_cast<intptr_t>(src) + src_offs)
            );
        return node;
    }
};

template <ID _record_id, class _record_t, typename _src_t>
struct serialize_primitive_by_value {

    static constexpr ID record_id = _record_id;
    typedef _src_t src_t;
    typedef _record_t record_t;

    static inline NodeHandle serialize(const src_t &src)
    {
        NodeHandle node = NodeHandleFactory<record_t>::create(record_id);

        static_cast<record_t*>(node.get())->set(src);
        return node;
    }
};

template <ID _record_id, class _record_t, typename _src_t, const std::string& (_src_t::*getfunc)() const>
struct serialize_string {
    static constexpr ID record_id = _record_id;
    typedef _src_t src_t;
    typedef _record_t record_t;

    static inline NodeHandle serialize(const src_t *src)
    {
        NodeHandle node = NodeHandleFactory<record_t>::create(record_id);
        static_cast<record_t*>(node.get())->set((src->*getfunc)());
        return node;
    }
};

template <ID _record_id, class _record_t, typename _src_t, typename _value_t, _value_t (_src_t::*getfunc)() const>
struct serialize_custom {
    static constexpr ID record_id = _record_id;
    typedef _src_t src_t;
    typedef _record_t record_t;

    static inline NodeHandle serialize(const src_t *src)
    {
        NodeHandle node = NodeHandleFactory<record_t>::create(record_id);
        static_cast<record_t*>(node.get())->set((src->*getfunc)());
        return node;
    }
};

template <typename _src_t, typename... field_ts>
struct serialize_block_impl
{
};

template <typename _src_t, typename field_t, typename... field_ts>
struct serialize_block_impl<_src_t, field_t, field_ts...>
{
    typedef _src_t src_t;

    inline static void serialize_into(Container *parent, const src_t *src)
    {
        parent->child_add(field_t::serialize(src));
        serialize_block_impl<_src_t, field_ts...>::serialize_into(parent, src);
    }
};

template <typename _src_t>
struct serialize_block_impl<_src_t>
{
    typedef _src_t src_t;

    inline static void serialize_into(Container *parent, const src_t *src)
    {

    };
};

template <ID _record_id, typename _src_t, typename... field_ts>
struct serialize_block
{
    typedef Container record_t;
    typedef _src_t src_t;
    static constexpr ID record_id = _record_id;
private:
    typedef serialize_block_impl<_src_t, field_ts...> serializer_impl;
public:
    inline static NodeHandle serialize(const src_t *src)
    {
        NodeHandle parent_node = NodeHandleFactory<Container>::create(record_id);
        Container *parent = static_cast<Container*>(parent_node.get());

        serializer_impl::serialize_into(parent, src);

        return parent_node;
    }
};

template <typename serializer_t>
struct serialize_iterator
{
    typedef Container record_t;

    template <typename iterator_t>
    inline static void serialize_into(Container *parent,
                                      iterator_t iterator_begin,
                                      iterator_t iterator_end)
    {
        for (auto it = iterator_begin;
             it != iterator_end;
             it++)
        {
            parent->child_add(serializer_t::serialize(*it));
        }
    }
};

}

#endif
