/**********************************************************************
File name: streaming_utils.hpp
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
#ifndef _STRUCTSTREAM_STREAMING_UTILS_H
#define _STRUCTSTREAM_STREAMING_UTILS_H

#include <forward_list>
#include <initializer_list>

#include "structstream/streaming_base.hpp"

namespace StructStream {

class SplitStream: public StreamSinkIntf {
public:
    SplitStream(std::initializer_list<StreamSink> sinks);
    virtual ~SplitStream();
private:
    std::forward_list<StreamSink> _sinks;
public:
    virtual void start_container(ContainerHandle cont, const ContainerMeta *meta);
    virtual void push_node(NodeHandle node);
    virtual void end_container(ContainerFooter *foot);
};

}

#endif