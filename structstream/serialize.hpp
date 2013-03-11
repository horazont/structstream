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
#include "structstream/streaming_base.hpp"

namespace StructStream {

template <typename A, typename B>
struct most_subclassed
{
    static_assert(std::is_class<A>::value && std::is_class<B>::value, "most_subclassed requires classes as arguments.");

    typedef typename std::conditional<
        std::is_base_of<A, B>::value,
        B,
        A>::type type;
};

template <typename A, typename B>
struct common_struct_type
{
    typedef typename most_subclassed<A, B>::type type;
    static_assert(std::is_base_of<A, type>::value, "A and B do not share a common descendant.");
    static_assert(std::is_base_of<B, type>::value, "A and B do not share a common descendant.");
};

template <typename A>
struct common_struct_type<A, void>
{
    typedef A type;
};

template <typename A>
struct common_struct_type<void, A>
{
    typedef A type;
};


template <typename deserializer>
StreamSink deserialize(typename deserializer::arg_t dest)
{
    return StreamSink(new typename deserializer::deserializer(dest));
}

template <typename deserializer>
typename deserializer::deserializer deserializer_obj(typename deserializer::dest_t &dest)
{
    return typename deserializer::deserializer(dest);
}


template <typename _record_t, ID _id, typename _dest_t, typename member_t, intptr_t offset>
struct member_raw
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        deserializer(dest_t &dest): _dest(dest) {};
        virtual ~deserializer() = default;
    private:
        dest_t& _dest;
    public:
        virtual bool push_node(NodeHandle node) {
            record_t *rec = static_cast<record_t*>(node.get());
            dest_t *dest_ptr = &_dest;
            *reinterpret_cast<member_t*>(reinterpret_cast<uint8_t*>(dest_ptr) + offset) = rec->get();
            return true;
        };
    };
};

template <typename _record_t, ID _id, typename _dest_t, typename member_t, member_t _dest_t::*member_ptr>
struct member
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        deserializer(dest_t &dest): _dest(dest) {};
        virtual ~deserializer() = default;
    private:
        dest_t& _dest;
    public:
        virtual bool push_node(NodeHandle node) {
            record_t *rec = static_cast<record_t*>(node.get());
            ((&_dest)->*member_ptr) = rec->get();
            return true;
        };
    };
};

template <typename _record_t, ID _id, typename _dest_t, typename member_t, void (_dest_t::*setfunc)(member_t)>
struct member_cb
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        deserializer(dest_t &dest): _dest(dest) {};
        virtual ~deserializer() = default;
    private:
        dest_t& _dest;
    public:
        virtual bool push_node(NodeHandle node) {
            record_t *rec = static_cast<record_t*>(node.get());
            ((&_dest)->*setfunc)(rec->get());
            return true;
        };
    };
};

template <typename _record_t, ID _id, typename _dest_t, void (_dest_t::*setfunc)(const char*, const intptr_t)>
struct member_cb_len
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        deserializer(dest_t &dest): _dest(dest) {};
        virtual ~deserializer() = default;
    private:
        dest_t& _dest;
    public:
        virtual bool push_node(NodeHandle node) {
            record_t *rec = static_cast<record_t*>(node.get());
            ((&_dest)->*setfunc)(rec->dataptr(), rec->datalen());
            return true;
        };
    };
};

template <typename _record_t, ID _id, typename _dest_t, std::string _dest_t::*member_ptr>
struct member_string
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        deserializer(dest_t &dest): _dest(dest) {};
        virtual ~deserializer() = default;
    private:
        dest_t& _dest;
    public:
        virtual bool push_node(NodeHandle node) {
            record_t *rec = static_cast<record_t*>(node.get());
            ((&_dest)->*member_ptr) = rec->datastr();
            return true;
        };
    };
};

template <typename _record_t, ID _id, typename _dest_t, void (_dest_t::*setfunc)(const std::string &ref)>
struct member_string_cb
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        deserializer(dest_t &dest): _dest(dest) {};
        virtual ~deserializer() = default;
    private:
        dest_t& _dest;
    public:
        virtual bool push_node(NodeHandle node) {
            record_t *rec = static_cast<record_t*>(node.get());
            ((&_dest)->*setfunc)(rec->datastr());
            return true;
        };
    };
};

template <typename... member_ts>
struct struct_members
{
};

template <typename member_t, typename... member_ts>
struct struct_members<member_t, member_ts...>
{
private:
    typedef typename member_t::record_t member_record_t;
    static constexpr ID member_id = member_t::id;

    typedef struct_members<member_ts...> other_members;

public:
    typedef typename common_struct_type<typename member_t::dest_t, typename other_members::dest_t>::type dest_t;

