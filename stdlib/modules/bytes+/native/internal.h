/** @file
 * @brief Keep mutable share accounting separate from immutable binary value bytes.
 * [PROTO-06](../../../../spec/implementation/native-interface.md#proto-06)
 * requires nonwrapping share acquisition before publication. One conbytes has
 * one executor; these backings are never a cross-conbytes transport mechanism.
 */
#ifndef AMA_BYTES_NATIVE_INTERNAL_H
#define AMA_BYTES_NATIVE_INTERNAL_H
#include "../../bytes.h"
#include <stdint.h>
/** Conbytes-local counted resource; published length/bytes never change. Only
 * share acquisition/finalization changes shares while the backing is alive.
 */
struct AMA_NATIVE(Backing) { size_t shares, length; unsigned char bytes[]; };
#endif
