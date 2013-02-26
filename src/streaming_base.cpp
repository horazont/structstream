/**********************************************************************
File name: streaming_base.cpp
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
#include "structstream/streaming_base.hpp"

#include "structstream/node_container.hpp"

namespace StructStream {

ContainerMeta::ContainerMeta():
    child_count(-1)
{

}

ContainerMeta::ContainerMeta(const Container &cont):
    child_count(cont.child_count())
{

}

ContainerMeta::ContainerMeta(const ContainerMeta &ref):
    child_count(ref.child_count)
{

}

ContainerMeta::~ContainerMeta()
{

}

ContainerMeta* ContainerMeta::copy() const
{
    return new ContainerMeta(*this);
}

/* StructStream::ContainerFooter */

ContainerFooter::ContainerFooter():
    validated(false),
    hash_function(HT_INVALID)
{

}

ContainerFooter::ContainerFooter(const ContainerFooter &ref):
    validated(ref.validated),
    hash_function(ref.hash_function)
{

}

ContainerFooter::~ContainerFooter()
{

}

ContainerFooter *ContainerFooter::copy() const
{
    return new ContainerFooter(*this);
}

}
