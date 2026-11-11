#ifndef AMA_NATIVE_TOOLCHAIN_GCC_H
#define AMA_NATIVE_TOOLCHAIN_GCC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ != 15 || __GNUC_MINOR__ != 2 || __GNUC_PATCHLEVEL__ != 0
#error "Ama native backend requires verified GCC 15.2.0"
#endif
#if !defined(__linux__) || !defined(__x86_64__) || !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "Ama native backend requires C11 on x86_64 Linux"
#endif

_Static_assert(sizeof(void *) == 8 && sizeof(size_t) == 8, "Ama requires LP64");
_Static_assert(sizeof(uint32_t) == 4 && sizeof(uint64_t) == 8, "Invalid integer layout");
_Static_assert(sizeof(bool) == 1, "Invalid bool representation");

/* Only direct entry calls are supported; taking entry addresses is forbidden. */
#define AMA_NATIVE_ENTRY static inline __attribute__((always_inline))

#endif
