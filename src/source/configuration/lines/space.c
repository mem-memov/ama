/**
 * @file
 * @brief Classify boundary whitespace without locale or platform substitutions.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) forbids padded INI lines.
 * Unicode 17's [White_Space property](https://www.unicode.org/Public/17.0.0/ucd/PropList.txt)
 * supplies the exact scalar set; the language's pinned identifier edition is
 * defined by [SRC-03](../../../../spec/language/source-and-names.md#src-03).
 */
#include "internal.h"

bool configuration_space(uint32_t scalar) {
    return (scalar >= 0x0009 && scalar <= 0x000d) || scalar == 0x0020
        || scalar == 0x0085 || scalar == 0x00a0 || scalar == 0x1680
        || (scalar >= 0x2000 && scalar <= 0x200a)
        || scalar == 0x2028 || scalar == 0x2029 || scalar == 0x202f
        || scalar == 0x205f || scalar == 0x3000;
}
