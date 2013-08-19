/**********************************************************************
File name: serialize.hpp
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

#include <stdexcept>

#include "structstream/static.hpp"
#include "structstream/errors.hpp"
#include "structstream/node_container.hpp"
#include "structstream/streaming_sinks.hpp"
#include "structstream/streaming_tree.hpp"

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

template <typename deserializer, typename... arg_ts>
StreamSink deserialize(typename deserializer::deserializer::arg_t dest, arg_ts... args)
{
    return StreamSink(new typename deserializer::deserializer(dest));
}

template <typename deserializer, typename... arg_ts>
typename deserializer::deserializer deserializer_obj(typename deserializer::deserializer::arg_t dest, arg_ts... args)
{
    return typename deserializer::deserializer(dest, args...);
}

template <typename decl>
void serialize_to_sink(typename decl::serializer::arg_t src, StreamSink sink)
{
    decl::serializer::to_sink(src, sink);
}

template <typename _record_t, ID _id, typename _dest_t, typename member_t, intptr_t offset>
struct member_raw
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        typedef dest_t& arg_t;
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

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(const dest_t &obj, StreamSink sink)
        {
            NodeHandle node = NodeHandleFactory<record_t>::create(id);
            record_t *rec = static_cast<record_t*>(node.get());
            rec->set(*reinterpret_cast<const member_t*>(reinterpret_cast<const uint8_t*>(&obj) + offset));
            sink->push_node(node);
        }
    };
};

template <typename _record_t, ID _id, typename _dest_t, std::shared_ptr<_record_t> _dest_t::*member_ptr>
struct member_record
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;

    class deserializer: public ToTree
    {
    public:
        typedef dest_t& arg_t;
    public:
        deserializer(arg_t dest):
            ToTree(NodeHandleFactory<Container>::create(_id)),
            _first_event(true),
            _dest(dest)
        {
        };
    private:
        bool _first_event;
        arg_t _dest;

        inline void set_up_root()
        {
            if (std::is_base_of<Container, record_t>::value) {
                _dest.*member_ptr = root();
            }
        }
    public:
        virtual bool start_container(ContainerHandle cont, const ContainerMeta* meta)
        {
            if (_first_event) {
                set_up_root();
                _first_event = false;
            }
            return ToTree::start_container(cont, meta);
        };

        virtual bool push_node(NodeHandle node) {
            if (_first_event) {
                set_up_root();
                _first_event = false;
            }

            if (std::is_base_of<Container, record_t>::value) {
                return ToTree::push_node(node);
            } else {
                _dest.*member_ptr = std::static_pointer_cast<_record_t>(node);
            }
        };
    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t obj, StreamSink sink)
        {
            std::shared_ptr<_record_t> node = obj.*member_ptr;
            if (!node) {
                return;
            }
            if (std::is_base_of<Container, record_t>::value) {
                FromTree(sink, {node}, false);
            } else {
                sink->push_node(node);
            }
        }
    };
};

template <typename _record_t, ID _id, typename _dest_t,
          std::shared_ptr<_record_t> (_dest_t::*get_record)(ID id) const,
          void (_dest_t::*set_record)(const std::shared_ptr<_record_t>&)>
struct member_direct
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;

    static_assert(!std::is_base_of<Container, record_t>::value,
                  "member_direct cannot be used with containers.");

    class deserializer: public ThrowOnAll
    {
    public:
        typedef dest_t& arg_t;
    public:
        deserializer(arg_t dest):
            _dest(dest)
        {

        };
    private:
        arg_t _dest;
    public:
        virtual bool push_node(NodeHandle node) {
            std::shared_ptr<record_t> rec = std::static_pointer_cast<record_t>(node);
            (_dest.*set_record)(rec);
            return true;
        };
    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t obj, StreamSink sink)
        {
            sink->push_node((obj.*get_record)(id));
        };
    };
};

struct value_extractor
{
    /* SFINAE at work to pick the best implementation */

    template <typename record_t, typename value_t>
    static inline auto assign(record_t *rec, value_t &value, long foo = 0)
        -> decltype(rec->get(), (void)0)
    {
        /* use this if a compatible get() is available */
        value = rec->get();
    }


    template <typename record_t, typename value_t>
    static inline auto assign(record_t *rec, value_t &value, int foo = 0)
        -> decltype(rec->datastr(), (void)0)
    {
        /* use this otherwise; will not work always, but for some
           types which do not offer string getters */
        value = rec->datastr();
    }
};

