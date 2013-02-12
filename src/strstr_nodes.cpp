#include "include/strstr_nodes.hpp"

namespace StructStream {

/* StructStream::Node */

Node::Node(ID id):
    _self(),
    _id(id),
    _parent()
{

}

Node::Node(const Node &ref):
    _self(),
    _id(ref._id),
    _parent()
{

}

void detach_from_parent() {

}

/* StructStream::Container */

Container::Container(ID id):
    Node::Node(id),
    _children()
{

}

Container::Container(const Container &ref):
    Node::Node(ref),
    _children()
{
    for (auto it = ref.children_cbegin(); it != ref.children_cend(); it++) {
        child_add((*it)->copy());
    }
}

Container::~Container()
{

}

void Container::_check_valid_child(NodeHandle child)
{
    if (child->parent().get() != nullptr) {
        throw new std::exception(); //("node cannot have multiple parents. un-parent first!");
    }
}

void Container::child_add(NodeHandle child)
{
    _check_valid_child(child);
    _children.push_back(child);
    child->set_parent(std::static_pointer_cast<Container>(_self.lock()));
}

void Container::child_insert_before(NodeVector::iterator &ref, NodeHandle child)
{
    _check_valid_child(child);
    _children.insert(ref, child);
    child->set_parent(std::static_pointer_cast<Container>(_self.lock()));
}

NodeVector::iterator Container::children_begin()
{
    return _children.begin();
}

NodeVector::const_iterator Container::children_cbegin() const
{
    return _children.cbegin();
}

NodeVector::iterator Container::children_end()
{
    return _children.end();
}

NodeVector::const_iterator Container::children_cend() const
{
    return _children.cend();
}

void Container::child_erase(NodeVector::iterator &to_remove)
{
    NodeHandle child = *to_remove;
    if ((*to_remove)->parent().get() != this) {
        throw std::exception();
    }
    _children.erase(to_remove);
    child->set_parent(nullptr);
}

NodeHandle Container::copy() const
{
    return NodeHandleFactory<Container>::copy(*this);
}

}
