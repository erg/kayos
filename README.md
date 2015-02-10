# kayos

Keep All Your Objects Safe

## Cloning:
* ``git clone --recursive git@github.com:erg/kayos.git``

## Build dependencies:
* ``cmake``
* ``snappy``
* ``sudo apt-get install cmake libsnappy-dev``

## To build kayos:
* Unix: ``mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . -- -j9``

## To run:
* ``./kayos-server dbname``
* ``./kayos-producer dbname``
* ``./kayos-consumer dbname``

## Producer port:
* ``telnet localhost 9890``

## Producer commands:
* `set key val`
* `delete key`
* `{"command": "set", "key": "a", "value": "1"}`
* `[{"command": "set", "key": "a", "value": "1"},{"command": "set", "key": "b", "value": "2"}]`


## Consumer port:
* `telnet localhost 9891`

## Consumer commands:
* `get key`
* `iterate`
* `iterate 100 -- seqnum >= 100`
* `{"command": "iterate"}`
* `{"command": "iterate", "start": "100"}`
* `{"command": "get", "key": "a"}`

## HTTP commands:
* `curl -X GET http://127.0.0.1:9891/_iterate`

## Troubleshooting:
* `./kayos-consumer: error while loading shared libraries: libforestdb.so: cannot open shared object file: No such file or directory`
Run `ldconfig` as root to udpate the library cache.

* ```erg@ubuntu64:~/kayos/build$ ./kayos-consumer asdf
cannot make path: /usr/local/var/lib/kayos
libc fatal_error: mkdir failed
errno = 13, strerrno: Permission denied```
Fix the permissions for `/usr/local` so that it is writable by your user.


### git submodule cheat sheet:
* `git submodule update --init`
* `git submodule update --remote forestdb`
* `git submodule update --remote jansson`
* `git submodule update --remote http-parser`
* `git submodule foreach git pull origin master`

### cmake cheat sheet (forestdb):
* ``mkdir -p forestdb/build && cd forestdb/build && cmake .. && make -j && make install && cd ../../``
* ``sudo echo "hi" && mkdir -p forestdb/build && cd forestdb/build && cmake .. && make -j && sudo make install && cd ../../``
* Linux: ``cmake --build . -- -j8``

### cmake cheat sheet (jansson):
* ``mkdir -p jansson/build && cd jansson/build && cmake -DJANSSON_BUILD_SHARED_LIBS=1 .. && make -j && make install && cd ../../``
* ``sudo echo "hi" && mkdir -p jansson/build && cd jansson/build && cmake -DJANSSON_BUILD_SHARED_LIBS=1 .. && make -j && sudo make install && cd ../../``

### windows cmake cheat sheet:
* Windows 32: ``mkdir build && cd build && cmake -G"Visual Studio 12" -DCMAKE_BUILD_TYPE=Release .. && cmake --build .``
* Windows 64: ``mkdir build && cd build && cmake -G"Visual Studio 12 Win64" -DCMAKE_BUILD_TYPE=Release .. && cmake --build .``
