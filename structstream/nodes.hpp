/**********************************************************************
File name: strstr_nodes.hpp
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
#ifndef _STRUCTSTREAM_NODES_H
#define _STRUCTSTREAM_NODES_H

#include <memory>
#include <vector>
#include <iterator>

#include <cstring>

#include "structstream/static.hpp"
#include "structstream/io.hpp"
#include "structstream/utils.hpp"

namespace StructStream {

class Node;
typedef std::shared_ptr<Node> NodeHandle;
typedef std::weak_ptr<Node> NodeWeakHandle;
class Container;
typedef std::shared_ptr<Container> ContainerHandle;
typedef std::weak_ptr<Container> ContainerWeakHandle;

template <class NodeT>
struct NodeHandleFactory {
    inline static NodeHandle create(ID id) {
        NodeT *node = new NodeT(id);
        NodeHandle handle = NodeHandle(node);
        node->_self = NodeWeakHandle(handle);
        return handle;
    };

    inline static NodeHandle copy(const NodeT &ref) {
        NodeT *node = new NodeT(ref);
        NodeHandle handle = NodeHandle(node);
        node->_self = NodeWeakHandle(handle);
        return handle;
    };
private:
    NodeHandleFactory();
    NodeHandleFactory(const NodeHandleFactory &ref);
};

/**
 * Base class for all nodes which can written into a structstream
 * file. You may derive from this or DataRecord or Container depending
 * on your needs if you want to define a new record type.
 *
 * You must /never/ create nodes by hand, which is why the
 * constructors are protected. Use
 * NodeHandleFactory<NodeClass>::create(id) to create nodes.
 *
 * @param id structstream ID of the node
 */
class Node {
protected:
    Node(ID id);
    Node(const Node &ref);
public:
    virtual ~Node();
protected:
    NodeWeakHandle _self;
    const ID _id;
    ContainerWeakHandle _parent;
protected:
    void set_parent(ContainerHandle parent);
public:
    inline ID id() const {
        return  _id;
    };

    inline ContainerHandle parent() const {
        return _parent.lock();
    };

    /**
     * Detach the node from it's parent, do nothing if no parent is
     * assigned. This also removes the node from the parents child
     * list.
     */
    void detach_from_parent();

    /**
     * Create and return a deep copy of the node.
     */
    virtual NodeHandle copy() const = 0;

    /**
     * Return the record type of the node.
     */
    virtual RecordType record_type() const = 0;

    /**
     * If the node has contents, read them from the stream. This is
     * /not/ how containers are read. Containers do nothing in read()
     * and it's also not called by the Reader class.
     *
     * @param stream The stream to read from.
     */
    virtual void read(IOIntf *stream) = 0;

    /**
     * Write the nodes header (record type and id) and it's contents
     * to the given stream.
     *
     * This is not how containers are written. Containers do nothing
     * in write().
     *
     * @param stream The stream to write to.
     */
    virtual void write(IOIntf *stream) const = 0;

    /**
     * Helper function to write the record type and the id to a
     * stream.
     *
     * @param stream Stream to write the data to.
     */
    void write_header(IOIntf *stream) const;

    // If you want to make your node constructible using the
    // NodeHandleFactory, include this line and adapt it
    // appropriately.
    friend class NodeHandleFactory<Node>;
    friend class Container;
};

typedef std::vector<NodeHandle> NodeVector;

/**
 * The base class for all containers.
 *
 * If you implement a container, you /must/ inherit from this
 * class. There are dynamic_casts at several places, because Readers
 * and Writers need information about containers.
 */
class Container: public Node {
protected:
    Container(ID id);
    Container(const Container &ref);
public:
    virtual ~Container();
protected:
    NodeVector _children;
protected:
    void check_valid_child(NodeHandle child) const;
public:
    /**
     * Add a child to the container.
     *
     * Raises an ParentAlreadySet exception if the child is already
     * assigned to a parent.
     *
     * @param child Node to add to the container.
     */
    void child_add(NodeHandle child);

    /**
     * Return the amount of children attached to the container.
     *
     * @return Amount of children attached to the container.
     */
    intptr_t child_count() const;

