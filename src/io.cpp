/**********************************************************************
File name: io.cpp
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
#include "structstream/io.hpp"

#include <cassert>

#include "structstream/errors.hpp"

namespace StructStream {

void sread(IOIntf *io, void *buf, const intptr_t len)
{
    intptr_t read_bytes = io->read(buf, len);
    if (read_bytes < len) {
        throw EndOfStreamError("Premature end-of-stream while reading.");
    }
}

void swrite(IOIntf *io, const void *buf, const intptr_t len)
{
    // printf("writing:");
    // for (const uint8_t *item = (const uint8_t*)buf;
    //      item < (const uint8_t*)buf + len;
    //      item++)
    // {
    //     printf(" 0x%x", *item);
    // }
    // printf("\n");
    intptr_t written_bytes = io->write(buf, len);
    if (written_bytes < len) {
        throw EndOfStreamError("Premature end-of-stream while writing.");
    }
}

}
