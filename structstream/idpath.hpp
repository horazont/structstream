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

class idpath_find_most_shallow {
public:
    typedef std::forward_iterator_tag category;
    typedef NodeHandle value_type;
    typedef NodeHandle* pointer;
    typedef NodeHandle& reference;
private:
    inline void advance_to_next_matching_child() {
        while (true) {

            // printf("idpath 0x%lx: adv: loop\n", (intptr_t)this);

            if (_child_iter == _container->children_end()) {

                // printf("idpath 0x%lx: adv: eoc\n", (intptr_t)this);

                _container = ContainerHandle();
                return;
            }

            NodeHandle child = *_child_iter;

            if (child->id() == _target_id) {

                // printf("idpath 0x%lx: adv: found match: 0x%lx\n", (intptr_t)this, (intptr_t)(child.get()));

                return;
            }

            ContainerHandle cont = std::dynamic_pointer_cast<Container>(child);
            if (cont) {
                // printf("idpath 0x%lx: adv: found container: 0x%lx\n", (intptr_t)this, (intptr_t)(cont.get()));
                // printf("idpath 0x%lx: adv: forking ... \n", (intptr_t)this);

                _nested_iter = new idpath_find_most_shallow(_target_id, cont);
                if (_nested_iter->valid()) {

                    // printf("idpath 0x%lx: adv: forked 0x%lx, which was valid\n", (intptr_t)this, (intptr_t)(_nested_iter));

                    return;
                } else {

                    // printf("idpath 0x%lx: adv: forked 0x%lx, which was dead\n", (intptr_t)this, (intptr_t)(_nested_iter));

                    delete _nested_iter;
                    _nested_iter = nullptr;
                }
            }

            _child_iter++;
        }
    };
public:
    inline idpath_find_most_shallow(ID target_id, ContainerHandle container):
        _target_id(target_id),
        _container(container),
        _child_iter(container->children_begin()),
        _nested_iter()
    {
        advance_to_next_matching_child();
    };

    ~idpath_find_most_shallow()
    {
        if (_nested_iter) {
            delete _nested_iter;
            _nested_iter = nullptr;
        }
    };

    inline bool valid() const {
        return _container.get() != nullptr;
    };

    inline idpath_find_most_shallow& operator++() {
        if (_container.get() == nullptr) {
            // printf("idpath 0x%lx: !_container\n", (intptr_t)this);
            return *this;
        }

        if (_nested_iter) {
            // printf("idpath 0x%lx: _nested_iter available: 0x%lx\n", (intptr_t)this, (intptr_t)_nested_iter);
            ++(*_nested_iter);
            /* check if end was reached */
            if (_nested_iter->valid()) {
                // printf("idpath 0x%lx: nested is valid\n", (intptr_t)this);
                return *this;
            }

            // printf("idpath 0x%lx: nested is not valid\n", (intptr_t)this);
            delete _nested_iter;
            _nested_iter = nullptr;
        }

        // printf("idpath 0x%lx: next child\n", (intptr_t)this);
        _child_iter++;
        // printf("idpath 0x%lx: advance\n", (intptr_t)this);
        advance_to_next_matching_child();
        // printf("idpath 0x%lx: ret\n", (intptr_t)this);
        return *this;
    };

    inline bool operator==(const idpath_find_most_shallow& other) const {
        return ((_target_id == other._target_id) || (other._container.get() == nullptr))
            && (_container == other._container)
            && (_child_iter == other._child_iter);
    };

    inline bool operator!=(const idpath_find_most_shallow& other) const {
        return !this->operator==(other);
    };

    inline NodeHandle const& operator*() const {
        if (_nested_iter) {
            return **_nested_iter;
        } else {
            return *_child_iter;
        }
    };
private:
    const ID _target_id;
    ContainerHandle _container;
    NodeVector::iterator _child_iter;
    idpath_find_most_shallow *_nested_iter;
};

}

#endif