    /**
     * Erase a child from the container.
     *
     * @param to_remove Iterator pointing at the child to remove.
     */
    void child_erase(NodeVector::iterator &to_remove);

    /**
     * Find a child node in the container.
     *
     * @param child handle to the node to find.
     *
     * @return Iterator pointing at the child if found or iterator
     * pointing to the end of list if not.
     */
    NodeVector::iterator child_find(NodeHandle child);

    /**
     * Insert a child before another.
     *
     * Inserts child before the child pointed to by ref.
     *
     * Raises ParentAlreadySet if child already has a parent.
     *
     * @param ref Iterator pointing to the child before which the new
     *            child will be inserted.
     * @param child The child to insert.
     */
    void child_insert_before(NodeVector::iterator &ref, NodeHandle child);

    /**
     * Return iterator pointing at the first child.
     */
    NodeVector::iterator children_begin();

    /**
     * Return constant interator pointing at the first child.
     */
    NodeVector::const_iterator children_cbegin() const;

    /**
     * Return iterator pointing behind the last child.
     */
    NodeVector::iterator children_end();

    /**
     * Return constant iterator pointing behind the last child.
     */
    NodeVector::const_iterator children_cend() const;

    /**
     * Create and return a deep copy of the container.
     *
     * This includes copying all the child nodes and their possible
     * child nodes.
     *
     * @return The new copy
     */
    virtual NodeHandle copy() const;
    virtual RecordType record_type() const {
        return RT_CONTAINER;
    };

    /**
     * A no-op. If you want to find out how reading works, check the
     * Reader class.
     */
    virtual void read(IOIntf *stream) {};

    /**
     * A no-op. If you want to find out how writing works, check the
     * Writer class.
     */
    virtual void write(IOIntf *stream) const {};

    friend class NodeHandleFactory<Container>;
};

/**
 * Base class for primitive data records.
 */
class DataRecord: public Node {
protected:
    DataRecord(ID id);
    DataRecord(const DataRecord &ref);
public:
    virtual ~DataRecord();
public:
    /**
     * Copy the contents to a buffer.
     *
     * Copy the contents of the record to a buffer given by the
     * user. It is the users responsibility to assure that the buffer
     * is large enough.
     *
     * @param to Buffer to copy the data to.
     *
     * @seealso raw_size
     */
    virtual void raw_get(void *to) const = 0;

    /**
     * Return the size of the data
     *
     * Return the amount of bytes needed to store a copy of the data
     * held by this node.
     *
     * @return Amount of bytes the data takes
     */
    virtual intptr_t raw_size() const = 0;

    /**
     * Set the contents from a buffer
     *
     * Copy the contents of the given buffer and take it as a
     * value. Will only read the amount of bytes needed to fully
     * specify the value. It is the callers responsibility to assure
     * that the buffer handed in to this function is long enough.
     *
     * @param from Buffer to read from
     *
     * @seealso raw_size
     */
    virtual void raw_set(const void *from) = 0;
};

/**
 * Template for primitive C types. Specializations of this template
 * support additional get() and set() methods, which allow compile
 * time type checking by returning/accepting respectively the type
 * stored by the specialization.
 */
template <class _T, RecordType rt>
class PrimitiveDataRecord: public DataRecord {
protected:
    PrimitiveDataRecord(ID id):
        DataRecord::DataRecord(id),
        _data() {}
    PrimitiveDataRecord(const PrimitiveDataRecord<_T, rt> &ref):
        DataRecord::DataRecord(ref),
        _data(ref._data) {}
public:
    virtual ~PrimitiveDataRecord() {}
private:
    _T _data;
public:
    virtual NodeHandle copy() const {
        return NodeHandleFactory< PrimitiveDataRecord<_T, rt> >::copy(*this);
    };

    virtual void raw_get(void *to) const {
        memcpy(to, &_data, sizeof(_T));
    };

    virtual intptr_t raw_size() const {
        return sizeof(_T);
    };

    virtual void raw_set(const void *from) {
        memcpy(&_data, from, sizeof(_T));
    };

