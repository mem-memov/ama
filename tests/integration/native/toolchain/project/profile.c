/**
 * @file
 * @brief Require the supported compiler, target and language-mode header guards.
 *
 * Negative acceptance cases deliberately override one required configuration after
 * the profile flags and require the corresponding explicit header diagnostic.
 */
#include "native/toolchain/gcc.h"

/** Compile only under the verified backend profile; unsupported probes never execute. */
int main(void) {
    return 0;
}
