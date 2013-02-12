#ifndef _STRSTR_NODES_H
#define _STRSTR_NODES_H

#include <memory>
#include <vector>
#include <iterator>

#include <cstring>

#include "include/strstr_static.hpp"

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

    virtual NodeHandle copy() const = 0;
    void detach_from_parent();
    virtual RecordType record_type() const = 0;

    friend class NodeHandleFactory<Node>;
    friend class Container;
};

typedef std::vector<NodeHandle> NodeVector;

class Container: public Node {
protected:
    Container(ID id);
    Container(const Container &ref);
public:
    virtual ~Container();
protected:
    NodeVector _children;
protected:
    void _check_valid_child(NodeHandle child);
public:
    void child_add(NodeHandle child);
    void child_erase(NodeVector::iterator &to_remove);
    void child_insert_before(NodeVector::iterator &ref, NodeHandle child);

    NodeVector::iterator children_begin();
    NodeVector::const_iterator children_cbegin() const;
    NodeVector::iterator children_end();
    NodeVector::const_iterator children_cend() const;

    virtual NodeHandle copy() const;
    virtual RecordType record_type() const {
        return RT_CONTAINER;
    };

    friend class NodeHandleFactory<Container>;
};

class DataRecord: public Node {
protected:
    DataRecord(ID id);
    DataRecord(const DataRecord &ref);
public:
    virtual ~DataRecord();
public:
    virtual void raw_get(void *to) const = 0;
    virtual intptr_t raw_size() const = 0;
    virtual void raw_set(const void *from) = 0;
};

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

typedef PrimitiveDataRecord<uint64_t, RT_UINT> UIntRecord;
typedef PrimitiveDataRecord<int64_t, RT_INT> IntRecord;
typedef PrimitiveDataRecord<float, RT_FLOAT32> Float32Record;
typedef PrimitiveDataRecord<double, RT_FLOAT64> Float64Record;

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
private:
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

class UTF8Record: public BlobDataRecord<char> {
protected:
    UTF8Record(ID id):
        BlobDataRecord<char>::BlobDataRecord(id) {};
    UTF8Record(const UTF8Record &ref):
        BlobDataRecord<char>::BlobDataRecord(ref) {};
public:
    virtual ~UTF8Record() {};
public:
    virtual NodeHandle copy() const {
        return NodeHandleFactory<UTF8Record>::copy(*this);
    };

    virtual RecordType record_type() const {
        return RT_UTF8STRING;
    };

    friend class NodeHandleFactory<UTF8Record>;
};

class BlobRecord: public BlobDataRecord<char> {
protected:
    BlobRecord(ID id):
        BlobDataRecord<char>::BlobDataRecord(id) {};
    BlobRecord(const BlobRecord &ref):
        BlobDataRecord<char>::BlobDataRecord(ref) {};
public:
    virtual ~BlobRecord() {};
public:
    virtual NodeHandle copy() const {
        return NodeHandleFactory<BlobRecord>::copy(*this);
    };

    virtual RecordType record_type() const {
        return RT_BLOB;
    };

    friend class NodeHandleFactory<BlobRecord>;
};

};

#endif
