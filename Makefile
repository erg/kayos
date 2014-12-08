VERSION = 0.0.1
LIBPATH =
CC = clang
CXX = clang++
CFLAGS = -Wall -Wpedantic -g -std=c99
LIBS = -lboost_filesystem -lboost_system -lsnappy
INCLUDE_LIB_PATHS = -L /usr/local/lib
SERVER_LIBS =
PRODUCER_LIBS = -lforestdb -ljansson
CONSUMER_LIBS = -lforestdb -ljansson
TEST_BUFFER_LIBS =
TESTER_LIBS = -ljansson
INCLUDE_PATHS = -I ./src -I /usr/local/include -I ./forestdb/include/libforestdb
TEST_INCLUDE_PATHS = $(INCLUDE_PATHS) -I ./tests
TESTER_INCLUDE_PATHS = $(INCLUDE_PATHS) -I ./tester

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
	src/hexdump.h \
	src/io.h \
	src/paths.h

SERVER_HEADERS = $(MASTER_HEADERS)

PRODUCER_HEADERS = $(MASTER_HEADERS) \
	src/both.h \
	src/producer.h

CONSUMER_HEADERS = $(MASTER_HEADERS) \
	src/both.h \
	src/http.h \
	src/consumer.h

# core binaries
DLL_OBJS = $(PLAF_DLL_OBJS) \
	src/buffer.o \
	src/errors.o \
	src/hexdump.o \
	src/io.o

SERVER_OBJS = $(DLL_OBJS) \
	src/paths.o \
	src/kayos_server_main.o

PRODUCER_OBJS = $(DLL_OBJS) \
	src/both.o \
	src/json_utils.o \
	src/paths.o \
	src/producer.o \
	src/producer_http.o \
	src/producer_json.o \
	src/kayos_producer_main.o

CONSUMER_OBJS = $(DLL_OBJS) \
	src/both.o \
	src/json_utils.o \
	src/paths.o \
	src/consumer.o \
	src/consumer_http.o \
	src/consumer_json.o \
	src/kayos_consumer_main.o

# test binaries
TEST_BUFFER_OBJS = $(DLL_OBJS) \
	tests/test_buffer.o

TESTER_OBJS = $(DLL_OBJS) \
	tester/tester.o \
	src/json_utils.o \
	tester/tester_main.o

.PHONY: kayos-server kayos-producer kayos-consumer test-buffer tester clean directories

ALL = directories kayos-server kayos-producer kayos-consumer test-buffer tester

default:
	$(MAKE) $(ALL)

kayos-server: $(SERVER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-server $(CFLAGS) $(INCLUDE_PATHS) $(INCLUDE_LIB_PATHS) $(SERVER_LIBS) $(SERVER_OBJS)

kayos-producer: $(PRODUCER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-producer $(CFLAGS) $(INCLUDE_PATHS) $(INCLUDE_LIB_PATHS) $(PRODUCER_LIBS) $(PRODUCER_OBJS)

kayos-consumer: $(CONSUMER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-consumer $(CFLAGS) $(INCLUDE_PATHS) $(INCLUDE_LIB_PATHS) $(CONSUMER_LIBS) $(CONSUMER_OBJS)

test-buffer: $(TEST_BUFFER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR_TESTS)/test-buffer $(CFLAGS) $(TEST_INCLUDE_PATHS) $(INCLUDE_LIB_PATHS) $(TEST_BUFFER_LIBS) $(TEST_BUFFER_OBJS)

tester: $(TESTER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR_TESTS)/tester $(CFLAGS) $(TESTER_INCLUDE_PATHS) $(INCLUDE_LIB_PATHS) $(TESTER_LIBS) $(TESTER_OBJS)

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
