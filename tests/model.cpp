/**********************************************************************
File name: model.cpp
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
#include "catch.hpp"

#include "structstream/node_container.hpp"
#include "structstream/node_primitive.hpp"

using namespace StructStream;

TEST_CASE ("model/container", "Test the container model thoroughly")
{
    ContainerHandle cont = NodeHandleFactory<Container>::create(0x00);

    cont->child_add(NodeHandleFactory<UInt32Record>::create(0x01));

    std::shared_ptr<UInt32Record> rec = NodeHandleFactory<UInt32Record>::create(0x02);
    cont->child_add(rec);
    cont->child_add(NodeHandleFactory<UInt32Record>::create(0x03));
    cont->child_add(NodeHandleFactory<UInt32Record>::create(0x04));

    CHECK(cont->child_count() == 4);

    auto it = cont->child_find(rec);
    REQUIRE(it != cont->children_end());

    cont->child_erase(it);
    CHECK(cont->child_count() == 3);

    CHECK(!rec->parent());

    ContainerHandle clone = NodeHandleFactory<Container>::copy(*cont);

    CHECK(clone->child_count() == 3);
    CHECK(*clone->children_begin() != *cont->children_begin());

    clone = nullptr;

    clone = std::static_pointer_cast<Container>(cont->shallow_copy());

    CHECK(clone->child_count() == 0);
}
