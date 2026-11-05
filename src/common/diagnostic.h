#ifndef AMA_COMMON_DIAGNOSTIC_H
#define AMA_COMMON_DIAGNOSTIC_H

#include <stdint.h>

_Noreturn void fatal_at(const char* module, uint32_t line, const char* fmt, ...);

#endif