template <typename _record_t, ID _id, typename _dest_t, typename member_t, member_t _dest_t::*member_ptr>
struct member
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        typedef dest_t& arg_t;
    public:
        deserializer(dest_t &dest):
            _dest(dest)
        {
        };
        virtual ~deserializer() = default;
    private:
        dest_t& _dest;
    public:
        virtual bool push_node(NodeHandle node)
        {
            record_t *rec = static_cast<record_t*>(node.get());
            value_extractor::assign<record_t, member_t>(
                std::move(rec),
                _dest.*member_ptr,
                0L
                );
            return true;
        };
    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(const dest_t &obj, StreamSink sink)
        {
            NodeHandle node = NodeHandleFactory<record_t>::create(id);
            record_t *rec = static_cast<record_t*>(node.get());
            rec->set(obj.*member_ptr);
            sink->push_node(node);
        }
    };
};

template <typename _record_t, ID _id, typename _dest_t, typename member_t, const member_t& (_dest_t::*getfunc)() const, void (_dest_t::*setfunc)(const member_t&)>
struct member_cb
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        typedef dest_t& arg_t;
    public:
        deserializer(dest_t &dest):
            _dest(dest)
        {
        };
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

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t obj, StreamSink sink)
        {
            NodeHandle node = NodeHandleFactory<record_t>::create(id);
            record_t *rec = static_cast<record_t*>(node.get());
            rec->set((obj.*getfunc)());
            sink->push_node(node);
        }
    };
};

template <typename record_t, ID id, typename dest_t, std::string dest_t::*member_ptr>
using member_string = member<record_t, id, dest_t, std::string, member_ptr>;

template <typename record_t, typename dest_t, void (dest_t::*setfunc)(const std::string &ref)>
class deserialize_member_string_cb: public ThrowOnAll
{
public:
    typedef dest_t& arg_t;
public:
    deserialize_member_string_cb(dest_t &dest): _dest(dest) {};
    virtual ~deserialize_member_string_cb() = default;
private:
    dest_t& _dest;
public:
    virtual bool push_node(NodeHandle node) {
        record_t *rec = static_cast<record_t*>(node.get());
        ((&_dest)->*setfunc)(rec->datastr());
        return true;
    };
};

template <typename record_t, ID id, typename src_t, const std::string& (src_t::*getfunc)() const>
struct serialize_member_string_cb
{
    typedef const src_t& arg_t;

    static inline void to_sink(arg_t obj, StreamSink sink)
    {
        NodeHandle node = NodeHandleFactory<record_t>::create(id);
        record_t *rec = static_cast<record_t*>(node.get());
        rec->set(((&obj)->*getfunc)());
        sink->push_node(node);
    }
};

// template <typename _record_t, ID _id, typename _dest_t, const std::string &(_dest_t::*getfunc)() const, void (_dest_t::*setfunc)(const std::string &ref), typename enabled = void>
// struct member_string_cb;

// template <typename _record_t, ID _id, typename _dest_t, const std::string &(_dest_t::*getfunc)() const, void (_dest_t::*setfunc)(const std::string &ref)>
// struct member_string_cb<_record_t, _id, _dest_t, getfunc, setfunc, typename std::enable_if<(setfunc == nullptr) && (getfunc == nullptr)>::type>
// {
//     typedef _record_t record_t;
//     typedef _dest_t dest_t;
//     typedef dest_t& arg_t;
//     static constexpr ID id = _id;
// };

// template <typename _record_t, ID _id, typename _dest_t, const std::string &(_dest_t::*getfunc)() const, void (_dest_t::*setfunc)(const std::string &ref)>
// struct member_string_cb<_record_t, _id, _dest_t, getfunc, setfunc, typename std::enable_if<(setfunc != nullptr) && (getfunc == nullptr)>::type>
// {
//     typedef _record_t record_t;
//     typedef _dest_t dest_t;
//     typedef dest_t& arg_t;
//     static constexpr ID id = _id;

