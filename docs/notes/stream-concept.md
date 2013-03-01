Stream concept
==============

Currently: reader and writer share some functionality, same goes for
serializers and deserializers.

The new concept shall abstract this all away a little bit. We'll have
the intermediate *stream* format, which is nothing else than a
sequence of function calls, so called *stream events*, which determine
the contents of the structstream. It is used to communicate between
*stream nodes*, which can convert the *stream* to anything usable,
such as a *octet stream*, a *document tree* or a data tree (what
deserializers currently do). Likewise, the other direction shall be
possible, converting any of these into a stream.

Inside the stream one could imagine attaching hooks, which would allow
for an application to make use of certain *stream events*.

User interface / API
--------------------

Classes to define:

* ``bitstream_to_stream``: Convert a bitstream (i.e. a sequence of
  bytes which resememble the structstream) into a *stream*.
* ``tree_to_stream``: Convert a document tree into a *stream*.
* ``data_to_stream``: Convert a set of data objects into a *stream*,
  much what serializers do right now.
* ``stream_to_bitstream``: Convert a *stream* into a *bitstream*.
* ``stream_to_tree``: Convert a *stream* into a document tree.
* ``stream_to_data``: Deserialize a *stream* into a set of data
  objects, much what deserializers do right now.

Also, there should be the following addon classes to manage streams:

* ``split_stream``: Forward *stream events* to multiple child nodes.
* ``chain_stream``: Append one *stream* onto another.

The minimal internal stream API
-------------------------------

Methods to define on a stream sink:

* ``void start_container(container_meta *meta)``

  Start a new container in the current container. Only the metadata of
  the container is required; it not neccessary or supported to feed
  all children here at once.

  ``container_meta`` is a to-be-defined struct which shall contain at
  least the following data:

  * ``id`` of the container to create
  * ``record_type`` of the container to create
  * ``child_count`` amount of children to anticipate or -1 if unknown

  Some endpoints may support subclasses of ``container_meta`` which
  offer more information, but all sinks *MUST* be able to work
  with only this set of data and all sources *MUST* at least emit this
  set of data.

* ``void push_node(NodeHandle node)``

  Push a node to the current container. This requires the full node,
  as nodes do not share a very common format, except for their record
  type and the ID.

* ``void end_container()``

  End the current container and switch one context upwards.
