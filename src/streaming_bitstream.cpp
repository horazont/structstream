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

#include "structstream/utils.hpp"
#include "structstream/errors.hpp"
#include "structstream/node_container.hpp"

namespace StructStream {

/* StructStream::FromFile */

FromFile::FromFile(IOIntfHandle source, const RegistryHandle nodetypes,
                   StreamSink sink):
    _source_h(source),
    _source(source.get()),
    _node_factory_h(nodetypes),
    _node_factory(nodetypes.get()),
    _sink_h(sink),
    _sink(sink.get()),
    _parent_stack(),
    _curr_parent()
{
    push_root();
}

FromFile::~FromFile()
{

}

void FromFile::check_end_of_container()
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

void FromFile::push_root()
{
    ParentInfo *root_pi = new ParentInfo();
    root_pi->meta->child_count = -1;
    root_pi->armored = true;
    root_pi->read_child_count = 0;
    root_pi->cont = ContainerHandle();
    _parent_stack.push_front(root_pi);
    _curr_parent = root_pi;
}

FromFile::ParentInfo *FromFile::new_parent_info() const
{
    return new ParentInfo();
}

void FromFile::start_of_container(ContainerHandle cont_h)
{
    VarUInt flags_int = Utils::read_varuint(_source);
    ParentInfo *info = new_parent_info();
    info->cont = cont_h;
    info->read_child_count = 0;

    proc_container_flags(flags_int, info);

    if (flags_int != 0) {
        delete info;
        throw UnsupportedContainerFlags("Unsupported container flags encountered.");
    }

    try {
        end_of_container_header(info);
    } catch (...) {
        delete info;
        throw;
    }

    _parent_stack.push_front(info);
    _curr_parent = info;
    _sink->start_container(info->cont, info->meta);
    info->cont = ContainerHandle();

    // printf("bitstream: push %lx\n", (uint64_t)_curr_parent->cont.get());
}

void FromFile::proc_container_flags(VarUInt &flags_int, FromFile::ParentInfo *info)
{
    info->meta->child_count = -1;
    info->footer->hash_function = HT_INVALID;
    info->armored = false;

    if ((flags_int & CF_WITH_SIZE) != 0) {
        flags_int ^= CF_WITH_SIZE;
        info->meta->child_count = Utils::read_varint(_source);
    }
    if ((flags_int & CF_ARMORED) != 0) {
        flags_int ^= CF_ARMORED;
        info->armored = true;
    }
    if (!info->armored && (info->meta->child_count == -1)) {
        throw IllegalCombinationOfFlags("Illegal combination of container flags: no CF_WITH_SIZE, but no CF_ARMORED either -- how am I supposed to find out the length?");
    }
    if ((flags_int & CF_HASHED) != 0) {
	flags_int ^= CF_HASHED;
        info->footer->hash_function = static_cast<HashType>(Utils::read_varint(_source));
    }
}

void FromFile::end_of_container_header(ParentInfo *info)
{
    if (info->footer->hash_function != HT_INVALID) {
        throw UnsupportedHashFunction("Unsupported hash function.");
    }
}

void FromFile::end_of_container_body(ContainerFooter *foot)
{

}

void FromFile::end_of_container()
{
    ParentInfo *info = _curr_parent;
    _parent_stack.pop_front();
    if (_parent_stack.empty()) {
        _curr_parent = nullptr;
    } else {
        _curr_parent = _parent_stack.front();
    }

    if (_curr_parent) {
        // printf("bitstream: pop %lx\n", (intptr_t)(info->cont.get()));

        // Do not call this virtual method for the root node
        end_of_container_body(info->footer);
        _sink->end_container(info->footer);

        _curr_parent->read_child_count += 1;
    } else {
        // printf("bitstream: end-of-stream reached\n");
        _sink->end_of_stream();
    }

    delete info;

    check_end_of_container();
}

NodeHandle FromFile::read_next() {
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
                throw UnexpectedEndOfChildren("Armored container ended unexpectedly (not all announced children found).");
            } else {
                throw UnexpectedEndOfChildren("Non-armored container closed by End-Of-Children tag. This may also imply that some children are missing.");
            }
        }
	return read_next();
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

    // printf("bitstream: found 0x%x with id 0x%lx\n", rt, id);

    NodeHandle new_node = _node_factory->node_from_record_type(rt, id);
    if (!new_node.get()) {
	throw UnsupportedRecordType("Unsupported record type.");
    }

    ContainerHandle new_parent = std::dynamic_pointer_cast<Container>(new_node);
    if (new_parent.get() != nullptr) {
        start_of_container(new_parent);
    } else {
	new_node->read(_source);
        _sink->push_node(new_node);

        _curr_parent->read_child_count++;
    }

    check_end_of_container();

    return new_node;
}

void FromFile::read_all()
{
    NodeHandle node;
    do {
        node = read_next();
    } while (node.get() != nullptr);
}

}
