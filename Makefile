VERSION = 0.0.1
LIBPATH =
CXX = clang++
CFLAGS = -Wall -g -std=c++11
LIBS = -lboost_filesystem -lboost_system -lsnappy -lforestdb -lzmq
SERVER_LIBS = -lforestdb -lzmq
CLIENT_LIBS = -lzmq

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
	src/utils.o

MASTER_HEADERS = src/portable_endian.h \
	src/endian_types.h \
	src/utils.h

SERVER_HEADERS = $(HEADERS) \
	src/kayos_server.h

CLIENT_HEADERS = $(HEADERS) \
	src/kayos_client.h

SERVER_OBJS = $(DLL_OBJS) \
	src/kayos_server.o

CLIENT_OBJS = $(DLL_OBJS) \
	src/kayos_client.o

.PHONY: kayos-server kayos-client clean directories

ALL = directories kayos-server kayos-client

default:
	$(MAKE) $(ALL)

kayos-client: $(CLIENT_OBJS)
	$(TOOLCHAIN_PREFIX)$(CXX) -o $(BINDIR)/kayos-client $(CFLAGS) $(CLIENT_LIBS) $(CLIENT_OBJS)

kayos-server: $(SERVER_OBJS)
	$(TOOLCHAIN_PREFIX)$(CXX) -o $(BINDIR)/kayos-server $(CFLAGS) $(SERVER_LIBS) $(SERVER_OBJS)

src/kayos_master.h.gch: src/kayos_master.h $(MASTER_HEADERS)
	$(TOOLCHAIN_PREFIX)$(CXX) -c -x c++-header $(CFLAGS) -o $@ $<

%.o: %.cc src/kayos_master.h.gch
	$(TOOLCHAIN_PREFIX)$(CXX) $(CFLAGS) -I./src -c -o $@ $<

directories: $(BINDIR) $(BINDIR_TESTS)

$(BINDIR):
	$(MKDIR_P) $(BINDIR)

$(BINDIR_TESTS):
	$(MKDIR_P) $(BINDIR_TESTS)

clean:
	rm -f src/*.o
	rm -f $(BINDIR)/*
	rm -f $(BINDIR_TESTS)/*
