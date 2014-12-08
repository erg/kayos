kayos
=====

Keep All Your Objects Safe

The project now uses submodules for forestdb and jansson.
Added support for HTTP iterate.

Cloning:
* ``git clone --recursive git@github.com:erg/kayos.git``

To update dependencies:
* ``git submodule update --remote forestdb``
* ``git submodule update --remote jansson``
* ``git submodule foreach git pull``

Build forestdb:
* ``mkdir -p forestdb/build && cd forestdb/build && cmake .. && make -j && make install && cd ../../``

Build jansson:
* ``mkdir -p jansson/build && cd jansson/build && cmake .. && make -j && make install && cd ../../``

To build kayos:
* ``make``
* ``make DEBUG=1``

To run:
* ``./bin/kayos-server``

Producer port:
* ``telnet 9890``

Producer commands:
* set key val
* delete key
* {"command": "set", "key": "a", "value": "1"}
* [{"command": "set", "key": "a", "value": "1"},{"command": "set", "key": "b", "value": "2"}]


Consumer port:
* ``telnet 9891``

Consumer commands:
* get key
* iterate
* iterate 100 -- seqnum >= 100
* {"command": "iterate"}
* {"command": "iterate", "start": "100"}
* {"command": "get", "key": "a"}

HTTP commands:
* ``curl -X GET http://127.0.0.1:9891/_iterate``
