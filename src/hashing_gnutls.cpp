/**********************************************************************
File name: hashing_gnutls.cpp
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
#ifndef _STRUCTSTREAM_HASHING_GNUTLS_H
#define _STRUCTSTREAM_HASHING_GNUTLS_H

#include <structstream/hashing_gnutls.hpp>

#include <structstream/hashing_base.hpp>
#include <structstream/errors.hpp>

#ifdef WITH_GNUTLS

#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>

#endif

namespace StructStream {

#ifdef WITH_GNUTLS

class GnuTLSError: public std::runtime_error {
public:
    GnuTLSError(int error_code):
        std::runtime_error(std::string("GnuTLS error: ") + std::to_string(error_code)),
        _error_code(error_code)
    {
    };
private:
    int _error_code;
};

class GnuTLSHash: public IncrementalHash {
public:
    GnuTLSHash(gnutls_digest_algorithm_t algorithm):
        _ready(false),
        _hash_hd(),
        _len(gnutls_hash_get_len(algorithm))
    {
        int err = gnutls_hash_init(&_hash_hd, algorithm);
        if (err < 0) {
            throw GnuTLSError(err);
        }
        _ready = true;
    };

    virtual ~GnuTLSHash() {
        if (_ready) {
            gnutls_hash_deinit(_hash_hd, nullptr);
        }
    };
private:
    bool _ready;
    gnutls_hash_hd_t _hash_hd;
    int _len;
public:
    virtual intptr_t len() const
    {
        return _len;
    };

    virtual void feed(const void* data, intptr_t data_len)
    {
        if (!_ready) {
            throw std::logic_error("feed() cannot be called on finish'd hash.");
        }
        int err = gnutls_hash(_hash_hd, data, data_len);
        if (err < 0) {
            throw GnuTLSError(err);
        }
    };

    virtual void finish(void* digest)
    {
        if (!_ready) {
            throw std::logic_error("finish() cannot be called on finish'd hash.");
        }
        gnutls_hash_deinit(_hash_hd, digest);
        _ready = false;
    };
};

template <gnutls_digest_algorithm_t algo>
IncrementalHash* create_gnutls_hash()
{
    return new GnuTLSHash(algo);
}

#endif

bool load_gnutls_hashes_into(HashRegistry &registry)
{
#ifdef WITH_GNUTLS
    registry.register_hash(HT_SHA1, create_gnutls_hash<GNUTLS_DIG_SHA1>);
    registry.register_hash(HT_SHA256, create_gnutls_hash<GNUTLS_DIG_SHA256>);
    registry.register_hash(HT_SHA512, create_gnutls_hash<GNUTLS_DIG_SHA512>);
    return true;
#else
    return false;
#endif
};

bool load_gnutls_hashes()
{
    return load_gnutls_hashes_into(hashes);
}

}


#endif
