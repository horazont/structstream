/**********************************************************************
File name: streaming_bitstream.cpp
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
#include "structstream/streaming_bitstream.hpp"

#include <cassert>
#include <cstring>

#include "structstream/utils.hpp"
#include "structstream/errors.hpp"
#include "structstream/node_container.hpp"

namespace StructStream {

/* StructStream::FromBitstream::ContainerMeta */

FromBitstream::ContainerMeta::ContainerMeta():
    ::StructStream::ContainerMeta(),
    has_hash(false)
{

}

FromBitstream::ContainerMeta::ContainerMeta(const FromBitstream::ContainerMeta &ref):
    ::StructStream::ContainerMeta(ref),
    has_hash(ref.has_hash)
{

}

ContainerMeta *FromBitstream::ContainerMeta::copy() const
{
    return new ContainerMeta(*this);
}

/* StructStream::FromBitstream */

FromBitstream::FromBitstream(IOIntfHandle source, const RegistryHandle nodetypes,
                   StreamSink sink):
    _original_source_h(source),
    _source_h(source),
    _source(source.get()),
    _node_factory_h(nodetypes),
    _node_factory(nodetypes.get()),
    _sink_h(sink),
    _sink(sink.get()),
    _parent_stack(),
    _curr_parent(),
    _forgiveness(0)
{
    push_root();
}

FromBitstream::~FromBitstream()
{

}

void FromBitstream::cleanup_state()
{
    // drop all parent info
    for (auto it: _parent_stack) {
        delete it;
    }
    _parent_stack.clear();
    _curr_parent = nullptr;
    _sink = nullptr;
    _sink_h = StreamSink();

    // kill all hash pipes
    while (_source_h != _original_source_h) {
        HashPipe<HP_READ> *pipe = static_cast<HashPipe<HP_READ>*>(_source);
        _source_h = pipe->underlying_io();
        _source = _source_h.get();
    }
}

void FromBitstream::check_end_of_container()
{
    if (!_curr_parent)
        return;

    // printf("bitstream: %d out of %d children found\n",
    //        _curr_parent->read_child_count,
    //        _curr_parent->meta->child_count);

    if (!_curr_parent->armored
        && _curr_parent->meta->child_count == _curr_parent->read_child_count)
    {
        end_of_container();
    }
}

void FromBitstream::check_hash_length(VarUInt len)
{
    if (len > 1024) {
        throw LimitError(std::string("Max hash length violated: ") + std::to_string(len));
    }
}

void FromBitstream::push_root()
{
    ParentInfo *root_pi = new ParentInfo();
    root_pi->meta->child_count = -1;
    root_pi->armored = true;
    root_pi->read_child_count = 0;
    root_pi->cont = ContainerHandle();
    _parent_stack.push_front(root_pi);
    _curr_parent = root_pi;
}

FromBitstream::ParentInfo *FromBitstream::new_parent_info() const
{
    return new ParentInfo();
}

void FromBitstream::start_of_container(ContainerHandle cont_h)
{
    VarUInt flags_int = Utils::read_varuint(_source);
    ParentInfo *info = new_parent_info();
    info->cont = cont_h;
    info->read_child_count = 0;

    try {
        proc_container_flags(flags_int, info);

        if (flags_int != 0) {
            if ((_forgiveness & UnknownContainerFlags) == 0) {
                throw UnsupportedContainerFlags("Unsupported container flags encountered.");
            }
        }

        end_of_container_header(info);
    } catch (...) {
        delete info;
        throw;
    }

    _parent_stack.push_front(info);
    _curr_parent = info;
    if (!_sink->start_container(info->cont, info->meta)) {
        throw SinkClosed();
    };

    // printf("bitstream: push 0x%lx\n", (uint64_t)_curr_parent->cont.get());
}

void FromBitstream::proc_container_flags(VarUInt &flags_int, FromBitstream::ParentInfo *info)
{
    info->meta->child_count = -1;
    info->footer->hash_function = HT_NONE;
    info->armored = false;

    if ((flags_int & CF_WITH_SIZE) != 0) {
        flags_int ^= CF_WITH_SIZE;
        info->meta->child_count = Utils::read_varuint(_source);
    }

    if ((flags_int & CF_ARMORED) != 0) {
        flags_int ^= CF_ARMORED;
        info->armored = true;
    }

    if (!info->armored && (info->meta->child_count == -1)) {
        throw IllegalCombinationOfFlags("Illegal combination of container flags: no CF_WITH_SIZE, but no CF_ARMORED either -- how am I supposed to find out the length?");
    }

    if ((flags_int & CF_HASHED) != 0) {
        HashType hash_function = static_cast<HashType>(Utils::read_varuint(_source));;

        flags_int ^= CF_HASHED;
        info->meta->has_hash = true;
        info->footer->hash_function = hash_function;
    }
}

