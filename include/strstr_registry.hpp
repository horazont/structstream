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
