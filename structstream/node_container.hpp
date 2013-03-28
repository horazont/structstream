/**********************************************************************
File name: node_container.hpp
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
#ifndef _STRUCTSTREAM_NODE_CONTAINER_H
#define _STRUCTSTREAM_NODE_CONTAINER_H

#include <vector>
#include <map>

#include "structstream/node_base.hpp"

namespace StructStream {

typedef std::vector<NodeHandle> NodeVector;

class idpath_find_most_shallow;

/**
 * The base class for all containers.
 *
 * If you implement a container, you /must/ inherit from this
 * class. There are dynamic_casts at several places, because Readers
 * and Writers need information about containers.
 */
class Container: public Node {
public:
    typedef std::multimap<ID, NodeHandle>::iterator NodeByIDIterator;
    typedef std::multimap<ID, NodeHandle>::const_iterator NodeByIDConstIterator;
    typedef std::pair<NodeByIDConstIterator, NodeByIDConstIterator> NodeRangeByID;
protected:
    explicit Container(ID id);
    Container(ID id, std::initializer_list<NodeHandle> children);
    Container(const Container &ref);
public:
    virtual ~Container();
private:
    bool _validated;
    HashType _hash_function;
protected:
    NodeVector _children;
    std::multimap<ID, NodeHandle> _id_lut;
protected:
    void check_valid_child(NodeHandle child) const;
    void checkin_child(NodeHandle child);
    void checkout_child(NodeHandle child);
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
    void child_erase(NodeVector::iterator to_remove);

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
     * Return a pair of iterators to grant access to the range of
     * children with equal id.
     *
     * The first iterator designates the start of the range and the
     * last iterator designetes the end of the range. As with the
     * usual iterator semantics, you should stop iteration when
     * reaching the second iterator, without evaluating it.
     */
    NodeRangeByID children_by_id(const ID id) const;

    NodeHandle first_child_by_id(const ID id) const;

    idpath_find_most_shallow idpath_most_shallow(const ID id) const;

    void set_hashed(bool validated, HashType hash_function = HT_NONE);

    inline HashType get_hashed() const {
        return  _hash_function;
    };

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

    friend struct NodeHandleFactory<Container>;
};

}

#endif
