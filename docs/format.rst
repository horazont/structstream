Bitstream format
****************

The structstream file format is designed to be simple and yet
extensible. Still, it is supposed to be not too large, so we don't
want to waste space in places where we don't have to.

structstream is loosly based on the EBML format, but it has some
strong simplifications. As an EBML relative, it also uses varints as
described in the EBML RFC to store (expectedly) small integer
numbers in the format metadata.

In the following, we use some extensions of the ABML form to simplify
writing the specification of the format a bit. First, the symbol
``varint`` shall refer to a serialized varint. If the symbol is
immediately (without space) followed by a pair of braces (``()``),
whatever these braces enclose shall be encoded as varint.

If a symbol is immediately followed by a pair of sharp braces
(``<>``), this defines a reference to the symbol. We make use of
references in the text below the ABML forms.

Node tree
=========

The format basically consists of several trees of nodes, terminated by
the ``END_OF_CHILDREN`` marker. Each node can either be a container or
a single data record.

Whether it is a container or a data record depends on the
record-type. By default, only the ``CONTAINER`` record-type designates
a container node, but implementations MAY define arbitrary additional
container node types. These, however, will be rejected by standard
parsers which do not offer these extensions. Thus, we find the
following rough ABML for the format::

    bitstream := *node end_of_children

    node := varuint<record-type> varuint<id> node_body

    node_body := container_body
              |  record_body

    end_of_children := varint(RT_END_OF_CHILDREN)

Note that this ABML matches more documents than which are in fact
allowed, as we did not distinguish based on the record-type here. The
*id* is encoded for semantic purposes and the application is free to
choose any valid varuint as an id.

Container nodes
===============

Container nodes are rather complex nodes which can carry other nodes
within them. In the header, they have a varint field which defines the
flags of the container node::

    container_header := varuint<flags>

``flags`` is a bitfield which can contain several flags. Depending on
the flags, the remaining header is a bit different (and also can have
a different length).

If the ``WITH_SIZE`` flag is set, a varuint follows which defines the
amount of children which are held in the container.

If the ``ARMORED`` flag is set, the container is finished by an
``END_OF_CHILDREN`` marker. If ``WITH_SIZE`` is *not* set, ``ARMORED``
MUST be set on a container.

If the ``HASHED`` flag is set, a varuint designating the hash function
used follows. The whole container body after the header is hashed with
the hash function and the resulting hash is appended after the last
child (or the ``END_OF_CHILDREN`` marker, if present). The length of
the hash is prepended so that even implementations which do not
support the used hash can read beyond the container.

Thus, containers look similar to this::

    container_body := container_header *1varuint<size>
                   *1varuint<hash-function> children *1end_of_children
                   *1varuint<hash-length> *<hash-length>BYTE

Hash types
----------

The following hash types are specified:

================== ======== =========================================
HashType enum name value    interpretation
================== ======== =========================================
``NONE``           ``0x00`` *undefined*: a parser MUST raise an error
                            if it encounters this hash type.
``SHA1``           ``0x01`` a standard SHA1 checksum
``SHA256``         ``0x02`` a standard SHA256 checksum
``SHA512``         ``0x03`` a standard SHA512 checksum
``CRC32``          ``0x04`` a standard CRC32 checksum
                            (``0x04C11DB7``)
``MD5``            ``0x05`` a standard MD5 checksum
``INVALID``        ``0x7f`` *reserved* for testing purposes. Do not
                            use.
================== ======== =========================================

Furthermore, an application is allowed to define custom hash types
whose value is above ``0x40``, except ``0x7f``. Everything below
``0x40`` is either specified or reserved.

Record nodes
============

The format of a record node depends on it's record type. Also, it's
length is either deduced from the record type (for fixed-length types)
or encoded in the bitstream.

The basic format supports the following record types (the
``RT_``-prefix is omitted from the table).

==================== ======== =======================================
RecordType enum name value    content format
==================== ======== =======================================
``RESERVED``         ``0x00`` *undefined* a parser MUST raise an
                              error if it encounters this record
                              type. It is reserved for future
                              extension of the specification.
``UINT32``           ``0x01`` 4 byte unsigned integer, little endian
``INT32``            ``0x02`` 4 byte signed integer, little endian
``UINT64``           ``0x03`` 8 byte unsigned integer, little endian
``INT64``            ``0x04`` 8 byte signed integer, little endian
``BOOL_TRUE``        ``0x05`` *no contents*, shall be interpreted as
                              boolean ``True``
``BOOL_FALSE``       ``0x06`` *no contents*, shall be interpreted as
                              boolean ``False``
``FLOAT32``          ``0x08`` 4 byte IEEE float (single precision),
                              little endian
``FLOAT64``          ``0x09`` 8 byte IEEE float (double precision),
                              little endian
``UTF8STRING``       ``0x0A`` varuint designating the length,
                              followed by that amount of bytes, which
                              are to be interpreted as UTF-8 encoded
                              string. No null-termination is
                              included.
``BLOB``             ``0x0B`` varuint designating the length,
                              followed by that amount of bytes, which
                              are to be taken as raw binary data.
``END_OF_CHILDREN``  ``0x0C`` *no contents*, marks the end of an
                              armored container.
``VARINT``           ``0x0D`` a signed variable-length integer
``VARUINT``          ``0x0E`` an unsigned variable-length integer
==================== ======== =======================================

Furthermore, the following ranges of RecordTypes are reserved and have
a special meaning:

* ``APPBLOB_MIN = 0x40`` .. ``APPBLOB_MAX = 0x5f``

  Applications may pick arbitrary record types from within this range,
  but records marked with an ID from within the above range MUST have
  a varuint encoding the length of the payload right after their
  headers. This allows parsers to skip these records if they are not
  interested in their contents, but the overall document structure.

* ``APP_NOSIZE_MIN = 0x60`` .. ``APP_NOSIZE_MAX = 0x7f``

  Applications may pick arbitrary record types from within this range
  without further restrictions. Note however that parsers which do not
  know about these record types will fail to parse the bitstream.

Everything outside these ranges and not specified in the table above
MUST NOT be used and is to be considered reserved.
