VERSION = 0.0.1
LIBPATH =
CC = clang
CXX = clang++
CFLAGS = -Wall -Wpedantic -g
LIBS = -lboost_filesystem -lboost_system -lsnappy
SERVER_LIBS =
WRITER_LIBS = -lforestdb
CLIENT_LIBS =
INCLUDE_PATHS = -I ./src -I /usr/local/include

MKDIR_P = mkdir -p
BINDIR = bin
BINDIR_TESTS = bin-test
LIBBASE = kayos

ifdef DEBUG
	CFLAGS += -DKAYOS_DEBUG
else
	CFLAGS += -O3
endif

LIBNAME = $(DLL_PREFIX)$(LIBBASE)$(DLL_SUFFIX)$(DLL_EXTENSION)

DLL_OBJS = $(PLAF_DLL_OBJS) \
	src/io.o \
	src/utils.o

MASTER_HEADERS = src/portable_endian.h \
	src/io.h \
	src/utils.h

SERVER_HEADERS = $(HEADERS) \
	src/kayos_server.h

WRITER_HEADERS = $(HEADERS) \
	src/kayos_writer.h

CLIENT_HEADERS = $(HEADERS) \
	src/kayos_client.h

SERVER_OBJS = $(DLL_OBJS) \
	src/kayos_server.o

WRITER_OBJS = $(DLL_OBJS) \
	src/kayos_writer.o

CLIENT_OBJS = $(DLL_OBJS) \
	src/kayos_client.o

.PHONY: kayos-server kayos-writer kayos-client clean directories

ALL = directories kayos-server kayos-writer kayos-client

default:
	$(MAKE) $(ALL)

kayos-server: $(SERVER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-server $(CFLAGS) $(INCLUDE_PATHS) $(SERVER_LIBS) $(SERVER_OBJS)

kayos-writer: $(WRITER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-writer $(CFLAGS) $(INCLUDE_PATHS) $(WRITER_LIBS) $(WRITER_OBJS)

kayos-client: $(CLIENT_OBJS)
	$(TOOLCHAIN_PREFIX)$(CC) -o $(BINDIR)/kayos-client $(CFLAGS) $(INCLUDE_PATHS) $(CLIENT_LIBS) $(CLIENT_OBJS)

src/kayos_master.h.gch: src/kayos_master.h $(MASTER_HEADERS)
	$(TOOLCHAIN_PREFIX)$(CC) -c -x c++-header $(CFLAGS) -o $@ $<

#%.o: %.cc src/kayos_master.h.gch
#	$(TOOLCHAIN_PREFIX)$(CXX) $(CFLAGS) $(INCLUDE_PATHS) -c -o $@ $<

%.o: %.c src/kayos_master.h.gch
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
