StructStream++
**************

*StructStream* is a flexible binary container format for arbitrary
data. It maintains portability of encoded standard [#standard-data]_
data and allows to keep binary data in an extensible, structured way,
much like you can keep non-binary data in XML (well, not *that*
flexible and semantic, but you might get the idea).

StructStream gives the user a simple API to access the data parsed
from a bitstream. To learn about how to use it, you might want to have
a look at the `example <examples/person.cpp>`_. It
features automatic serialization and deserialization of a C++ data
`struct`, which also includes a string, as well as other gimmicks.

.. [#standard-data] Standard data means, data which has been specified
   in the official format. This includes, among others, the usual
   integer and float types as well as strings.

   For a full reference on the StructStream bitstream format, check
   out the `latest format specification <docs/format.rst>`_.
