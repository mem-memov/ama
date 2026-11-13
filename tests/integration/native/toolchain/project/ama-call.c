/**
 * @file
 * @brief Deliberately dispatch an Ama-shaped body through a retained C function.
 *
 * The toolchain acceptance test must reject the exact registered control entry
 * at both optimization levels even though this ordinary C executable could run.
 */
#include "native/toolchain/gcc.h"

/** Deliberately introduce the forbidden C activation at an Ama control-entry name. */
__attribute__((noinline, noipa)) size_t ama_label_1(size_t value) {
    return value + 1;
}

/** Call the forbidden entry so symbol rejection, rather than execution, decides acceptance. */
int main(void) {
    return ama_label_1(3) == 4 ? 0 : 1;
}
