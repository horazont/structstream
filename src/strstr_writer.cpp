#include "include/strstr_writer.hpp"

#include <memory>

#include <cassert>

#include "include/strstr_static.hpp"
#include "include/strstr_errors.hpp"

namespace StructStream {

/* StructStream::Writer */
Writer::Writer():
    _dest(),
    _dest_h(),
    _hashed_containers(),
    _default_hash(HT_INVALID),
    _armor_containers(false)
{

}

Writer::~Writer()
{

}

HashType Writer::get_container_hash(const Container* node) const
{
    auto it = _hashed_containers.find(node->id());
    if (it == _hashed_containers.cend()) {
        return _default_hash;
    }
    return (*it).second;
}

bool Writer::get_container_armored(const Container *node) const
{
    return _armor_containers;
}

void Writer::require_open() const
{
    if (!_dest) {
        throw AlreadyClosed("This operation is not allowed on a closed Writer.");
    }
}

Writer::ContainerInfo *Writer::new_container_info() const
{
    return new Writer::ContainerInfo();
}

void Writer::write_header()
{
    // no header here
}

void Writer::write_container(const Container *node)
{
    ContainerInfo *info = new_container_info();
    std::unique_ptr<ContainerInfo> info_h{info};
    info->node = node;

    setup_container_header(info);

    write_container_header(info);

    for (auto it = node->children_cbegin();
         it != node->children_cend();
         it++)
    {
        write_subtree(*it);
    }

    write_container_footer(info);
}

void Writer::setup_container_header(Writer::ContainerInfo *info)
{
    info->armored = get_container_armored(info->node);
    info->hashfunc = get_container_hash(info->node);

    if (info->hashfunc != HT_INVALID) {
        throw UnsupportedHashFunction("Unsupported hash function.");
    }
}

void Writer::write_container_header(Writer::ContainerInfo *info)
{
    VarUInt flags = CF_WITH_SIZE;

    if (info->armored) {
        flags |= CF_ARMORED;
    }

    if (info->hashfunc != HT_INVALID) {
        flags |= CF_HASHED;
    }

    Utils::write_varuint(_dest, flags);
    intptr_t actual_child_count = info->node->child_count();
    assert(actual_child_count > 0);
    Utils::write_varuint(_dest, static_cast<VarUInt>(actual_child_count));
    if (info->hashfunc != HT_INVALID) {
        Utils::write_varuint(_dest, static_cast<VarUInt>(info->hashfunc));
    }
}

void Writer::write_container_footer(Writer::ContainerInfo *info)
{
    // TODO: hash result
}

void Writer::write_footer()
{
    Utils::write_record_type(_dest, RT_END_OF_CHILDREN);
}

void Writer::close()
{
    require_open();

    write_footer();
    _dest = nullptr;
    _dest_h = IOIntfHandle();
}

void Writer::open(IOIntfHandle stream)
{
    if (_dest) {
        throw AlreadyOpen("This operation is not allowed on open Writer:s.");
    }

    _dest_h = stream;
    _dest = _dest_h.get();

    write_header();
}

void Writer::write_subtree(NodeHandle subtree)
{
    Container *cont = dynamic_cast<Container*>(subtree.get());
    if (cont != nullptr) {
        // it's a container
        write_container(cont);
    } else {
        subtree->write(_dest);
    }
}

}