void FromBitstream::end_of_container_header(ParentInfo *info)
{
    if (info->footer->hash_function != HT_NONE) {
        // printf("bitstream: container with hash %x\n", info->footer->hash_function);

        IncrementalHash *hashfun = hashes.get_hash(info->footer->hash_function);
        if ((hashfun == nullptr) && ((_forgiveness & UnknownHashFunction) == 0)) {
            throw UnsupportedHashFunction("Unsupported hash function.");
        }

        if (hashfun != nullptr) {
            info->pipe = new HashPipe<HP_READ>(hashfun, _source_h);
            info->pipe_h = IOIntfHandle(info->pipe);
            _source_h = info->pipe_h;
            _source = info->pipe;
        } else {
            // printf("bitstream: no hash function for %x\n", info->footer->hash_function);
        }
    }
}

void FromBitstream::end_of_container_body(ParentInfo *info)
{
    if (info->footer->hash_function != HT_NONE) {
        // printf("bitstream: eoc with hash %x\n", info->footer->hash_function);
        if (info->pipe) {
            _source_h = info->pipe->underlying_io();
            _source = _source_h.get();

            IncrementalHash *hashfun = info->pipe->reclaim_hash();

            info->pipe = nullptr;
            info->pipe_h = IOIntfHandle();

            // the pipe should delete itself right now

            VarUInt hash_length = Utils::read_varuint(_source);
            // checking this first allows us to safely downcast to
            // intptr_t
            check_hash_length(hash_length);
            if ((intptr_t)hash_length != hashfun->len()) {
                delete hashfun;
                throw IllegalData("hash length does not match with what we know about the hash function.");
            }

            uint8_t *hash_calculated = (uint8_t*)malloc(hash_length);
            hashfun->finish(hash_calculated);
            delete hashfun;

            uint8_t *hash_from_stream = (uint8_t*)malloc(hash_length);
            try {
                sread(_source, hash_from_stream, hash_length);

                if (memcmp(hash_from_stream, hash_calculated, hash_length) != 0) {
                    if ((_forgiveness & ChecksumErrors) == 0) {
                        throw HashCheckError("calculated and bitstream checksum do not match.");
                    }
                } else {
                    info->footer->validated = true;
                }
            } catch (...) {
                free(hash_from_stream);
                free(hash_calculated);
                throw;
            }
            free(hash_from_stream);
            free(hash_calculated);
        } else {

            // printf("bitstream: eoc with hash %x, but no checking\n", info->footer->hash_function);

            // hash checking has been disabled for this container for
            // some reason (e.g. forgiving mode)
            VarUInt hash_length = Utils::read_varuint(_source);
            check_hash_length(hash_length);
            sskip(_source, hash_length);
        }

    }
    info->cont->set_hashed(
        info->footer->validated,
        info->footer->hash_function
    );
}

void FromBitstream::end_of_container()
{
    ParentInfo *info = _curr_parent;
    _parent_stack.pop_front();
    if (_parent_stack.empty()) {
        _curr_parent = nullptr;
    } else {
        _curr_parent = _parent_stack.front();
    }

    try {
        if (_curr_parent) {
            // printf("bitstream: pop 0x%lx\n", (intptr_t)(info->cont.get()));

            // Do not call this virtual method for the root node
            end_of_container_body(info);
            if (!_sink->end_container(info->footer)) {
                throw SinkClosed();
            };

            _curr_parent->read_child_count += 1;
        } else {
            // printf("bitstream: end-of-stream reached\n");
            _sink->end_of_stream();
        }
    } catch (...) {
        delete info;
        throw;
    }
    delete info;

    check_end_of_container();
}

void FromBitstream::close() {
    cleanup_state();
    _source_h = nullptr;
    _source = nullptr;
}

