/** @file
 * @brief Keep mutable share accounting separate from immutable UTF-8 value bytes.
 * [PROTO-06](../../../../spec/implementation/native-interface.md#proto-06)
 * requires nonwrapping share acquisition before publication. One context has
 * one executor; these backings are never a cross-context transport mechanism.
 */
#ifndef AMA_TEXT_NATIVE_INTERNAL_H
#define AMA_TEXT_NATIVE_INTERNAL_H
#include "../../text.h"
#include <stdint.h>
/** Context-local counted resource; published length/bytes never change. Only
 * share acquisition/finalization changes shares while the backing is alive.
 */
struct AMA_NATIVE(Backing) { size_t shares, length; unsigned char bytes[]; };
#endif
