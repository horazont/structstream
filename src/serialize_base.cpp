/**********************************************************************
File name: serialize_base.cpp
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
#include "structstream/serialize_base.hpp"

#include <cassert>

namespace StructStream {

/* StructStream::deserializer_base */

deserializer_base::~deserializer_base()
{

}

bool deserializer_base::end_container()
{
    assert(false);
    return false;
}

void deserializer_base::finalize()
{

}

bool deserializer_base::node(const NodeHandle &node)
{
    assert(false);
    return false;
}

bool deserializer_base::start_container(const ContainerHandle &cont)
{
    assert(false);
    return false;
}

/* StructStream::deserializer_nesting */

deserializer_nesting::deserializer_nesting():
    _depth(0),
    _nested(nullptr)
{

}

deserializer_nesting::~deserializer_nesting()
{
    if (_nested) {
        unnest();
    }
}

void deserializer_nesting::unnest()
{
    assert(_nested);
    _nested->finalize();
    delete _nested;
    _nested = nullptr;
    _depth = 0;
}

void deserializer_nesting::nest(deserializer_base *obj)
{
    assert(!_nested);
    _nested = obj;
    _depth = 1;
}

bool deserializer_nesting::_end_container()
{
    assert(false);
    return false;
}

bool deserializer_nesting::_node(const NodeHandle &node)
{
    assert(false);
    return false;
}

bool deserializer_nesting::_start_container(const ContainerHandle &cont)
{
    assert(false);
    return false;
}

bool deserializer_nesting::end_container()
{
    if (_nested) {
        _depth -= 1;
        if (_depth == 0) {
            unnest();
            return _end_container();
        } else {
            return _nested->end_container();
        }
    } else {
        return _end_container();
    }
}

void deserializer_nesting::finalize()
{
    if (_nested) {
        unnest();
    }
}

bool deserializer_nesting::node(const NodeHandle &node)
{
    if (_nested) {
        return _nested->node(node);
    } else {
        return _node(node);
    }
}

bool deserializer_nesting::start_container(const ContainerHandle &cont)
{
    if (_nested) {
        _depth += 1;
        return _nested->start_container(cont);
    } else {
        return _start_container(cont);
    }
}

/* StructStream::deserializer_null */

bool deserializer_null::end_container()
{
    return true;
}

bool deserializer_null::node(const NodeHandle &node)
{
    return true;
}

bool deserializer_null::start_container(const ContainerHandle &cont)
{
    return true;
}

}
