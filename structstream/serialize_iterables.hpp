#ifndef _STRUCTSTREAM_SERIALIZE_ITERABLES_H
#define _STRUCTSTREAM_SERIALIZE_ITERABLES_H

#include "structstream/serialize_base.hpp"
#include "structstream/node_base.hpp"

namespace StructStream {

/* helpers */

template <typename raw_value_type, typename container_value_type>
struct iterator_helper;

template <typename T>
struct iterator_helper<T, T>
{
    static inline void get_ptr(T *src, T **dest)
    {
        *dest = src;
    };

    static inline T &get_raw_reference(T *src)
    {
        return *src;
    };

    template <typename... arg_ts>
    static inline T construct(arg_ts... args)
    {
        return T(args...);
    }
};

template <typename T>
struct iterator_helper<T, T*>
{
    static inline void get_ptr(T **src, T **dest)
    {
        *dest = *src;
    };

    static inline T &get_raw_reference(T **src)
    {
        return **src;
    };

    template <typename... arg_ts>
    static inline T* construct(arg_ts... args)
    {
        return new T(args...);
    }
};

template <typename T>
struct iterator_helper<T, std::shared_ptr<T>>
{
    static inline void get_ptr(std::shared_ptr<T> *src, T **dest)
    {
        *dest = src->get();
    };

    static inline T &get_raw_reference(std::shared_ptr<T> *src)
    {
        return *src->get();
    };

    template <typename... arg_ts>
    static inline std::shared_ptr<T> construct(arg_ts... args)
    {
        return std::shared_ptr<T>(new T(args...));
    }
};

template <typename T>
struct iterator_helper<const T, const std::shared_ptr<T>>
{
    static inline void get_ptr(const std::shared_ptr<T> *src, const T **dest)
    {
        *dest = src->get();
    };

    static inline const T &get_raw_reference(const std::shared_ptr<T> *src)
    {
        return *src->get();
    };

};

template <typename T>
struct iterator_helper<T, std::unique_ptr<T>>
{
    static inline void get_ptr(std::unique_ptr<T> *src, T **dest)
    {
        *dest = src->get();
    };

    static inline T &get_raw_reference(std::unique_ptr<T> *src)
    {
        return *src->get();
    };

    template <typename... arg_ts>
    static inline std::unique_ptr<T> construct(arg_ts... args)
    {
        return std::unique_ptr<T>(new T(args...));
    }
};

template <typename T>
struct iterator_helper<const T, const std::unique_ptr<T>>
{
    static inline void get_ptr(const std::unique_ptr<T> *src, const T **dest)
    {
        *dest = src->get();
    };

    static inline const T &get_raw_reference(const std::unique_ptr<T> *src)
    {
        return *src->get();
    };

    template <typename... arg_ts>
    static inline const std::unique_ptr<T> construct(arg_ts... args)
    {
        return std::unique_ptr<T>(new T(args...));
    }
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

template <typename item_decl, typename element_t>
struct deserializer_sequence: public deserializer_nesting
{
    typedef typename item_decl::dest_t item_t;
    typedef iterator_helper<item_t, element_t> helper;

private:
    typedef typename item_decl::selector_t item_selector_t;
    typedef typename item_decl::record_t item_record_t;

    element_t _buf;

    deserializer_base *match_node(const NodeHandle &node)
    {
        if (item_selector_t::test(node->id())) {
            item_record_t *rec = dynamic_cast<item_record_t*>(
                node.get());
            if (rec) {
                _buf = helper::construct();
                item_t *item_ptr;
                helper::get_ptr(&_buf, &item_ptr);
                return new typename item_decl::deserializer(*item_ptr);
            }
        }
        return nullptr;
    };

    deserializer_base *match_cont(const ContainerHandle &cont)
    {
        deserializer_base *result = match_node(cont);
        if (result) {
            nest(result);
        }
        return result;
    };

    bool handle_unknown_node(const NodeHandle &node)
    {
        return true;
    };

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
        _submit_item(std::move(_buf));
        return true;
    };

    bool _node(const NodeHandle &node) override
    {
        deserializer_base *handler = match_node(node);
        if (handler == nullptr) {
            return handle_unknown_node(node);
        } else {
            handler->node(node);
            delete handler;
            _submit_item(std::move(_buf));
        }
        return true;
    };

    bool _start_container(const ContainerHandle &cont) override
    {
        deserializer_base *handler = match_cont(cont);
        if (handler == nullptr) {
            return handle_unknown_cont(cont);
        } else {
            nest(handler);
        }
        return true;
    };