NodeHandle FromBitstream::read_step() {
    if (_curr_parent == nullptr) {
        // printf("bitstream: state suggests end-of-stream, won't read further\n");
        return NodeHandle();
    }

    RecordType rt = Utils::read_record_type(_source);
    if (rt == RT_RESERVED) {
        throw UnsupportedRecordType("RT_RESERVED encountered. This stream may have been created with a newer version of structstream.");
    } else if (rt == RT_END_OF_CHILDREN) {
        // printf("bitstream: end of children encountered\n");

        if (_curr_parent->armored &&
            (_curr_parent->meta->child_count == -1
             || _curr_parent->meta->child_count == _curr_parent->read_child_count)
            )
        {
            // EOC is only valid if container has CF_ARMORED flag
            end_of_container();
        } else {
            if (_curr_parent->armored) {
                if ((_forgiveness & PrematureEndOfContainer) == 0) {
                    throw UnexpectedEndOfChildren("Armored container ended unexpectedly (not all announced children found).");
                } else {
                    end_of_container();
                }
            } else {
                throw UnexpectedEndOfChildren("Non-armored container closed by End-Of-Children tag. This may also imply that some children are missing.");
            }
        }
        return NodeHandle();
    }

    if (_curr_parent->armored
        && _curr_parent->meta->child_count != -1
        && _curr_parent->meta->child_count <= _curr_parent->read_child_count)
    {
        throw MissingEndOfChildren("CF_ARMORED | CF_WITH_SIZE container without EOC marker.");
    }


    ID id = Utils::read_id(_source);
    if (id == InvalidID) {
        throw InvalidIDError("Invalid object ID encountered.");
    }

    // printf("bitstream: found 0x%lx with id 0x%lx\n", rt, id);

    NodeHandle new_node = _node_factory->node_from_record_type(rt, id);
    if (!new_node.get()) {
        if ((rt >= RT_APPBLOB_MIN) && (rt <= RT_APPBLOB_MAX) &&
            ((_forgiveness & UnknownAppblobs) != 0))
        {
            // Appblobs are required to store their size in a varuint
            // right after the ID.

            // In this mode, we skip over the appblob as if it didn't
            // exist, except that we also increase the counter for the
            // surrounding container.

            VarUInt blob_size = Utils::read_varuint(_source);
            sskip(_source, blob_size);
            _curr_parent->read_child_count++;
            check_end_of_container();

            return read_step();
        } else {
            throw UnsupportedRecordType("Unsupported record type.");
        }
    }

    ContainerHandle new_parent = std::dynamic_pointer_cast<Container>(new_node);
    if (new_parent.get() != nullptr) {
        start_of_container(new_parent);
    } else {
        new_node->read(_source);
        if (!_sink->push_node(new_node)) {
            throw SinkClosed();
        };

        _curr_parent->read_child_count++;
    }

    check_end_of_container();

    return new_node;
}

void FromBitstream::read_next()
{
    NodeHandle node = read_step();
    if (!node) {
        return;
    }

    ContainerHandle cont = std::dynamic_pointer_cast<Container>(node);
    if (cont) {
        typename decltype(_parent_stack)::size_type this_len = _parent_stack.size();
        // printf("bitstream: read_next(): waiting for length %lu\n", this_len);
        while (_parent_stack.size() >= this_len) {
            read_step();
            // printf("bitstream: read_next(): current length %lu\n", _parent_stack.size());
        }
    }
}

void FromBitstream::read_all()
{
    try {
        NodeHandle node;
        do {
            node = read_step();
        } while (_curr_parent != nullptr);
    } catch (SinkClosed &foo) {
        cleanup_state();
        return;
    } catch (...) {
        cleanup_state();
        throw;
    }
}

void FromBitstream::set_forgiving_for(uint32_t forgiveness, bool forgiving)
{
    if (forgiving) {
        _forgiveness |= forgiveness;
    } else {
        _forgiveness &= ~forgiveness;
    }
}

/* StructStream::ToBitstream */

ToBitstream::ToBitstream(IOIntfHandle dest):
    _dest_h(dest),
    _dest(dest.get()),
    _parent_stack(),
    _curr_parent(),
    _default_armor(false)
{

}

ToBitstream::~ToBitstream()
{

}

void ToBitstream::require_open() const
{
    if (!_dest) {
        throw AlreadyClosed("This operation is not allowed on a closed writer.");
    }
}

ToBitstream::ParentInfo *ToBitstream::new_parent_info() const
{
    return new ParentInfo();
}

VarUInt ToBitstream::get_container_flags(ToBitstream::ParentInfo *info)
{
    VarUInt flags = 0;
    if (info->armored) {
        flags |= CF_ARMORED;
    }
    if (info->child_count >= 0) {
        flags |= CF_WITH_SIZE;
    }
    if (info->hash_function != HT_NONE) {
        flags |= CF_HASHED;
    }

    return flags;
}

