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


template <typename _record_t, ID _id, typename _dest_t, typename member_t, intptr_t offset>
struct deserialize_member_raw: public ThrowOnAll
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;

public:
    deserialize_member_raw(dest_t &dest): _dest(dest) {};
    virtual ~deserialize_member_raw() = default;
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

template <typename _record_t, ID _id, typename _dest_t, typename member_t, member_t _dest_t::*member_ptr>
struct deserialize_member: public ThrowOnAll
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;

public:
    deserialize_member(dest_t &dest): _dest(dest) {};
    virtual ~deserialize_member() = default;
private:
    dest_t& _dest;
public:
    virtual bool push_node(NodeHandle node) {
        record_t *rec = static_cast<record_t*>(node.get());
        ((&_dest)->*member_ptr) = rec->get();
        return true;
    };
};

template <typename _record_t, ID _id, typename _dest_t, typename member_t, void (_dest_t::*setfunc)(member_t)>
struct deserialize_member_cb: public ThrowOnAll
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;
public:
    deserialize_member_cb(dest_t &dest): _dest(dest) {};
    virtual ~deserialize_member_cb() = default;
private:
    dest_t& _dest;
public:
    virtual bool push_node(NodeHandle node) {
        record_t *rec = static_cast<record_t*>(node.get());
        ((&_dest)->*setfunc)(rec->get());
        return true;
    };
};

template <typename _record_t, ID _id, typename _dest_t, void (_dest_t::*setfunc)(const char*, const intptr_t)>
struct deserialize_member_cb_len: public ThrowOnAll
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;
public:
    deserialize_member_cb_len(dest_t &dest): _dest(dest) {};
    virtual ~deserialize_member_cb_len() = default;
private:
    dest_t& _dest;
public:
    virtual bool push_node(NodeHandle node) {
        record_t *rec = static_cast<record_t*>(node.get());
        ((&_dest)->*setfunc)(rec->dataptr(), rec->datalen());
        return true;
    };
};

template <typename _record_t, ID _id, typename _dest_t, std::string _dest_t::*member_ptr>
struct deserialize_member_string: public ThrowOnAll
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;
public:
    deserialize_member_string(dest_t &dest): _dest(dest) {};
    virtual ~deserialize_member_string() = default;
private:
    dest_t& _dest;
public:
    virtual bool push_node(NodeHandle node) {
        record_t *rec = static_cast<record_t*>(node.get());
        ((&_dest)->*member_ptr) = rec->datastr();
        return true;
    };
};

template <typename _record_t, ID _id, typename _dest_t, void (_dest_t::*setfunc)(const std::string &ref)>
struct deserialize_member_string_cb: public ThrowOnAll
{
    typedef _record_t record_t;
    typedef _dest_t dest_t;
    static constexpr ID id = _id;
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

    static inline StreamSink dispatch_node(StreamSink members[], const Node *node)
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
        return std::move(StreamSink(new member_t(dest)));
    }
};

template <>
struct struct_members<>
{
    static constexpr int member_count = 0;
    typedef void dest_t;

    static inline StreamSink dispatch_node(StreamSink members[], const Node *node)
    {
        return StreamSink();
    }

    template <typename U>
    static inline void initialize_sinks(StreamSink members[], U &ref)
    {
        // intentionally left blank
    }
};

template <typename _record_t, ID _id, typename members>
class deserialize_struct: public SinkTree
{
public:
    typedef typename members::dest_t dest_t;
    typedef _record_t record_t;
    static constexpr ID id = _id;
public:
    deserialize_struct(dest_t &dest):
        dest(dest)
    {
        members::initialize_sinks(member_sinks, dest);
    };
    virtual ~deserialize_struct() = default;
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

template <typename deserialize_item, typename dest_iterator_t, typename element_type = typename dest_iterator_t::value_type>
class deserialize_iterator: public SinkTree
{
public:
    typedef typename deserialize_item::record_t record_t;
    static constexpr ID id = deserialize_item::id;
    typedef typename deserialize_item::dest_t dest_t;
    typedef iterator_helper<dest_t, element_type> helper;
public:
    deserialize_iterator(dest_iterator_t dest):
        dest(dest)
    {

    };
    virtual ~deserialize_iterator() = default;
private:
    dest_iterator_t dest;
    element_type curr_el;
private:
    inline void submit_item()
    {
        *dest++ = std::move(curr_el);
    };
protected:
    virtual bool _start_container(ContainerHandle cont, const ContainerMeta *meta)
    {
        curr_el = helper::construct();
        dest_t *dest;
        helper::assign(&curr_el, &dest);
        nest(StreamSink(new deserialize_item(*dest)));
        return true;
    };

    virtual bool _push_node(NodeHandle node)
    {
        curr_el = helper::construct();
        dest_t *dest;
        helper::assign(&curr_el, &dest);
        deserialize_item(*dest).push_node(node);
        submit_item();
        return true;
    };

    virtual bool _end_container(const ContainerFooter *foot)
    {
        submit_item();
        return true;
    };

    virtual void _end_of_stream()
    {

    };
};

template <typename deserialize_item,
          typename output_iterator>
struct deserialize_container: deserialize_iterator<
    deserialize_item,
    output_iterator,
    typename output_iterator::container_type::value_type
    >
{
private:
    typedef typename output_iterator::container_type::value_type element_t;
public:
    typedef typename output_iterator::container_type dest_t;
    typedef typename deserialize_item::record_t record_t;
    static constexpr ID id = deserialize_item::id;
public:
    deserialize_container(dest_t &dest):
        deserialize_iterator<deserialize_item, output_iterator, element_t>::deserialize_iterator(output_iterator(dest))
    {

    }
    virtual ~deserialize_container() = default;
};

template <typename record_t, ID id, typename value_t>
struct deserialize_value
{
    typedef deserialize_struct<
        record_t,
        id,
        struct_members<
            deserialize_member_raw<record_t, id, value_t, value_t, 0>
            >
        > deserializer;
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

template <typename deserializer>
struct deserialize_only: public SinkTree
{
    typedef typename deserializer::dest_t dest_t;
    typedef typename deserializer::record_t record_t;
    static constexpr ID id = deserializer::id;
public:
    deserialize_only(dest_t &dest):
        _deserializer(new deserializer(dest))
    {

    };
    virtual ~deserialize_only() = default;
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

}

#endif
