kayos
=====

Keep All Your Objects Safe

The project now uses submodules for forestdb and jansson.
Added support for HTTP iterate.

Cloning:
* ``git clone --recursive git@github.com:erg/kayos.git``

To update submodule dependencies:
* ``git submodule update --init``
* ``git submodule update --remote forestdb``
* ``git submodule update --remote jansson``
* ``git submodule foreach git pull``

Build dependencies:
* ``cmake``
* ``snappy``
* ``sudo apt-get install cmake libsnappy-dev``

Build forestdb (Mac then Ubuntu):
* ``mkdir -p forestdb/build && cd forestdb/build && cmake .. && make -j && make install && cd ../../``
* ``sudo echo "hi" && mkdir -p forestdb/build && cd forestdb/build && cmake .. && make -j && sudo make install && cd ../../``


Build jansson (Mac then Ubuntu):
* ``mkdir -p jansson/build && cd jansson/build && cmake -DJANSSON_BUILD_SHARED_LIBS=1 .. && make -j && make install && cd ../../``
* ``sudo echo "hi" && mkdir -p jansson/build && cd jansson/build && cmake -DJANSSON_BUILD_SHARED_LIBS=1 .. && make -j && sudo make install && cd ../../``

To build kayos:
* ``make``
* ``make DEBUG=1``

To run:
* ``./bin/kayos-server dbname``
* ``./bin/kayos-producer dbname``
* ``./bin/kayos-consumer dbname``

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
