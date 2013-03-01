/**********************************************************************
File name: idpath_find_most_shallow.cpp
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
#include "structstream/idpath.hpp"
#include "structstream/node_primitive.hpp"

using namespace StructStream;

TEST_CASE ("iter/nodetree/all", "Check that a NodeTreeIterator catches all nodes")
{
    NodeHandle items[6];

    ContainerHandle tree = NodeHandleFactory<Container>::create_with_children(
        0x00,
        {
            items[0] = NodeHandleFactory<UInt32Record>::create(0x01),
            items[1] = NodeHandleFactory<UInt32Record>::create(0x01),
            items[2] = NodeHandleFactory<Container>::create_with_children(
                0x01,
                {
                    items[3] = NodeHandleFactory<UInt32Record>::create(0x01),
                    items[4] = NodeHandleFactory<UInt32Record>::create(0x01),
                }),
            items[5] = NodeHandleFactory<UInt32Record>::create(0x01),
        });

    NodeTreeIterator iter(tree);
    for (NodeHandle item: items) {
        REQUIRE(iter.valid());
        CHECK(*iter == item);
        ++iter;
    }
    CHECK(!iter.valid());
}

TEST_CASE ("idpath/most_shallow/empty", "An empty finder is immediately invalid")
{
    ContainerHandle tree = NodeHandleFactory<Container>::create(0x00);
    FindByID filter(0x01);
    FindMostShallow iter(tree, filter);
    CHECK(!iter.valid());
}

TEST_CASE ("idpath/most_shallow/non-matching", "A finder without matching elements is immediately invalid")
{
    ContainerHandle tree = NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(
        0x00,
        {
            NodeHandleFactory<UInt32Record>::create(0x02),
            NodeHandleFactory<UInt32Record>::create(0x02),
            NodeHandleFactory<UInt32Record>::create(0x02)
        }
        );
    FindByID filter(0x01);
    FindMostShallow iter(tree, filter);
    CHECK(!iter.valid());
}

TEST_CASE ("idpath/most_shallow/simple", "Foo")
{
    NodeHandle c1, c2, c3;

    ContainerHandle tree = NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(
	0x00,
	{
	    c1 = NodeHandleFactory<UInt32Record>::create(0x01),
	    c2 = NodeHandleFactory<UInt32Record>::create(0x01),
	    NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(0x02, {
                c3 = NodeHandleFactory<Container>::create(0x01),
	    })
	}
	);

    FindByID filter(0x01);
    FindMostShallow foo(tree, filter);

    NodeHandle c = *foo;
    CHECK(foo.valid());
    CHECK((*foo).get() == c1.get());
    ++foo;
    CHECK((*foo).get() == c2.get());
    ++foo;
    CHECK((*foo).get() == c3.get());
    ++foo;
    CHECK(!foo.valid());
}

TEST_CASE ("idpath/most_shallow/more_herings", "Foo")
{
    ContainerHandle c1, c2, c3;

    ContainerHandle tree = NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(
	0x00,
	{
	    c1 = NodeHandleFactory<Container>::create(0x01),
	    c2 = NodeHandleFactory<Container>::create(0x01),
	    NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(0x02, {
                NodeHandleFactory<UInt32Record>::create(0x02),
                c3 = NodeHandleFactory<Container>::create(0x01),
                NodeHandleFactory<UInt32Record>::create(0x02),
	    }),
            NodeHandleFactory<UInt32Record>::create(0x02)
	}
	);

    FindByID filter(0x01);
    FindMostShallow foo(tree, filter);

    CHECK((*foo).get() == c1.get());
    ++foo;
    CHECK((*foo).get() == c2.get());
    ++foo;
    CHECK((*foo).get() == c3.get());
    ++foo;
    CHECK(!foo.valid());
    if (foo.valid()) {
        printf("hint: (*foo).get() == 0x%lx\n", (intptr_t)(*foo).get());
    }
}

TEST_CASE ("idpath/most_shallow/even_more_herings", "Foo")
{
    NodeHandle c1, c2, c3;

    ContainerHandle tree = NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(
	0x00,
	{
	    c1 = NodeHandleFactory<UInt32Record>::create(0x01),
	    c2 = NodeHandleFactory<UInt32Record>::create(0x01),
	    NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(0x02, {
                NodeHandleFactory<UInt32Record>::create(0x02),
                c3 = NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(0x01, {
                    NodeHandleFactory<UInt32Record>::create(0x01)
                }),
                NodeHandleFactory<UInt32Record>::create(0x02),
	    }),
            NodeHandleFactory<UInt32Record>::create(0x02)
	}
	);

    FindByID filter(0x01);
    FindMostShallow foo(tree, filter);

    CHECK((*foo).get() == c1.get());
    ++foo;
    CHECK((*foo).get() == c2.get());
    ++foo;
    CHECK((*foo).get() == c3.get());
    ++foo;
    CHECK(!foo.valid());
}
