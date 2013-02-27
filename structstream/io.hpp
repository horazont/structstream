/**********************************************************************
File name: io.hpp
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
#ifndef _STRUCTSTREAM_IO_H
#define _STRUCTSTREAM_IO_H

#include "structstream/io_base.hpp"
#include "structstream/io_memory.hpp"
#include "structstream/io_std.hpp"
#include "structstream/io_hash.hpp"

namespace StructStream {

void sread(IOIntf *io, void *buf, const intptr_t len);
void swrite(IOIntf *io, const void *buf, const intptr_t len);
void sskip(IOIntf *io, const intptr_t len);

template <class _T>
inline void swritev(IOIntf *io, const _T value)
{
    swrite(io, &value, sizeof(_T));
}

}

#endif
