#include "catch.hpp"

#include <cstring>

#include "structstream/io.hpp"
#include "structstream/hashing.hpp"

using namespace StructStream;

// TODO: if we ever get an own SHA-whatever implementation, these test
// cases should use that one, so that we get rid of the dependency on GnuTLS.
#ifdef WITH_GNUTLS

static const uint8_t ref_hash[] = {
    0x88, 0x43, 0xd7, 0xf9, 0x24, 0x16, 0x21, 0x1d, 0xe9, 0xeb, 0xb9, 0x63,
    0xff, 0x4c, 0xe2, 0x81, 0x25, 0x93, 0x28, 0x78};
static const char ref_message[] = "foobar";
static const HashType ref_hash_function = HT_SHA1;

TEST_CASE ("hashing/direct", "Test a direct SHA1 hash")
{
    uint8_t hash_buf[sizeof(ref_hash)];
    memset(hash_buf, 0, sizeof(hash_buf));

    load_all_hashes();

    IncrementalHash *hash = hashes.get_hash(ref_hash_function);

    REQUIRE(hash != 0);
    hash->feed(ref_message, strlen(ref_message));

    REQUIRE(hash->len() == sizeof(hash_buf));
    hash->finish(hash_buf);

    delete hash;

    CHECK(memcmp(hash_buf, ref_hash, sizeof(hash_buf)) == 0);
}

TEST_CASE ("hashing/stream", "Using a HashPipe to feed a hash buffer")
{
    uint8_t hash_buf[sizeof(ref_hash)];
    memset(hash_buf, 0, sizeof(hash_buf));

    uint8_t msg_buf[sizeof(ref_message)];

    load_all_hashes();

    IOIntfHandle io(new WritableMemory(msg_buf, sizeof(msg_buf)));
    HashPipe<HP_WRITE> pipe(hashes.get_hash(ref_hash_function), io);

    CHECK(pipe.write(ref_message, strlen(ref_message)) == strlen(ref_message));

    IncrementalHash *hash = pipe.reclaim_hash();
    REQUIRE(hash != 0);
    REQUIRE(hash->len() == sizeof(hash_buf));
    hash->finish(hash_buf);

    REQUIRE(memcmp(hash_buf, ref_hash, sizeof(hash_buf)) == 0);
    CHECK(pipe.write(ref_message, strlen(ref_message)) == 0);
}

#endif
