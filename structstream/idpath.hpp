/**********************************************************************
File name: idpath.hpp
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
#ifndef _STRUCTSTREAM_IDPATH_H
#define _STRUCTSTREAM_IDPATH_H

#include "structstream/node_container.hpp"

namespace StructStream {

class NodeTreeIterator {
public:
    typedef std::forward_iterator_tag category;
    typedef NodeHandle value_type;
    typedef NodeHandle* pointer;
    typedef NodeHandle& reference;
private:
    ContainerHandle _cont;
    NodeVector::iterator _cont_iter;
    NodeTreeIterator *_nested_iter;
public:
    NodeTreeIterator();
    explicit NodeTreeIterator(ContainerHandle cont);
    virtual ~NodeTreeIterator();
    NodeTreeIterator &operator++();
    bool operator==(const NodeTreeIterator &other) const;
    void skip();
    void kill();
public:
    inline bool valid() const {
        return _cont.get() != nullptr;
    };
    inline bool operator!=(const NodeTreeIterator &other) const {
        return !this->operator==(other);
    };
    inline NodeHandle const& operator*() const {
        if (_nested_iter) {
            return **_nested_iter;
        } else {
            return *_cont_iter;
        }
    };
};

typedef std::function< bool (const Node &node) > NodeFilter;

class FindMostShallow {
public:
    typedef std::forward_iterator_tag category;
    typedef NodeHandle value_type;
    typedef NodeHandle* pointer;
    typedef NodeHandle& reference;
private:
    NodeTreeIterator _iter;
    NodeFilter _filter;
protected:
    void advance();
public:
    FindMostShallow();
    FindMostShallow(ContainerHandle cont, NodeFilter filter);
    FindMostShallow &operator++();
    virtual ~FindMostShallow();
    bool operator==(const FindMostShallow &other) const;
public:
    inline bool operator!=(const FindMostShallow &other) const {
        return !this->operator==(other);
    };
    inline NodeHandle const& operator*() const {
        return *_iter;
    };
    inline bool valid() const {
        return _iter.valid();
    };
};

struct FindByID {
    FindByID(const ID id);

    bool operator() (const Node &node) const;
private:
    const ID _id;
};

}

#endif