//     typedef deserialize_member_string_cb<record_t, dest_t, setfunc> deserializer;
// };

template <typename _record_t, ID _id, typename _dest_t, const std::string &(_dest_t::*getfunc)() const, void (_dest_t::*setfunc)(const std::string &ref)>
// struct member_string_cb<_record_t, _id, _dest_t, getfunc, setfunc, typename std::enable_if<(setfunc != nullptr) && (getfunc != nullptr)>::type>
struct member_string_cb
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    typedef dest_t& arg_t;
    static constexpr ID id = _id;

    typedef deserialize_member_string_cb<record_t, dest_t, setfunc> deserializer;
    typedef serialize_member_string_cb<record_t, id, dest_t, getfunc> serializer;
};

// template <typename _record_t, ID _id, typename _dest_t, const std::string &(_dest_t::*getfunc)() const, void (_dest_t::*setfunc)(const std::string &ref)>
// struct member_string_cb<_record_t, _id, _dest_t, getfunc, setfunc, typename std::enable_if<(setfunc == nullptr) && (getfunc != nullptr)>::type>
// {
//     typedef _record_t record_t;
//     typedef _dest_t dest_t;
//     typedef dest_t& arg_t;
//     static constexpr ID id = _id;

//     typedef serialize_member_string_cb<record_t, id, dest_t, getfunc> serializer;
// };

template <typename _dest_t, typename struct_t, typename struct_t::dest_t _dest_t::*member_ptr, ID struct_id = struct_t::id>
struct member_struct
{
    typedef typename struct_t::record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = struct_id;

    class deserializer: public struct_t::deserializer
    {
    public:
        typedef dest_t& arg_t;
    public:
        deserializer(arg_t dest):
            struct_t::deserializer::deserializer(dest.*member_ptr)
        {

        }
        virtual ~deserializer() {};
    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t obj, StreamSink sink)
        {
            struct_t::serializer::to_sink(obj.*member_ptr, sink);
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

    static inline void to_sink(const dest_t &obj, StreamSink sink)
    {
        member_t::serializer::to_sink(obj, sink);
        other_members::to_sink(obj, sink);
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

    template <typename U>
    static inline void to_sink(const U &obj, StreamSink sink)
    {
        // intentionally left blank
    }
};

template <typename _record_t, ID _id, typename members, bool strict=false>
struct struct_decl
{
    typedef typename members::dest_t dest_t;
    typedef _record_t record_t;
    static constexpr ID id = _id;

    static_assert(std::is_base_of<Container, record_t>::value, "Struct record_t must be a container subclass.");

    class deserializer: public SinkTree
    {
    public:
        typedef dest_t& arg_t;
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
        static inline void throw_unknown_child(NodeHandle child)
        {
            throw std::runtime_error("Unknown struct child found: rt="
                                     + std::to_string(child->record_type())
                                     + "; id=" + std::to_string(child->id())
                                     + "; typeid=." + typeid(*child.get()).name());
        };

        virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta) {
            Container *node = cont.get();
            StreamSink sink_to_use = members::dispatch_node(member_sinks, node);
            if (sink_to_use) {
                // printf("struct: found a way to deserialize rt %x, id %lx\n",
                //        node->record_type(),
                //        node->id());
                nest(sink_to_use);
            } else {
                if (strict)
                    throw_unknown_child(cont);
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
                //  printf("struct: found NO way to deserialize rt %x, id %lx\n",
                //         node->record_type(),
                //         node->id());
                if (strict)
                    throw_unknown_child(node_h);
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

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(const dest_t &obj, StreamSink sink)
        {
            ContainerHandle node = NodeHandleFactory<record_t>::create(id);
            ContainerMeta *meta = new ContainerMeta();
            try {
                sink->start_container(node, meta);
            } catch (...) {
                delete meta;
                throw;
            }

            members::to_sink(obj, sink);

            ContainerFooter *foot = new ContainerFooter();
            try {
                sink->end_container(foot);
            } catch (...) {
                delete foot;
                delete meta;
                throw;
            }
            delete foot;
            delete meta;
        }
    };
};

template <typename type, typename container>
struct iterator_helper;

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
        *dest = src->get();
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
        *dest = src->get();
    }

    static inline std::unique_ptr<type> construct()
    {
        return std::unique_ptr<type>(new type());
    }
};

template <typename item_decl, typename element_type>
struct deserialize_sequence
{
    typedef typename item_decl::record_t record_t;
    static constexpr ID id = item_decl::id;

