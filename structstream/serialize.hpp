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
#ifndef _STRUCTSTREAM_SERIALIZE_H
#define _STRUCTSTREAM_SERIALIZE_H

#include "structstream/errors.hpp"
#include "structstream/serialize_base.hpp"
#include "structstream/serialize_utils.hpp"
#include "structstream/serialize_struct.hpp"
#include "structstream/serialize_iterables.hpp"

namespace StructStream {

/* simple serializers */

template <typename _record_t, typename _selector_t, typename _dest_t>
struct value_decl
{
    typedef _record_t record_t;
    typedef _selector_t selector_t;
    typedef _dest_t dest_t;

    struct deserializer: public deserializer_base
    {
        deserializer(dest_t &dest):
            _dest(dest)
        {

        }

    private:
        dest_t &_dest;

    public:
        bool node(const NodeHandle &node) override
        {
            record_t *rec = static_cast<record_t*>(node.get());
            value_helper<record_t, dest_t>::from_record(rec, _dest);
            return true;
        }

    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t src, const StreamSink &sink)
        {
            sink->push_node(
                value_helper<record_t, dest_t>::to_record(
                    src, selector_t::first));
        }
    };
};

/* top level serializers */

template <typename object_decl, bool require=false, bool first=false>
struct only
{
    typedef typename object_decl::dest_t dest_t;
    typedef typename object_decl::selector_t object_selector_t;
    typedef typename object_decl::record_t object_record_t;

    struct deserializer: public deserializer_nesting
    {
        typedef dest_t& arg_t;

        deserializer(arg_t dest):
            _dest(dest),
            _found(false),
            _thrown(false)
        {

        };

    private:
        dest_t &_dest;
        bool _found;
        bool _thrown;

        deserializer_base *match_node(const NodeHandle &node)
        {
            if (object_selector_t::test(node->id())) {
                object_record_t *rec = dynamic_cast<object_record_t*>(
                    node.get());
                if (rec) {
                    return new typename object_decl::deserializer(_dest);
                }
            }
            return nullptr;
        }

        deserializer_base *match_cont(const ContainerHandle &cont)
        {
            deserializer_base *result = match_node(cont);
            if (result) {
                nest(result);
            }
            return result;
        }

        void throw_record_not_found()
        {
            if (_thrown) {
                return;
            }
            _thrown = true;
            throw RecordNotFound("Record required to be first by only<>"
                                 " was not found.");
        }

        bool handle_unknown_node(const NodeHandle &node)
        {
            if (require && first) {
                throw_record_not_found();
            }
            return true;
        }

        bool handle_unknown_cont(const ContainerHandle &cont)
        {
            bool continue_streaming = handle_unknown_node(cont);
            if (continue_streaming) {
                nest(new deserializer_null());
            }
            return continue_streaming;
        }

    protected:
        bool _end_container() override
        {
            return !_found;
        }

        bool _node(const NodeHandle &node) override
        {
            deserializer_base *handler = match_node(node);
            if (!handler) {
                return handle_unknown_node(node);
            } else {
                _found = true;
                handler->node(node);
                delete handler;
                return false;
            }
        };

        bool _start_container(const ContainerHandle &cont) override
        {
            deserializer_base *handler = match_node(cont);
            if (!handler) {
                return handle_unknown_cont(cont);
            } else {
                _found = true;
                nest(handler);
            }
            return true;
        }

    public:
        void finalize() override
        {
            try {
                deserializer_nesting::finalize();
            } catch (...) {
                if (!_found) {
                    throw_record_not_found();
                }
                throw;
            }
            if (!_found) {
                throw_record_not_found();
            }
        }
    };

    typedef typename object_decl::serializer serializer;
};

template <typename object_decl,
          typename output_iterator,
          typename input_iterator = typename output_iterator::container_type::const_iterator,
          typename element_type = typename output_iterator::container_type::value_type>
struct iterator
{
    struct deserializer: public deserializer_sequence<object_decl,
                                                      element_type>
    {
        typedef output_iterator arg_t;

        deserializer(output_iterator dest):
            _dest(dest)
        {

        }

    private:
        output_iterator _dest;

    protected:
        void _submit_item(element_type &&item) override
        {
            *_dest = std::move(item);
            ++_dest;
        }

    };

    struct serializer
    {
        static inline void to_sink(
            input_iterator curr,
            input_iterator end,
            const StreamSink &sink)
        {
            for (; curr != end; ++curr) {
                object_decl::serializer::to_sink(
                    iterator_helper<typename object_decl::dest_t,
                                    element_type>::get_raw_reference(
                                        &*curr));
            }
        }
    };
};

/* stream sink adapter */

class DeserializerSink: public StreamSinkIntf
{
public:
    DeserializerSink(deserializer_base *child);
    virtual ~DeserializerSink();

private:
    deserializer_base *_child;

private:
    void _finalize_child();

public:
    bool end_container(const ContainerFooter *foot) override;
    void end_of_stream() override;
    bool push_node(NodeHandle node) override;
    bool start_container(ContainerHandle cont,
                         const ContainerMeta *meta) override;
};

/* utility functions */

template <typename serializer_t>
struct use_serializer
{
    template <typename... arg_ts>
    static inline void serialize_to_sink(const StreamSink &sink, arg_ts... args)
    {
        serializer_t::serializer::to_sink(args..., sink);
    }

    template <typename... arg_ts>
    static inline typename serializer_t::deserializer *deserializer_obj(
        arg_ts... args)
    {
        return new typename serializer_t::deserializer(args...);
    }

    template <typename... arg_ts>
    static inline StreamSink deserialize(arg_ts... args)
    {
        return StreamSink(new DeserializerSink(
            deserializer_obj(args...)));
    }
};

template <typename serializer_t>
inline void serialize_to_sink(
    typename serializer_t::serializer::arg_t src,
    const StreamSink &sink)
{
    use_serializer<serializer_t>::serialize_to_sink(sink, src);
}

template <typename serializer_t>
inline typename serializer_t::deserializer *deserializer_obj(
    typename serializer_t::deserializer::arg_t dest)
{
    return new typename serializer_t::deserializer(dest);
}

template <typename serializer_t>
inline StreamSink deserialize(
    typename serializer_t::deserializer::arg_t dest)
{
    return StreamSink(new DeserializerSink(
        deserializer_obj<serializer_t>(dest)));
}

}

#endif
