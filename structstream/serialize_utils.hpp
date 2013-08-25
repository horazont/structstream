/**********************************************************************
File name: serialize_utils.hpp
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
#ifndef _STRUCTSTREAM_SERIALIZE_UTILS_H
#define _STRUCTSTREAM_SERIALIZE_UTILS_H

#include "structstream/static.hpp"
#include "structstream/node_factory.hpp"
#include "structstream/node_blob.hpp"
#include "structstream/streaming_base.hpp"

namespace StructStream {

/* struct type traits and derivates */

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

/* id selection */

template <ID... matches>
struct id_selector;

template <ID match, ID... matches>
struct id_selector<match, matches...>
{
    static constexpr ID first = match;

    static inline bool test(const ID to_test)
    {
        return (to_test == match)
               || id_selector<matches...>::test(to_test);
    };
};

template <ID match>
struct id_selector<match>
{
    static constexpr ID first = match;

    static inline bool test(const ID to_test)
    {
        return to_test == match;
    };
};

template <>
struct id_selector<>
{
    static inline bool test(const ID to_test)
    {
        return false;
    };
};

/* value helpers */

template <typename record_t, typename value_t>
struct value_helper
{
    static inline void from_record(record_t *src, value_t &dest)
    {
        dest = src->get();
    };

    static inline std::shared_ptr<record_t> to_record(
        const value_t &src,
        const ID record_id)
    {
        std::shared_ptr<record_t> result =
            NodeHandleFactory<record_t>::create(record_id);
        result->set(src);
        return result;
    };
};

template <>
struct value_helper<UTF8Record, std::string>
{
    static inline void from_record(UTF8Record *src, std::string &dest)
    {
        dest = src->datastr();
    };

    static inline std::shared_ptr<UTF8Record> to_record(
        const std::string &src,
        const ID record_id)
    {
        std::shared_ptr<UTF8Record> result =
            NodeHandleFactory<UTF8Record>::create(record_id);
        result->set(src);
        return result;
    };
};

}

#endif