    class deserializer: public SinkTree
    {
    private:
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
          typename element_type>
struct deserialize_sequence_cb
{
    typedef typename item_decl::record_t record_t;
    static constexpr ID id = item_decl::id;

    class deserializer: public deserialize_sequence<item_decl, element_type>::deserializer
    {
    public:
        typedef std::function< void(element_type&& item) > arg_t;
    public:
        deserializer(arg_t callback):
            _callback(callback)
        {

        };
    private:
        arg_t _callback;
    protected:
        virtual void submit_item(element_type &&item)
        {
            _callback(std::move(item));
        }
    };
};

template <typename item_decl,
          typename output_iterator,
          typename input_iterator,
          typename element_type = typename output_iterator::container_type::value_type>
struct iterator
{
    typedef deserialize_sequence<item_decl, typename output_iterator::container_type::value_type> sequence_decl;
    typedef typename sequence_decl::record_t record_t;
    static constexpr ID id = sequence_decl::id;
    typedef output_iterator arg_t;

    class deserializer: public sequence_decl::deserializer
    {
    public:
        typedef output_iterator arg_t;
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

    struct serializer
    {
        static inline void to_sink(input_iterator begin,
                                   input_iterator end,
                                   StreamSink sink)
        {
            for (; begin != end; begin++) {
                item_decl::serializer::to_sink(*begin, sink);
            }
        }
    };
};

template <typename output_iterator_t>
struct iterator_initializer
{
    static output_iterator_t init(
        typename output_iterator_t::container_type &dest)
    {
        return output_iterator_t(dest);
    }
};

template <typename container_t>
struct iterator_initializer<std::insert_iterator<container_t>>
{
    typedef std::insert_iterator<container_t> iterator_t;

    static iterator_t init(
        typename iterator_t::container_type &dest)
    {
        return iterator_t(dest, dest.begin());
    }
};

template <typename item_decl, typename output_iterator, typename input_iterator = typename output_iterator::container_type::const_iterator, typename iterator_initializer_t = iterator_initializer<output_iterator>>
struct container
{
    typedef iterator<item_decl,
                     output_iterator,
                     input_iterator,
                     typename output_iterator::container_type::value_type>
            iterator_decl;
    typedef typename output_iterator::container_type dest_t;
    typedef typename iterator_decl::record_t record_t;
    static constexpr ID id = iterator_decl::id;

    class deserializer: public iterator_decl::deserializer
    {
    public:
        typedef dest_t& arg_t;
    public:
        deserializer(dest_t &dest):
            iterator_decl::deserializer::deserializer(iterator_initializer_t::init(dest)),
            _dest(dest)
        {

        };
        virtual ~deserializer() {};
    private:
        dest_t &_dest;
    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(const dest_t &obj, StreamSink sink)
        {
            iterator_decl::serializer::to_sink(obj.cbegin(), obj.cend(), sink);
        }
    };
};

template <typename _record_t, ID _id, typename _dest_t>
struct value_decl
{
    typedef _dest_t dest_t;
    typedef _record_t record_t;
    static constexpr ID id = _id;

