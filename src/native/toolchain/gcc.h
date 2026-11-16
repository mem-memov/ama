/**
 * @file
 * @brief Enforce the verified GCC, C11 and x86_64 Linux representation assumptions.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * Always-inline entry declarations and target assertions are part of the supported backend contract, not portable C ABI promises.
 */
#ifndef AMA_NATIVE_TOOLCHAIN_GCC_H
#define AMA_NATIVE_TOOLCHAIN_GCC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ != 15 || \
    __GNUC_MINOR__ != 2 || __GNUC_PATCHLEVEL__ != 0
#error "Ama native backend requires verified GCC 15.2.0"
#endif
#if !defined(__linux__) || !defined(__x86_64__) || !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "Ama native backend requires C11 on x86_64 Linux"
#endif

_Static_assert(sizeof(void *) == 8 && sizeof(size_t) == 8, "Ama requires LP64");
_Static_assert(sizeof(uint32_t) == 4 && sizeof(uint64_t) == 8, "Invalid integer layout");
_Static_assert(sizeof(bool) == 1, "Invalid bool representation");

/* Only direct entry calls are supported; taking entry addresses is forbidden. */
/** Required direct-entry inlining declaration under the verified backend.
 * Taking native entry addresses is outside this contract; ordinary C helpers
 * may still call/return/ recurse on the C stack under STACK-02.
 * Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
#define AMA_NATIVE_ENTRY static inline __attribute__((always_inline))

#endif
