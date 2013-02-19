/**********************************************************************
File name: deserializer.hpp
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
#ifndef _STRUCTSTREAM_DESERIALIZER_H
#define _STRUCTSTREAM_DESERIALIZER_H

#include "structstream/static.hpp"
#include "structstream/node_container.hpp"

#include <cassert>

namespace StructStream {

typedef std::function< void (Node *rec) > SetFunc;

template <ID _record_id, class _record_t, typename _dest_t, intptr_t _dest_offs=0, bool _required=false>
struct deserialize_primitive {
    static_assert(std::is_standard_layout<_dest_t>::value, "primitive serialization target must be standard layout type.");

    static constexpr bool required = _required;
    static constexpr ID record_id = _record_id;
    static constexpr intptr_t dest_offs = _dest_offs;
    typedef _dest_t dest_t;
    typedef _record_t record_t;

    static inline void deserialize(const record_t *rec, void *dest)
        {
            *((dest_t*)((intptr_t)(dest) + dest_offs)) = rec->get();
        }
};

template <ID _record_id, class _record_t, typename _dest_t, void (_dest_t::*setfunc)(const _record_t*), bool _required=false>
struct deserialize_custom {
    static constexpr ID record_id = _record_id;
    static constexpr bool required = _required;
    typedef _record_t record_t;

    typedef _dest_t dest_t;

    static inline void deserialize(const record_t *rec, dest_t *dest)
        {
            (dest->*setfunc)(rec);
        }
};

template <ID _record_id, class _record_t, typename _dest_t, typename buf_t, void (_dest_t::*setfunc)(const buf_t*, const intptr_t), bool _required=false>
struct deserialize_buffer
{
    static constexpr ID record_id = _record_id;
    static constexpr bool required = _required;
    typedef _record_t record_t;

    typedef _dest_t dest_t;

    static inline void deserialize(const record_t *rec, dest_t *dest)
    {
        (dest->*setfunc)(rec->dataptr(), rec->datalen());
    }
};

template <ID _record_id, class _record_t, typename _dest_t, void (_dest_t::*setfunc)(const std::string&), bool _required=false>
struct deserialize_string
{
    static constexpr ID record_id = _record_id;
    static constexpr bool required = _required;
    typedef _record_t record_t;

    typedef _dest_t dest_t;

    static inline void deserialize(const record_t *rec, dest_t *dest)
    {
        (dest->*setfunc)(rec->datastr());
    }
};

template <ID _record_id, typename _dest_t, typename... field_ts>
struct deserialize_block
{
};


template <ID _record_id, typename _dest_t, typename field_t, typename... field_ts>
struct deserialize_block<_record_id, _dest_t, field_t, field_ts...>
{
    typedef _dest_t dest_t;
    typedef typename field_t::record_t child_record_t;
    typedef Container record_t;
    static constexpr ID record_id = _record_id;

    static inline void deserialize(Container *node, dest_t *dest)
        {
            const Node *child = node->first_child_by_id(field_t::record_id).get();
            if (child == nullptr) {
                if (field_t::required) {
                    throw std::exception();
                } else {
                    goto next;
                }
            }

            {
                const child_record_t *rec = dynamic_cast<const child_record_t*>(child);
                if (rec == nullptr) {
                    if (field_t::required) {
                        throw std::exception();
                    } else {
                        goto next;
                    }
                }

                field_t::deserialize(rec, dest);
            }

        next:
            deserialize_block<_record_id, dest_t, field_ts...>::deserialize(node, dest);
        }
};

template <ID _record_id, typename _dest_t>
struct deserialize_block<_record_id, _dest_t>
{
    typedef _dest_t dest_t;
    typedef Container record_t;
    static constexpr ID record_id = _record_id;

    static inline void deserialize(const Container *node, _dest_t *dest)
        {

        };
};

template <typename _serializer_t, typename iterator_t, typename... args_ts>
struct deserialize_array
{
    typedef _serializer_t serializer_t;
    typedef typename serializer_t::dest_t dest_t;
    static constexpr ID record_id = serializer_t::record_id;
    typedef typename serializer_t::record_t record_t;

    static inline void deserialize(const Container *node, iterator_t output_iterator, args_ts... args)
        {
            Container::NodeRangeByID range = node->children_by_id(serializer_t::record_id);
            dest_t *new_one = nullptr;
            try {
                for (auto it = range.first;
                     it != range.second;
                     it++)
                {
                    const NodeHandle curr_node = (*it).second;
                    const record_t *rec = dynamic_cast<const record_t*>(curr_node.get());
                    if (rec == nullptr) {
                        continue;
                    }

                    new_one = new dest_t(args...);
                    serializer_t::deserialize(rec, new_one);
                    *output_iterator++ = new_one;
                }
            } catch (...) {
                if (new_one) {
                    delete new_one;
                }
                throw;
            }
        };
};

}

#endif