    virtual void read(IOIntf *stream) {
	sread(stream, &_data, sizeof(_T));
    };

    virtual void write(IOIntf *stream) const {
        write_header(stream);
        swrite(stream, &_data, sizeof(_T));
    };

    virtual RecordType record_type() const {
        return rt;
    };
public:
    inline _T get() const {
        return _data;
    };

    inline void set(const _T &value) {
        _data = value;
    };

    friend class NodeHandleFactory< PrimitiveDataRecord<_T, rt> >;
};

typedef PrimitiveDataRecord<int32_t, RT_INT32> Int32Record;
typedef PrimitiveDataRecord<uint32_t, RT_UINT32> UInt32Record;
typedef PrimitiveDataRecord<int64_t, RT_INT64> Int64Record;
typedef PrimitiveDataRecord<uint64_t, RT_UINT64> UInt64Record;
typedef PrimitiveDataRecord<float, RT_FLOAT32> Float32Record;
typedef PrimitiveDataRecord<double, RT_FLOAT64> Float64Record;

/**
 * Base template to store a dynamic-length blob.
 *
 * Adds the set methods which allow reading from a buffer and a
 * std::string instance.
 */
template <class _IntfT>
class BlobDataRecord: public DataRecord {
protected:
    BlobDataRecord(ID id):
        DataRecord::DataRecord(id),
        _buf(),
        _len(0) {};
    BlobDataRecord(const BlobDataRecord<_IntfT> &ref):
        DataRecord::DataRecord(ref),
        _buf(malloc(ref.size())),
        _len(ref._len)
        {
            memcpy(_buf, ref._buf, size());
        }
public:
    virtual ~BlobDataRecord() {
        if (_buf) {
            free(_buf);
            _buf = nullptr;
        }
        _len = 0;
    }
protected:
    void *_buf;
    intptr_t _len;
protected:
    inline intptr_t size() const {
        return _len * sizeof(_IntfT);
    };
public:
    virtual void raw_get(void *to) const {
        memcpy(to, _buf, size());
    };

    virtual intptr_t raw_size() const {
        return size();
    };

    virtual void raw_set(const void *from) {
        memcpy(_buf, from, size());
    };

    virtual void read(IOIntf *stream) {
	VarInt length = Utils::read_varint(stream);
	if (length < 0) {
	    throw std::exception();
	}
	if (length != _len) {
	    _len = length;
	    _buf = realloc(_buf, size());
	}
	sread(stream, _buf, size());
    };
public:
    void set(const _IntfT *from, const intptr_t len) {
        if (len != _len) {
            _len = len;
            _buf = realloc(_buf, size());
        }
        memcpy(_buf, from, size());
    };

    void set(const std::string &str) {
        set((_IntfT*)str.c_str(), str.size());
    };

};

/**
 * Implement a UTF8 string record.
 */
class UTF8Record: public BlobDataRecord<char> {
protected:
    UTF8Record(ID id):
        BlobDataRecord<char>::BlobDataRecord(id) {};
    UTF8Record(const UTF8Record &ref):
        BlobDataRecord<char>::BlobDataRecord(ref) {};
public:
    virtual ~UTF8Record();
public:
    virtual NodeHandle copy() const;
    virtual void read(IOIntf *stream);
    virtual void write(IOIntf *stream) const;

    virtual RecordType record_type() const {
        return RT_UTF8STRING;
    };

    friend class NodeHandleFactory<UTF8Record>;
};

/**
 * Implement a binary blob of arbitrary length with arbitrary
 * contents.
 */
class BlobRecord: public BlobDataRecord<char> {
protected:
    BlobRecord(ID id):
        BlobDataRecord<char>::BlobDataRecord(id) {};
    BlobRecord(const BlobRecord &ref):
        BlobDataRecord<char>::BlobDataRecord(ref) {};
public:
    virtual ~BlobRecord();
public:
    virtual NodeHandle copy() const;
    virtual RecordType record_type() const {
        return RT_BLOB;
    };

    virtual void write(IOIntf *stream) const;

    friend class NodeHandleFactory<BlobRecord>;
};

};

#endif