    static constexpr int member_count = other_members::member_count + 1;

    static inline void initialize_sinks(StreamSink members[], dest_t &dest)
    {
        members[0] = std::move(initialize_my_sink(dest));
        other_members::initialize_sinks(&members[1], dest);
    }

    template <typename T>
    static inline T dispatch_node(T members[], const Node *node)
    {
        if (node->id() == member_id) {
            const member_record_t *rec = dynamic_cast<const member_record_t*>(node);
            if (rec != nullptr) {
                return members[0];
            }
        }

        return other_members::dispatch_node(&members[1], node);
    }
private:
    static inline StreamSink initialize_my_sink(dest_t &dest)
    {
        return std::move(StreamSink(new typename member_t::deserializer(dest)));
    }
};

template <>
struct struct_members<>
{
    static constexpr int member_count = 0;
    typedef void dest_t;

    template <typename T>
    static inline T dispatch_node(T members[], const Node *node)
    {
        return T();
    }

    template <typename U>
    static inline void initialize_sinks(StreamSink members[], U &ref)
    {
        // intentionally left blank
    }
};

template <typename _record_t, ID _id, typename members>
struct struct_decl
{
    typedef typename members::dest_t dest_t;
    typedef _record_t record_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    class deserializer: public SinkTree
    {
    public:
        deserializer(dest_t &dest):
            dest(dest)
            {
                members::initialize_sinks(member_sinks, dest);
            };
        virtual ~deserializer() = default;
    private:
        StreamSink member_sinks[members::member_count];
        dest_t &dest;
    protected:
        virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta) {
            Container *node = cont.get();
            StreamSink sink_to_use = members::dispatch_node(member_sinks, node);
            if (sink_to_use) {
                // printf("struct: found a way to deserialize rt %x, id %lx\n",
                //        node->record_type(),
                //        node->id());
                nest(sink_to_use);
            } else {
                // printf("struct: found NO way to deserialize rt %x, id %lx; skipping container\n",
                //        node->record_type(),
                //        node->id());
                nest(StreamSink(new NullSink()));
            }
            return true;
        };

        virtual bool _push_node(NodeHandle node_h) {
            Node *node = node_h.get();
            StreamSink sink_to_use = members::dispatch_node(member_sinks, node);
            if (sink_to_use) {
                // printf("struct: found a way to deserialize rt %x, id %lx\n",
                //        node->record_type(),
                //        node->id());
                sink_to_use->push_node(node_h);
            } else {
                // printf("struct: found NO way to deserialize rt %x, id %lx\n",
                //        node->record_type(),
                //        node->id());
            }
            return true;
        };

        virtual bool _end_container(const ContainerFooter *foot) {
            // nothing to do here.
            return true;
        };

        virtual void _end_of_stream() {
            // nothing to do here.
        };
    };
};

template <typename type, typename container>
struct iterator_helper
{
};

template <typename type>
struct iterator_helper<type, type*>
{
    static inline void assign(type **src, type **dest)
    {
        *dest = *src;
    }

    static inline type* construct()
    {
        return new type();
    }
};

template <typename type>
struct iterator_helper<type, type>
{
    static inline void assign(type *src, type **dest)
    {
        *dest = src;
    }

    static inline type construct()
    {
        return type();
    }
};

template <typename type>
struct iterator_helper<type, std::shared_ptr<type>>
{
    static inline void assign(std::shared_ptr<type> *src, type **dest)
    {
        *dest = src.get();
    }

    static inline std::shared_ptr<type> construct()
    {
        return std::shared_ptr<type>(new type());
    }
};

template <typename type>
struct iterator_helper<type, std::unique_ptr<type>>
{
    static inline void assign(std::unique_ptr<type> *src, type **dest)
    {
        *dest = src.get();
    }

    static inline std::unique_ptr<type> construct()
    {
        return std::unique_ptr<type>(new type());
    }
};

template <typename item_decl, typename element_type>
struct sequence
{
    typedef typename item_decl::record_t record_t;
    static constexpr ID id = item_decl::id;

    class deserializer: public SinkTree
    {
        typedef iterator_helper<typename item_decl::dest_t, element_type> helper;
        typedef typename item_decl::dest_t item_t;
    public:
        deserializer():
            _item()
        {

        };
        virtual ~deserializer() {};
    private:
        element_type _item;
    protected:
        virtual void submit_item(element_type &&item) = 0;
    public:
        virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta)
        {
            _item = helper::construct();
            item_t *dest;
            helper::assign(&_item, &dest);
            nest(deserialize<item_decl>(*dest));
            return true;
        };

