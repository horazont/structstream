/**********************************************************************
File name: serialize_struct.hpp
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
#ifndef _STRUCTSTREAM_SERIALIZE_STRUCT_H
#define _STRUCTSTREAM_SERIALIZE_STRUCT_H

#include <cassert>

#include "structstream/serialize_base.hpp"
#include "structstream/serialize_utils.hpp"

namespace StructStream {

/* leaf serializers */

template <typename _record_t,
          typename _selector_t,
          typename _struct_t,
          typename _value_t,
          _value_t (_struct_t::*_value_ptr)>
struct member
{
    typedef _record_t record_t;
    typedef _selector_t selector_t;
    typedef _struct_t dest_t;

    struct deserializer: public deserializer_base
    {
        typedef dest_t& arg_t;

        deserializer(arg_t dest):
            _dest(dest)
        {

        };

    private:
        dest_t &_dest;

    public:
        bool node(const NodeHandle &node) override
        {
            record_t *rec = dynamic_cast<record_t*>(node.get());
            assert(rec);
            value_helper<record_t, _value_t>::from_record(
                rec,
                _dest.*_value_ptr);
            return true;
        };

    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t src, const StreamSink &sink)
        {
            sink->push_node(
                value_helper<record_t, _value_t>::to_record(
                    src.*_value_ptr, selector_t::first));
        };
    };
};

template <typename _struct_t,
          typename _selector_t,
          typename _record_t,
          std::shared_ptr<_record_t> (_struct_t::*get_record)(ID id) const,
          void (_struct_t::*set_record)(const std::shared_ptr<_record_t>&)>
struct member_direct
{
    typedef _struct_t dest_t;
    typedef _selector_t selector_t;
    typedef _record_t record_t;

    static_assert(!std::is_base_of<Container, record_t>::value,
                  "member_direct does not support Containers.");

    struct deserializer: public deserializer_base
    {
        typedef dest_t& arg_t;

        deserializer(arg_t dest):
            _dest(dest)
        {

        }

    private:
        dest_t &_dest;

    public:
        bool node(const NodeHandle &node) override
        {
            std::shared_ptr<record_t> rec =
                std::static_pointer_cast<record_t>(node);

            (_dest.*set_record)(rec);
            return true;
        };

    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t src, const StreamSink sink)
        {
            sink->push_node(
                (src.*get_record)(selector_t::first));
        }
    };
};

template <typename _struct_t,
          typename _value_decl,
          typename _value_decl::dest_t (_struct_t::*_value_ptr),
          typename _selector_t = typename _value_decl::selector_t>
struct member_struct
{
    typedef typename _value_decl::record_t record_t;
    typedef _selector_t selector_t;
    typedef _struct_t dest_t;

    struct deserializer: public _value_decl::deserializer
    {
        typedef dest_t& arg_t;

        deserializer(arg_t dest):
            _value_decl::deserializer(dest.*_value_ptr)
        {

        };

    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t src, const StreamSink &sink)
        {
            ContainerHandle parent =
                NodeHandleFactory<Container>::create(selector_t::first);
            ContainerMeta meta;
            sink->start_container(parent, &meta);

            _value_decl::serializer::to_sink(src.*_value_ptr, sink);

            ContainerFooter foot;
            sink->end_container(&foot);
        }
    };
};

/* struct */

template <typename... member_ts>
struct struct_members;

template <typename member_t, typename... member_ts>
struct struct_members<member_t, member_ts...>
{
private:
    typedef typename member_t::record_t member_record_t;
    typedef typename member_t::selector_t member_selector_t;
    typedef struct_members<member_ts...> other_members;

public:
    /* find the common base type of all types used by the members */
    typedef typename common_struct_type<
        typename member_t::dest_t,
        typename other_members::dest_t>::type dest_t;

    static constexpr size_t member_count =
        other_members::member_count + 1;

    static inline deserializer_base *dispatch_node(
        const Node *node, dest_t &dest)
    {
        if (member_selector_t::test(node->id())) {
            const member_record_t *rec =
                dynamic_cast<const member_record_t*>(node);
            if (rec) {
                return new typename member_t::deserializer(dest);
            }
        }
        return other_members::dispatch_node(node, dest);
    };

    static inline void to_sink(
        const dest_t &src,
        const StreamSink &sink)
    {
        member_t::serializer::to_sink(src, sink);
        other_members::to_sink(src, sink);
    };
};

template <>
struct struct_members<>
{
public:
    typedef void dest_t;
    static constexpr size_t member_count = 0;

    template <typename U>
    static inline deserializer_base *dispatch_node(
        const Node *node, U dest)
    {
        return nullptr;
    }

    template <typename U>
    static inline void to_sink(U src, const StreamSink &sink)
    {

    }
};

template <typename _record_t, typename _selector_t, typename members_t>
struct struct_decl
{
    typedef _record_t record_t;
    typedef _selector_t selector_t;
    typedef typename members_t::dest_t dest_t;

    static_assert(!std::is_same<dest_t, void>::value,
                  "Could not deduce common base type for struct_decl. "
                  "Make sure that your members' structs share a common"
                  " base.");

    struct deserializer: public deserializer_nesting
    {
        typedef dest_t& arg_t;

        deserializer(arg_t dest):
            _dest(dest)
        {

        }

    private:
        dest_t &_dest;

    private:
        inline bool handle_unknown_node(const NodeHandle &node)
        {
            return true;
        };

        inline bool handle_unknown_cont(const ContainerHandle &cont)
        {
            bool continue_streaming = handle_unknown_node(cont);
            if (continue_streaming) {
                nest(new deserializer_null());
            }
            return continue_streaming;
        };

    protected:
        bool _end_container() override
        {
            return true;
        };

        bool _node(const NodeHandle &node) override
        {
            deserializer_base *handler = members_t::dispatch_node(
                node.get(), _dest);
            if (handler == nullptr) {
                return handle_unknown_node(node);
            } else {
                handler->node(node);
                delete handler;
            }
            return true;
        };

        bool _start_container(const ContainerHandle &cont) override
        {
            deserializer_base *handler = members_t::dispatch_node(
                cont.get(), _dest);
            if (handler == nullptr) {
                return handle_unknown_cont(cont);
            } else {
                nest(handler);
            }
            return true;
        };

    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t src, StreamSink sink)
        {
            ContainerHandle parent =
                NodeHandleFactory<Container>::create(
                    selector_t::first);

            ContainerMeta meta;
            sink->start_container(parent, &meta);

            members_t::to_sink(src, sink);

            ContainerFooter foot;
            sink->end_container(&foot);
        };
    };
};


}

#endif
