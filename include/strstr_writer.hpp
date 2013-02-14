#ifndef _STRSTR_WRITER_H
#define _STRSTR_WRITER_H

#include <unordered_map>

#include "include/strstr_io.hpp"
#include "include/strstr_nodes.hpp"

namespace StructStream {

class Writer
{
public:
    struct ContainerInfo
    {
        const Container *node;
        HashType hashfunc;
        bool armored;
    };
public:
    Writer();
    virtual ~Writer();
private:
    IOIntf *_dest;
    IOIntfHandle _dest_h;

    std::unordered_map<ID, HashType> _hashed_containers;
    HashType _default_hash;
    bool _armor_containers;
protected:
    HashType get_container_hash(const Container* node) const;
    bool get_container_armored(const Container* node) const;
    void require_open() const;
protected:
    virtual ContainerInfo *new_container_info() const;
    virtual void write_header();
    virtual void write_container(const Container *node);
    virtual void setup_container_header(ContainerInfo *info);
    virtual void write_container_header(ContainerInfo *info);
    virtual void write_container_footer(ContainerInfo *info);
    virtual void write_footer();
public:
    void close();
    inline bool is_open() const { return _dest != nullptr; };
    void open(IOIntfHandle stream);
    void write_subtree(const NodeHandle subtree);
};

}

#endif
