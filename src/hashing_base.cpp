/**********************************************************************
File name: hashing_base.cpp
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
#include <structstream/hashing_base.hpp>

namespace StructStream {

/* StructStream::IncrementalHash */

IncrementalHash::IncrementalHash()
{

}

IncrementalHash::~IncrementalHash()
{

}

IncrementalHash::IncrementalHash(const IncrementalHash &ref)
{

}

IncrementalHash &IncrementalHash::operator= (const IncrementalHash &ref)
{
    return *this;
}

/* StructStream::HashRegistry */

HashRegistry::HashRegistry()
{

}

HashRegistry::HashRegistry(const HashRegistry &ref):
    _map(ref._map)
{

}

HashRegistry::~HashRegistry()
{

}

HashRegistry& HashRegistry::operator=(const HashRegistry &ref)
{
    _map = ref._map;
    return *this;
}

IncrementalHash *HashRegistry::get_hash(HashType type) const
{
    auto found = _map.find(type);
    if (found == _map.end()) {
        return nullptr;
    }

    return (found->second)();
}

void HashRegistry::register_hash(HashType type, HashConstructor con)
{
    _map[type] = con;
}

HashRegistry hashes;

}
