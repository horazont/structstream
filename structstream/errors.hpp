/**********************************************************************
File name: strstr_errors.hpp
This file is part of: ebml++

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

For feedback and questions about ebml++ please e-mail one of the authors
named in the AUTHORS file.
**********************************************************************/
#ifndef _STRUCTSTREAM_ERRORS_H
#define _STRUCTSTREAM_ERRORS_H

#include <stdexcept>

namespace StructStream {

template<class Ancestor>
class DefaultException: public Ancestor
{
public:
    DefaultException(const std::string& what_arg):
        Ancestor(what_arg) {};
    DefaultException(const char *what_arg):
        Ancestor(what_arg) {};
    DefaultException(const DefaultException& ref):
        Ancestor(ref) {};
};

// programmers errors
typedef DefaultException<std::logic_error> ParentAlreadySet;
typedef DefaultException<std::logic_error> AlreadyOpen;
typedef DefaultException<std::logic_error> AlreadyClosed;

// errors for incoming files
typedef DefaultException<std::runtime_error> FormatError;
typedef DefaultException<std::runtime_error> UnsupportedInput;


typedef DefaultException<UnsupportedInput> UnsupportedContainerFlags;
typedef DefaultException<UnsupportedInput> UnsupportedRecordType;
typedef DefaultException<UnsupportedInput> UnsupportedHashFunction;

typedef DefaultException<FormatError> MissingEndOfChildren;
typedef DefaultException<FormatError> UnexpectedEndOfChildren;
typedef DefaultException<FormatError> IllegalCombinationOfFlags;
typedef DefaultException<FormatError> InvalidIDError;

}

#endif
