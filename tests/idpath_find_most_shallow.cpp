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

TEST_CASE ("idpath/most_shallow/simple", "Foo")
{
    ContainerHandle c1, c2, c3;

    ContainerHandle tree = NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(
	0x00,
	{
	    c1 = NodeHandleFactory<Container>::create(0x01),
	    c2 = NodeHandleFactory<Container>::create(0x01),
	    NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(0x02, {
                c3 = NodeHandleFactory<Container>::create(0x01),
	    })
	}
	);

    idpath_find_most_shallow foo(0x01, tree);

    NodeHandle c = *foo;
    CHECK(foo.valid());
    CHECK(c.get() == c1.get());
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

    idpath_find_most_shallow foo(0x01, tree);

    NodeHandle c = *foo;
    CHECK(c.get() == c1.get());
    ++foo;
    CHECK((*foo).get() == c2.get());
    ++foo;
    CHECK((*foo).get() == c3.get());
    ++foo;
    CHECK(!foo.valid());
}

TEST_CASE ("idpath/most_shallow/even_more_herings", "Foo")
{
    ContainerHandle c1, c2, c3;

    ContainerHandle tree = NodeHandleFactory<Container>::createv<std::initializer_list<NodeHandle>>(
	0x00,
	{
	    c1 = NodeHandleFactory<Container>::create(0x01),
	    c2 = NodeHandleFactory<Container>::create(0x01),
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

    idpath_find_most_shallow foo(0x01, tree);

    NodeHandle c = *foo;
    CHECK(c.get() == c1.get());
    ++foo;
    CHECK((*foo).get() == c2.get());
    ++foo;
    CHECK((*foo).get() == c3.get());
    ++foo;
    CHECK(!foo.valid());
}
