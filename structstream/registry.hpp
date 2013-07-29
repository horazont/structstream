/**********************************************************************
File name: registry.hpp
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
#ifndef _STRUCTSTREAM_REGISTRY_H
#define _STRUCTSTREAM_REGISTRY_H

#include <unordered_map>
#include <memory>

#include "structstream/node_base.hpp"

namespace StructStream {

typedef std::function< NodeHandle(ID) > NodeConstructor;

/**
 * Manage association of RecordType:s with classes representing them.
 *
 * You need an instance of this class to drive a Reader class. A new
 * instance of this class already has the default types registered, so
 * you don't need to do anything if you don't implement custom types.
 */
class Registry {
public:
    Registry();
    Registry(const Registry &ref);
    Registry(
        const std::initializer_list<std::pair<RecordType, NodeConstructor>> &initial,
        bool add_defaults = true);
    virtual ~Registry();
private:
    std::unordered_map<RecordType, NodeConstructor> _record_types;
private:
    void register_defaults();
public:
    /**
     * Create a node for the given record type with the given id.
     *
     * Return the newly created node or an empty handle, if the given
     * record type is not known to the registry.
     *
     * @param rt Record type for which a node is wanted.
     * @param id ID of the node
     *
     * @result Newly created node or empty handle.
     */
    NodeHandle node_from_record_type(RecordType rt, ID id) const;

    void register_record_type(RecordType rt, NodeConstructor constructor);

    template <class record_type>
    void register_record_class(RecordType rt)
    {
        register_record_type(rt, [](ID id){ return NodeHandleFactory<record_type>::create(id); });
    }
};

typedef std::shared_ptr<Registry> RegistryHandle;

}

#endif