void ToBitstream::setup_container(ToBitstream::ParentInfo *info, ContainerHandle cont, const ContainerMeta *meta)
{
    info->cont = cont;
    info->child_count = meta->child_count;
    info->armored = _default_armor || (meta->child_count < 0);
    info->hash_function = HT_NONE;
}

void ToBitstream::write_container_header(VarUInt flags, ParentInfo *info)
{
    Utils::write_record_type(_dest, info->cont->record_type());
    Utils::write_id(_dest, info->cont->id());

    Utils::write_varuint(_dest, flags);
    if ((flags & CF_WITH_SIZE) != 0) {
        assert(info->child_count >= 0);
        Utils::write_varuint(_dest, static_cast<VarUInt>(info->child_count));
    }

    if ((flags & CF_HASHED) != 0) {
        assert(info->hash_function != HT_NONE);
        Utils::write_varuint(_dest, static_cast<VarUInt>(info->hash_function));
    }

    if (info->hash_function != HT_NONE) {
        IncrementalHash *hashfun = hashes.get_hash(info->hash_function);
        if (hashfun == nullptr) {
            throw UnsupportedHashFunction("Unsupported hash function passed to ToBitstream.");
        }
        HashPipe<HP_WRITE> *pipe = new HashPipe<HP_WRITE>(hashfun, _dest_h);
        _dest = pipe;
        _dest_h = IOIntfHandle(pipe);
    }
}

void ToBitstream::write_container_footer(ParentInfo *info)
{
    if (info->armored) {
        Utils::write_record_type(_dest, RT_END_OF_CHILDREN);
    }

    if (info->hash_function != HT_NONE) {
        HashPipe<HP_WRITE> *pipe = dynamic_cast<HashPipe<HP_WRITE>*>(_dest);
        assert(pipe != nullptr);

        IncrementalHash *hashfun = pipe->reclaim_hash();

        _dest_h = pipe->underlying_io();
        _dest = _dest_h.get();

        intptr_t hash_length = hashfun->len();
        assert(hash_length > 0);
        Utils::write_varuint(_dest, hash_length);

        uint8_t *hash_buffer = (uint8_t*)malloc(hash_length);
        hashfun->finish(hash_buffer);
        swrite(_dest, hash_buffer, hash_length);

        delete hashfun;
    }
}

void ToBitstream::write_footer()
{
    Utils::write_record_type(_dest, RT_END_OF_CHILDREN);
}

bool ToBitstream::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    require_open();

    ParentInfo *info = new_parent_info();
    setup_container(info, cont, meta);

    VarUInt flags = get_container_flags(info);

    write_container_header(flags, info);

    _parent_stack.push_front(info);
    _curr_parent = info;
    return true;
}

bool ToBitstream::push_node(NodeHandle node)
{
    require_open();

    node->write(_dest);
    return true;
}

bool ToBitstream::end_container(const ContainerFooter *foot)
{
    require_open();

    ParentInfo *old = _curr_parent;
    _parent_stack.pop_front();
    if (_parent_stack.empty()) {
        _curr_parent = nullptr;
    } else {
        _curr_parent = _parent_stack.front();
    }

    write_container_footer(old);

    delete old;

    return true;
}

void ToBitstream::end_of_stream()
{
    close();
}

void ToBitstream::close()
{
    require_open();

    write_footer();
    _dest = nullptr;
    _dest_h = IOIntfHandle();
}

/* StructStream::ToBitstreamHashing */

ToBitstreamHashing::ToBitstreamHashing(IOIntfHandle dest):
    ToBitstream::ToBitstream(dest),
    _hash_functions()
{

}

void ToBitstreamHashing::setup_container(ToBitstream::ParentInfo *info,
                                         ContainerHandle cont,
                                         const ContainerMeta *meta)
{
    ToBitstream::setup_container(info, cont, meta);
    HashType hash_function = get_hash_function(cont->record_type(), cont->id());
    info->hash_function = hash_function;
}

HashType ToBitstreamHashing::get_hash_function(RecordType rt, ID id) const
{
    auto it = _hash_functions.find(std::pair<RecordType, ID>(rt, id));
    if (it != _hash_functions.end()) {
        return (*it).second;
    } else {
        return HT_NONE;
    }
}

void ToBitstreamHashing::set_hash_function(RecordType rt, ID id, HashType ht)
{
    if (ht != HT_NONE) {
        _hash_functions[std::pair<RecordType, ID>(rt, id)] = ht;
    } else {
        _hash_functions.erase(_hash_functions.find(std::pair<RecordType, ID>(rt, id)));
    }
}

}
