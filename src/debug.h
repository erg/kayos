#ifndef KAYOS_SRC_DEBUG_H_
#define KAYOS_SRC_DEBUG_H_

#include <stdio.h>
#include <stdarg.h>
#include "hexdump.h"

//http://stackoverflow.com/questions/5588855/standard-alternative-to-gccs-va-args-trick
//http://stackoverflow.com/questions/4054085/appending-to-va-args

#define VA_COMMA(...) GET_6TH_ARG(,##__VA_ARGS__,COMMA,COMMA,COMMA,COMMA,)
#define GET_6TH_ARG(a1,a2,a3,a4,a5,a6,...) a6
#define COMMA ,

#define debug_print(fmt, ...) \
  do {\
	if(KAYOS_DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt "%s", __FILE__, __LINE__, __func__, __VA_ARGS__ VA_COMMA(__VA_ARGS__) "");\
  } while(0)

#define debug_hexdump(name, buffer, nbytes) \
  do {\
    char debug_buffer[1024];\
    snprintf(debug_buffer, sizeof(debug_buffer), "%s:%d:%s(): %s", __FILE__, __LINE__, __func__, name);\
    if(KAYOS_DEBUG) named_hexdump(stderr, debug_buffer, buffer, nbytes);\
  } while(0)

#endif
