VERSION = 0.0.1
LIBPATH =
CC = clang
CXX = clang++
CFLAGS = -Wall -Wpedantic -g -std=c99
LIBS = -lboost_filesystem -lboost_system -lsnappy
SERVER_LIBS =
PRODUCER_CLIENT_LIBS = -lforestdb -ljansson
CONSUMER_CLIENT_LIBS = -lforestdb -ljansson
TEST_BUFFER_LIBS =
INCLUDE_PATHS = -I ./src -I /usr/local/include -I ./forestdb/include/libforestdb
TEST_INCLUDE_PATHS = $(INCLUDE_PATHS) -I ./tests

MKDIR_P = mkdir -p
BINDIR = bin
BINDIR_TESTS = bin-tests
LIBBASE = kayos

ifdef DEBUG
	CFLAGS += -DDEBUG
else
	CFLAGS += -O3
endif

LIBNAME = $(DLL_PREFIX)$(LIBBASE)$(DLL_SUFFIX)$(DLL_EXTENSION)

MASTER_HEADERS = src/buffer.h \
	src/io.h \
	src/utils.h

SERVER_HEADERS = $(MASTER_HEADERS)

PRODUCER_CLIENT_HEADERS = $(MASTER_HEADERS) \
	src/http.h \
	src/producer_client.h \
	src/kayos_common.h

CONSUMER_CLIENT_HEADERS = $(MASTER_HEADERS) \
	src/http.h \
	src/consumer_client.h \
	src/kayos_common.h

# core binaries
DLL_OBJS = $(PLAF_DLL_OBJS) \
	src/buffer.o \
	src/io.o \
	src/utils.o

SERVER_OBJS = $(DLL_OBJS) \
	src/kayos_server_main.o

PRODUCER_CLIENT_OBJS = $(DLL_OBJS) \
	src/http.o \
	src/kayos_common.o \
	src/consumer_client.o \
	src/producer_client.o \
	src/kayos_producer_client_main.o

CONSUMER_CLIENT_OBJS = $(DLL_OBJS) \
	src/http.o \
	src/kayos_common.o \
	src/consumer_client.o \
	src/producer_client.o \
	src/kayos_consumer_client_main.o

# test binaries
TEST_BUFFER_OBJS = $(DLL_OBJS) \
	tests/test_buffer.o

.PHONY: kayos-server kayos-producer-client kayos-consumer-client test-buffer clean directories

ALL = directories kayos-server kayos-producer-client kayos-consumer-client test-buffer

default:
	$(MAKE) $(ALL)

kayos-server: $(SERVER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-server $(CFLAGS) $(INCLUDE_PATHS) $(SERVER_LIBS) $(SERVER_OBJS)

kayos-producer-client: $(PRODUCER_CLIENT_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-producer-client $(CFLAGS) $(INCLUDE_PATHS) $(PRODUCER_CLIENT_LIBS) $(PRODUCER_CLIENT_OBJS)

kayos-consumer-client: $(CONSUMER_CLIENT_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-consumer-client $(CFLAGS) $(INCLUDE_PATHS) $(CONSUMER_CLIENT_LIBS) $(CONSUMER_CLIENT_OBJS)

test-buffer: $(TEST_BUFFER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR_TESTS)/test-buffer $(CFLAGS) $(TEST_INCLUDE_PATHS) $(TEST_BUFFER_LIBS) $(TEST_BUFFER_OBJS)

%.o: %.c
	$(TOOLCHAIN_PREFIX)$(CC) $(CFLAGS) $(INCLUDE_PATHS) -c -o $@ $<

directories: $(BINDIR) $(BINDIR_TESTS)

$(BINDIR):
	$(MKDIR_P) $(BINDIR)

$(BINDIR_TESTS):
	$(MKDIR_P) $(BINDIR_TESTS)

clean:
	rm -f src/*.o
	rm -f $(BINDIR)/*
	rm -f $(BINDIR_TESTS)/*
