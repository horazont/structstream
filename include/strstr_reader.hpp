#ifndef _STRSTR_READER_H
#define _STRSTR_READER_H

#include "include/strstr_registry.hpp"
#include "include/strstr_io.hpp"

namespace StructStream {

class Reader {
public:
    Reader(IOIntf *source, const RegistryHandle node_types);
    Reader(const Reader &ref);
    virtual ~Reader();
private:
    IOIntf _source;
    const Registry *_node_factory;
    const RegistryHandle _node_factory_handle;
public:

};

}

#endif
