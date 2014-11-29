kayos
=====

Keep All Your Objects Safe

The project now uses submodules for forestdb and jansson.

Cloning:
git clone --recursive git@github.com:erg/kayos.git

To update dependencies:
git submodule update --remote forestdb
git submodule update --remote jansson

Build forestdb:
cd forestdb && mkdir build && cd build && cmake .. && make -j && make install && cd ../../

Build jansson:
cd jansson && mkdir build && cd build && cmake .. && make -j && make install && cd ../../

To build kayos:
make
or
make DEBUG=1

To run:
./bin/kayos-server

Producer port:
telnet 9890

Producer commands:
* set key val
* delete key

Consumer port:
telnet 9891

Consumer commands:
* get key
* iterate
