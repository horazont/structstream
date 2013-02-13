/**********************************************************************
File name: strstr_registry.hpp
This file is part of: ebml++

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

For feedback and questions about ebml++ please e-mail one of the authors
named in the AUTHORS file.
**********************************************************************/
#ifndef _STRSTR_REGISTRY_H
#define _STRSTR_REGISTRY_H

#include <unordered_map>
#include <memory>

#include "include/strstr_nodes.hpp"

namespace std {

template<>
struct hash<StructStream::RecordType> {
public:
    size_t operator()(const StructStream::RecordType &id) const {
        return hash<unsigned long>()(id);
    };
};

}

namespace StructStream {

typedef std::function< NodeHandle(ID) > NodeConstructor;

class Registry {
public:
    Registry();
    Registry(const Registry &ref);
    virtual ~Registry();
private:
    std::unordered_map<RecordType, NodeConstructor> _record_types;
private:
    void register_defaults();
public:
    NodeHandle node_from_record_type(RecordType rt, ID id) const;
};

typedef std::shared_ptr<Registry> RegistryHandle;

}

#endif
