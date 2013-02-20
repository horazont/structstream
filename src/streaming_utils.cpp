/**********************************************************************
File name: streaming_utils.cpp
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
#include "structstream/streaming_utils.hpp"

namespace StructStream {

SplitStream::SplitStream(std::initializer_list<StreamSink> sinks):
    _sinks(sinks)
{

}

SplitStream::~SplitStream()
{

}

void SplitStream::start_container(ContainerHandle cont, const ContainerMeta *meta)
{
    for (auto &child: _sinks)
    {
        child->start_container(cont, meta);
    }
}

void SplitStream::push_node(NodeHandle node)
{
    for (auto &child: _sinks)
    {
        child->push_node(node);
    }
}

void SplitStream::end_container(ContainerFooter *foot)
{
    for (auto &child: _sinks)
    {
        child->end_container(foot);
    }
}

}
