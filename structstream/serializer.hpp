#ifndef _STRUCTSTREAM_SERIALIZER_H
#define _STRUCTSTREAM_SERIALIZER_H

#include "structstream/static.hpp"
#include "structstream/node_container.hpp"

#include <cassert>

namespace StructStream {

typedef std::function< void (Node *rec) > SetFunc;

template <ID _record_id, class _record_t, typename _dest_t, intptr_t _dest_offs, bool _required=false>
struct deserialize_primitive {
    static_assert(std::is_standard_layout<_dest_t>::value, "primitive serialization target must be standard layout type.");

    static const bool required = _required;
    static const ID record_id = _record_id;
    static const intptr_t dest_offs = _dest_offs;
    typedef _dest_t dest_t;
    typedef _record_t record_t;

    static inline void deserialize(const record_t *rec, void *dest)
        {
            *((dest_t*)((intptr_t)(dest) + dest_offs)) = rec->get();
        }
};

template <ID _record_id, class _record_t, typename _struct_t, void (_struct_t::*setfunc)(const _record_t*), bool _required=false>
struct deserialize_custom {
    static const ID record_id = _record_id;
    static const bool required = _required;
    typedef _record_t record_t;

    typedef _struct_t struct_t;

    static inline void deserialize(const record_t *rec, struct_t *dest)
        {
            (dest->*setfunc)(rec);
        }
};

template <ID _record_id, class _record_t, typename _struct_t, typename buf_t, void (_struct_t::*setfunc)(const buf_t*, const intptr_t), bool _required=false>
struct deserialize_buffer
{
    static const ID record_id = _record_id;
    static const bool required = _required;
    typedef _record_t record_t;

    typedef _struct_t struct_t;

    static inline void deserialize(const record_t *rec, struct_t *dest)
    {
        (dest->*setfunc)(rec->dataptr(), rec->datalen());
    }
};

template <ID _record_id, class _record_t, typename _struct_t, void (_struct_t::*setfunc)(const std::string&), bool _required=false>
struct deserialize_string
{
    static const ID record_id = _record_id;
    static const bool required = _required;
    typedef _record_t record_t;

    typedef _struct_t struct_t;

    static inline void deserialize(const record_t *rec, struct_t *dest)
    {
        (dest->*setfunc)(rec->datastr());
    }
};

template <typename _struct_t, typename... field_ts>
struct deserialize_block
{
};


template <typename _struct_t, typename field_t, typename... field_ts>
struct deserialize_block<_struct_t, field_t, field_ts...>
{
    typedef _struct_t struct_t;

    static inline void deserialize(Container *node, struct_t *dest)
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
                const typename field_t::record_t *rec = dynamic_cast<const typename field_t::record_t*>(child);
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
            deserialize_block<struct_t, field_ts...>::deserialize(node, dest);
        }
};

template <typename _struct_t>
struct deserialize_block<_struct_t>
{
    typedef _struct_t struct_t;

    static inline void deserialize(Container *node, _struct_t *dest)
        {

        };
};

}

#endif
