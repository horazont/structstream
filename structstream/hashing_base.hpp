/**********************************************************************
File name: hashing_base.hpp
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
#ifndef _STRUCTSTREAM_HASHING_BASE
#define _STRUCTSTREAM_HASHING_BASE

#include <unordered_map>
#include <functional>

#include <structstream/static.hpp>

namespace std {

template<>
struct hash<StructStream::HashType> {
public:
    size_t operator()(const StructStream::HashType &ht) const {
        return hash<unsigned long>()(ht);
    };
};

}

namespace StructStream {

class IncrementalHash {
public:
    IncrementalHash();
    virtual ~IncrementalHash();
protected:
    IncrementalHash(const IncrementalHash& ref);
    IncrementalHash &operator= (const IncrementalHash &ref);
public:
    virtual intptr_t len() const = 0;
    virtual void feed(const void *data, intptr_t data_len) = 0;
    virtual void finish(void *digest) = 0;
};

typedef std::function< IncrementalHash*() > HashConstructor;

class HashRegistry {
public:
    HashRegistry();
    HashRegistry(const HashRegistry &ref);
    virtual ~HashRegistry();
    HashRegistry& operator= (const HashRegistry &ref);
private:
    std::unordered_map<HashType, HashConstructor> _map;
public:
    IncrementalHash *get_hash(HashType type) const;
    void register_hash(HashType type, HashConstructor con);
};

extern HashRegistry hashes;

}

#endif