        virtual bool _push_node(NodeHandle node)
        {
            _item = helper::construct();
            item_t *dest;
            helper::assign(&_item, &dest);
            deserializer_obj<item_decl>(*dest).push_node(node);
            submit_item(std::move(_item));
            return true;
        };

        virtual bool _end_container(const ContainerFooter *foot)
        {
            submit_item(std::move(_item));
            return true;
        };

        virtual void _end_of_stream()
        {

        };
    };
};

template <typename item_decl,
          typename output_iterator,
          typename element_type = typename output_iterator::container_type::value_type>
struct iterator
{
    typedef sequence<item_decl, typename output_iterator::container_type::value_type> sequence_decl;
    typedef typename sequence_decl::record_t record_t;
    static constexpr ID id = sequence_decl::id;
    typedef output_iterator arg_t;

    class deserializer: public sequence_decl::deserializer
    {
    public:
        deserializer(arg_t dest):
            sequence_decl::deserializer::deserializer(),
            _iterator(dest)
        {

        };
        virtual ~deserializer() {};
    private:
        output_iterator _iterator;
    protected:
        virtual void submit_item(element_type &&item)
        {
            *_iterator++ = std::move(item);
        };
    };
};

template <typename item_decl, typename output_iterator>
struct container
{
    typedef iterator<item_decl,
                     output_iterator,
                     typename output_iterator::container_type::value_type>
            iterator_decl;
    typedef typename output_iterator::container_type dest_t;
    typedef typename iterator_decl::record_t record_t;
    static constexpr ID id = iterator_decl::id;
    typedef dest_t& arg_t;

    class deserializer: public iterator_decl::deserializer
    {
    public:
        deserializer(dest_t &dest):
            iterator_decl::deserializer::deserializer(output_iterator(dest)),
            _dest(dest)
        {

        };
        virtual ~deserializer() {};
    private:
        dest_t &_dest;
    };
};

template <typename _record_t, ID _id, typename _dest_t>
struct value
{
    typedef _dest_t dest_t;
    typedef _record_t record_t;
    static constexpr ID id = _id;
    typedef dest_t& arg_t;

    typedef typename struct_decl<
        record_t,
        id,
        struct_members<
            member_raw<record_t, id, dest_t, dest_t, 0>
            >
        >::deserializer deserializer;
};

template <typename deserializer>
struct deserialize_one: public SinkTree
{
    typedef typename deserializer::dest_t dest_t;
    typedef typename deserializer::record_t record_t;
    static constexpr ID id = deserializer::id;
public:
    deserialize_one(dest_t &dest):
        dest(dest)
    {

    };
    virtual ~deserialize_one() = default;
private:
    dest_t &dest;
    StreamSink _deserializer;
public:
    virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta)
    {
        if (cont->id() == id) {
            record_t *rec = dynamic_cast<record_t*>(cont.get());
            if (rec != nullptr) {
                _deserializer = StreamSink(new deserializer(dest));
                nest(_deserializer);
            }
        }
        return true;
    };

    virtual bool _push_node(NodeHandle node)
    {
        if (node->id() == id) {
            record_t *rec = dynamic_cast<record_t*>(node.get());
            if (rec != nullptr) {
                deserializer(dest).push_node(node);
                return false;
            }
        }
        return true;
    };

    virtual bool _end_container(const ContainerFooter *foot)
    {
        return !_deserializer;
    };

    virtual void _end_of_stream()
    {

    };
};

template <typename item_decl>
struct only
{
    typedef typename item_decl::dest_t dest_t;
    typedef typename item_decl::arg_t arg_t;
    typedef typename item_decl::record_t record_t;
    static constexpr ID id = item_decl::id;

    class deserializer: public SinkTree
    {
    public:
        deserializer(arg_t dest):
            _deserializer(deserialize<item_decl>(dest))
        {

        };
        virtual ~deserializer() = default;
    private:
        StreamSink _deserializer;
    public:
        virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta)
            {
                if (cont->id() == id) {
                    record_t *rec = dynamic_cast<record_t*>(cont.get());
                    if (rec != nullptr) {
                        nest(_deserializer);
                    }
                }
                return true;
            };

        virtual bool _push_node(NodeHandle node)
            {
                if (node->id() == id) {
                    record_t *rec = dynamic_cast<record_t*>(node.get());
                    if (rec != nullptr) {
                        _deserializer->push_node(node);
                    }
                }
                return true;
            };

        virtual bool _end_container(const ContainerFooter *foot)
            {
                return true;
            };

        virtual void _end_of_stream()
            {

            };
    };
};


}

#endif