    virtual void _submit_item(element_t &&item) = 0;
};


/* iterables */

template <typename item_decl, typename _selector_t,
          typename input_iterator_t, typename output_iterator_t,
          typename _record_t = Container>
struct iterable
{
    static_assert(std::is_base_of<Container, _record_t>::value,
                  "iterable serializer requires Container subclass as"
                  " record type!");

    typedef _record_t record_t;
    typedef _selector_t selector_t;

private:
    typedef typename item_decl::selector_t item_selector_t;
    typedef typename item_decl::record_t item_record_t;
    typedef deserializer_sequence<
        item_decl,
        typename output_iterator_t::container_type::value_type> deserializer_baseclass;

public:
    struct deserializer: public deserializer_baseclass
    {
        deserializer(output_iterator_t dest):
            _dest(dest)
        {

        };

    private:
        output_iterator_t _dest;

    protected:
        void _submit_item(typename output_iterator_t::container_type::value_type &&item)
        {
            *_dest = std::move(item);
            ++_dest;
        }

    };

    struct serializer
    {
        typedef input_iterator_t arg_t;

        static inline void to_sink(
            input_iterator_t curr,
            input_iterator_t end,
            const StreamSink &sink)
        {
            typedef iterator_helper<
                const typename item_decl::dest_t,
                const typename output_iterator_t::container_type::value_type> helper;

            ContainerHandle parent =
                NodeHandleFactory<Container>::create(selector_t::first);
            ContainerMeta meta;
            sink->start_container(parent, &meta);

            for (; curr != end; ++curr) {
                item_decl::serializer::to_sink(
                    helper::get_raw_reference(&*curr),
                    sink);
            }

            ContainerFooter foot;
            sink->end_container(&foot);
        }
    };
};

/* containers */

template <typename item_decl,
          typename _selector_t,
          typename output_iterator,
          typename input_iterator = typename output_iterator::container_type::const_iterator,
          typename iterator_initializer_t = iterator_initializer<output_iterator>,
          typename _record_t = Container>
struct container
{
    static_assert(std::is_base_of<Container, _record_t>::value,
                  "container serializer requires Container subclass as"
                  " record type!");

    typedef _selector_t selector_t;
    typedef _record_t record_t;
    typedef typename output_iterator::container_type dest_t;

private:
    typedef iterable<item_decl, _selector_t,
                     input_iterator, output_iterator,
                     _record_t> iterable_base;

public:
    struct deserializer: public iterable_base::deserializer
    {
        typedef dest_t& arg_t;

        deserializer(dest_t &dest):
            iterable_base::deserializer(
                iterator_initializer<output_iterator>::init(dest))
        {

        };
    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t src, const StreamSink sink)
        {
            iterable_base::serializer::to_sink(
                src.cbegin(), src.cend(), sink);
        }
    };

};

/* static array */

template <typename item_decl,
          typename _selector_t,
          unsigned int item_count,
          typename _dest_t = typename item_decl::dest_t,
          bool require_all = true,
          bool fail_on_overflow = true,
          typename _record_t = Container>
struct static_array
{
    static_assert(std::is_base_of<Container, _record_t>::value,
                  "static_array serializer requires Container subclass "
                  "as record type!");

    typedef _selector_t selector_t;
    typedef _record_t record_t;
    typedef _dest_t dest_t[item_count];

    struct deserializer: public deserializer_sequence<item_decl, _dest_t>
    {
        typedef dest_t& arg_t;

        deserializer(arg_t dest):
            _dest(dest),
            _offs(0)
        {

        }

    private:
        dest_t &_dest;
        unsigned int _offs;

    protected:
        void _submit_item(_dest_t &&item) override
        {
            if (_offs >= item_count) {
                if (fail_on_overflow) {
                    throw UnexpectedRecord("static_array overflew.");
                } else {
                    return;
                }
            }
            _dest[_offs] = std::move(item);
            ++_offs;
        }

    public:
        void finalize() override
        {
            if (require_all) {
                if (_offs != item_count) {
                    throw RecordNotFound("static_array did not find all"
                                         " children.");
                }
            }
        }

    };

    struct serializer
    {
        typedef const dest_t& arg_t;

        static inline void to_sink(arg_t src, const StreamSink &sink)
        {
            ContainerHandle parent =
                NodeHandleFactory<record_t>::create(selector_t::first);
            ContainerMeta meta;

            sink->start_container(parent, &meta);
            for (unsigned int i = 0; i < item_count; i++) {
                item_decl::serializer::to_sink(src[i], sink);
            }

            ContainerFooter foot;
            sink->end_container(&foot);
        }
    };
};

}

#endif
