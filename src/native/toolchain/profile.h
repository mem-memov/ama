/**
 * @file
 * @brief Share the verified native compilation profile without fixing optimization level.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04) requires one
 * checked toolchain contract. Real project compilation, its compilation database
 * and acceptance probes expand this exact argv list. Append it after user options
 * so required diagnostics and non-LTO inspection cannot be disabled accidentally.
 * Optimization is selected separately; both O0 and O2 are verified. This header
 * does not restrict the compiler implementation's own host build.
 * [STACK-02](../../../spec/implementation/compiler.md#stack-02) permits ordinary
 * helper calls and optional inline hints. Global Winline is not requested;
 * always_inline enforcement and exact symbol checks verify native entry boundaries.
 */
#ifndef AMA_NATIVE_TOOLCHAIN_PROFILE_H
#define AMA_NATIVE_TOOLCHAIN_PROFILE_H

/** Required individual argv elements, never one shell command or combined option.
 * Non-whole-program compilation preserves externally visible inspection evidence.
 */
#define AMA_TOOLCHAIN_REQUIRED_FLAGS \
    "-std=c11", "-pedantic-errors", "-Wall", "-Wextra", "-Werror", \
    "-Werror=attributes", "-fno-lto", "-fno-whole-program"

#endif
