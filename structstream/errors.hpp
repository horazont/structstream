/**********************************************************************
File name: errors.hpp
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
    virtual ~DefaultException() noexcept {};
};

// programmers errors
typedef DefaultException<std::logic_error> ParentAlreadySet;
typedef DefaultException<std::logic_error> AlreadyOpen;
typedef DefaultException<std::logic_error> AlreadyClosed;
typedef DefaultException<std::logic_error> NotMyChild;


/**
 * FormatErrors represent corruption, malicious input data or unknown
 * features of a future format specification.
 */
class FormatError: public std::runtime_error {
public:
    FormatError(const std::string& what_arg): std::runtime_error(what_arg) {};
    FormatError(const char *what_arg): std::runtime_error(what_arg) {};
    FormatError(const FormatError &ref): std::runtime_error(ref) {};
};

/**
 * An illegal value for a field was encountered.
 */
class IllegalData: public FormatError {
public:
    IllegalData(const std::string& what_arg): FormatError(what_arg) {};
    IllegalData(const char *what_arg): FormatError(what_arg) {};
    IllegalData(const IllegalData &ref): FormatError(ref) {};
};

/**
 * There was an illegal (per spec) combination of container flags.
 */
class IllegalCombinationOfFlags: public IllegalData {
public:
    IllegalCombinationOfFlags(const std::string& what_arg): IllegalData(what_arg) {};
    IllegalCombinationOfFlags(const char *what_arg): IllegalData(what_arg) {};
    IllegalCombinationOfFlags(const IllegalCombinationOfFlags &ref): IllegalData(ref) {};
};

/**
 * There was an invalid ID on a record.
 */
class InvalidIDError: public IllegalData {
public:
    InvalidIDError(const std::string& what_arg): IllegalData(what_arg) {};
    InvalidIDError(const char *what_arg): IllegalData(what_arg) {};
    InvalidIDError(const InvalidIDError &ref): IllegalData(ref) {};
};

/**
 * An RT_END_OF_CHILDREN was expected at a certain point, but not
 * found in the input.
 */
class MissingEndOfChildren: public FormatError {
public:
    MissingEndOfChildren(const std::string& what_arg): FormatError(what_arg) {};
    MissingEndOfChildren(const char *what_arg): FormatError(what_arg) {};
    MissingEndOfChildren(const MissingEndOfChildren &ref): FormatError(ref) {};
};

/**
 * An RT_END_OF_CHILDREN record was encountered where none should have
 * been, for example at the end of an unarmored container or in the
 * middle of the stream.
 */
class UnexpectedEndOfChildren: public FormatError {
public:
    UnexpectedEndOfChildren(const std::string& what_arg): FormatError(what_arg) {};
    UnexpectedEndOfChildren(const char *what_arg): FormatError(what_arg) {};
    UnexpectedEndOfChildren(const UnexpectedEndOfChildren &ref): FormatError(ref) {};
};

/**
 * UnsupportedInput is thrown if a feature is required which is known
 * to the parser but not supported (e.g. certain hash functions).
 */
class UnsupportedInput: public std::runtime_error {
public:
    UnsupportedInput(const std::string& what_arg): std::runtime_error(what_arg) {};
    UnsupportedInput(const char *what_arg): std::runtime_error(what_arg) {};
    UnsupportedInput(const UnsupportedInput &ref): std::runtime_error(ref) {};
};

/**
 * There were unknown/unsupported container flags.
 */
class UnsupportedContainerFlags: public UnsupportedInput {
public:
    UnsupportedContainerFlags(const std::string& what_arg): UnsupportedInput(what_arg) {};
    UnsupportedContainerFlags(const char *what_arg): UnsupportedInput(what_arg) {};
    UnsupportedContainerFlags(const UnsupportedContainerFlags &ref): UnsupportedInput(ref) {};
};

/**
 * An unsupported record type was encountered.
 */
class UnsupportedRecordType: public UnsupportedInput {
public:
    UnsupportedRecordType(const std::string& what_arg): UnsupportedInput(what_arg) {};
    UnsupportedRecordType(const char *what_arg): UnsupportedInput(what_arg) {};
    UnsupportedRecordType(const UnsupportedRecordType &ref): UnsupportedInput(ref) {};
};

/**
 * The requested hash function is not supported.
 */
class UnsupportedHashFunction: public UnsupportedInput {
public:
    UnsupportedHashFunction(const std::string& what_arg): UnsupportedInput(what_arg) {};
    UnsupportedHashFunction(const char *what_arg): UnsupportedInput(what_arg) {};
    UnsupportedHashFunction(const UnsupportedHashFunction &ref): UnsupportedInput(ref) {};
};

}

#endif
