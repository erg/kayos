#ifndef BTREE_BTREE_ENDIAN_TYPES_H_
#define BTREE_BTREE_ENDIAN_TYPES_H_

#include "portable_endian.h"

typedef uint8_t be_uint8_t;

struct be_uint16_t {
	be_uint16_t() : be_val(0) {}
	be_uint16_t(const uint16_t &val) : be_val(htobe16(val)) {}
	operator uint16_t() const { return be16toh(be_val); }
	uint16_t be_val;
} __attribute__((packed));

struct be_uint32_t {
	be_uint32_t() : be_val(0) {}
	be_uint32_t(const uint32_t &val) : be_val(htobe32(val)) {}
	operator uint32_t() const { return be32toh(be_val); }
	uint32_t be_val;
} __attribute__((packed));

struct be_uint64_t {
	be_uint64_t() : be_val(0) {}
	be_uint64_t(const uint64_t &val) : be_val(htobe64(val)) {}
	operator uint64_t() const { return be64toh(be_val); }
	uint64_t be_val;
} __attribute__((packed));

struct be_int16_t {
	be_int16_t() : be_val(0) {}
	be_int16_t(const int16_t &val) : be_val(htobe16(val)) {}
	operator int16_t() const { return be16toh(be_val); }
	int16_t be_val;
} __attribute__((packed));

struct be_int32_t {
	be_int32_t() : be_val(0) {}
	be_int32_t(const int32_t &val) : be_val(htobe32(val)) {}
	operator int32_t() const { return be32toh(be_val); }
	int32_t be_val;
} __attribute__((packed));

struct be_int64_t {
	be_int64_t() : be_val(0) {}
	be_int64_t(const int64_t &val) : be_val(htobe64(val)) {}
	operator int64_t() const { return be64toh(be_val); }
	int64_t be_val;
} __attribute__((packed));

#endif