    class deserializer: public ThrowOnAll
    {
    public:
        typedef dest_t& arg_t;
    public:
        deserializer(arg_t dest):
            _dest(dest)
        {
        };
        virtual ~deserializer() {};
    private:
        arg_t _dest;
    public:
        virtual bool push_node(NodeHandle node)
        {
            record_t *rec = dynamic_cast<record_t*>(node.get());
            value_extractor::assign<record_t, dest_t>(rec, _dest);
            return true;
        };
    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        inline static void to_sink(arg_t obj, StreamSink sink)
        {
            auto node = NodeHandleFactory<record_t>::create(id);
            node->set(obj);
            sink->push_node(node);
        };
    };
};

template <typename deserializer>
struct deserialize_one: public SinkTree
{
    typedef typename deserializer::dest_t dest_t;
    typedef typename deserializer::record_t record_t;
    typedef typename deserializer::deserializer::arg_t arg_t;
    static constexpr ID id = deserializer::id;
public:
    deserialize_one(arg_t dest):
        dest(dest)
    {

    };
    virtual ~deserialize_one() = default;
private:
    arg_t dest;
    StreamSink _deserializer;
public:
    virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta)
    {
        if (cont->id() == id) {
            record_t *rec = dynamic_cast<record_t*>(cont.get());
            if (rec != nullptr) {
                _deserializer = deserialize<deserializer>(dest);
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
                deserializer_obj<deserializer>(dest).push_node(node);
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

template <typename item_decl, bool require=false, bool first=false>
struct only
{
    typedef typename item_decl::dest_t dest_t;
    typedef typename item_decl::record_t record_t;
    static constexpr ID id = item_decl::id;

    class deserializer: public SinkTree
    {
    public:
        typedef typename item_decl::deserializer::arg_t arg_t;
    public:
        deserializer(arg_t dest):
            _deserializer(deserialize<item_decl>(dest)),
            _found(false)
        {

        };
        virtual ~deserializer() = default;

    private:
        StreamSink _deserializer;
        bool _found;

    public:
        virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta)
        {
            if (cont->id() == id) {
                record_t *rec = dynamic_cast<record_t*>(cont.get());
                if (rec != nullptr) {
                    nest(_deserializer);
                    _found = true;
                    return true;
                }
            }
            if (first) {
                throw RecordNotFound("Record required by only<> was not found (as first).");
            }
            return true;
        };

        virtual bool _push_node(NodeHandle node)
        {
            if (node->id() == id) {
                record_t *rec = dynamic_cast<record_t*>(node.get());
                if (rec != nullptr) {
                    _deserializer->push_node(node);
                    _found = true;
                    return false;
                }
            }
            if (first) {
                throw RecordNotFound("Record required by only<> was not found (as first).");
            }
            return true;
        };

        virtual bool _end_container(const ContainerFooter *foot)
        {
            return false;
        };

        virtual void _end_of_stream()
        {
            if (!_found && require) {
                throw RecordNotFound("Record required by only<> was not found.");
            }
        };
    };
};

template <typename _dest_t,
          typename item_decl,
          typename input_iterator,
          input_iterator (_dest_t::*iter_begin)() const,
          input_iterator (_dest_t::*iter_end)() const,
          void (_dest_t::*add_item)(typename std::remove_const<typename input_iterator::value_type>::type &&element_type)>
struct member_sequence_cb
{
    typedef _dest_t dest_t;
    typedef typename item_decl::record_t record_t;
    static constexpr ID id = item_decl::id;

private:
    typedef typename input_iterator::value_type element_type;
    typedef deserialize_sequence<item_decl, element_type> sequence_decl;
public:

    class deserializer: public sequence_decl::deserializer
    {
    public:
        typedef dest_t& arg_t;
    public:
        deserializer(arg_t dest):
            sequence_decl::deserializer::deserializer(),
            _dest(dest)
        {

        };
    private:
        arg_t _dest;
    protected:
        virtual void submit_item(element_type &&item)
        {
            (_dest.*add_item)(std::move(item));
        };
    };

    struct serializer
    {
    private:
        typedef iterator_helper<const typename item_decl::dest_t, const element_type> helper;
    public:
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t obj, StreamSink sink)
        {
            input_iterator curr = (obj.*iter_begin)();
            input_iterator end = (obj.*iter_end)();
            for (; curr != end; curr++) {
                const typename item_decl::dest_t *item;
                helper::assign(&*curr, &item);
                item_decl::serializer::to_sink(*item, sink);
            }
        };
    };
};

}

#endif
