/**
 * @file
 * @brief Check native identifier scalars against exact supported C11 Annex-D ranges.
 *
 * [PROTO-11](../../../../spec/implementation/native-interface.md#proto-11)
 * This fixed finite-domain test is separate from pinned Ama Unicode XID properties.
 */
#include <stdint.h>
#include "../internal.h"

/* C11 Annex D.1/D.2, independent of Ama's pinned XID grammar. */
bool abi_identifier_scalar(uint32_t scalar, bool initial) {
    static const uint32_t ranges[][2] = {
        {0xa8, 0xa8}, {0xaa, 0xaa}, {0xad, 0xad}, {0xaf, 0xaf},
        {0xb2, 0xb5}, {0xb7, 0xba}, {0xbc, 0xbe}, {0xc0, 0xd6},
        {0xd8, 0xf6}, {0xf8, 0xff}, {0x100, 0x167f}, {0x1681, 0x180d},
        {0x180f, 0x1fff}, {0x200b, 0x200d}, {0x202a, 0x202e},
        {0x203f, 0x2040}, {0x2054, 0x2054}, {0x2060, 0x206f},
        {0x2070, 0x218f}, {0x2460, 0x24ff}, {0x2776, 0x2793},
        {0x2c00, 0x2dff}, {0x2e80, 0x2fff}, {0x3004, 0x3007},
        {0x3021, 0x302f}, {0x3031, 0x303f}, {0x3040, 0xd7ff},
        {0xf900, 0xfd3d}, {0xfd40, 0xfdcf}, {0xfdf0, 0xfe44}, {0xfe47, 0xfffd}
    };
    if (scalar < 128) {
        return scalar == '_' || scalar == '$' || (scalar >= 'a' && scalar <= 'z')
            || (scalar >= 'A' && scalar <= 'Z') || (!initial && scalar >= '0' && scalar <= '9');
    }
    if (initial && ((scalar >= 0x300 && scalar <= 0x36f)
        || (scalar >= 0x1dc0 && scalar <= 0x1dff) || (scalar >= 0x20d0 && scalar <= 0x20ff)
        || (scalar >= 0xfe20 && scalar <= 0xfe2f))) {
        return false;
    }
    if (scalar >= 0x10000) {
        return scalar <= 0xefffd && (scalar & 0xffff) <= 0xfffd;
    }
    for (size_t index = 0; index < sizeof(ranges) / sizeof(ranges[0]); index++) {
        if (scalar >= ranges[index][0] && scalar <= ranges[index][1]) {
            return true;
        }
    }
    return false;
}
