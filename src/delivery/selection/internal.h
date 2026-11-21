/** @file
 * @brief Separate exact root/component admission from owned selection publication.
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01)
 * forbids static/open roots; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * keeps all failed preparation under the caller's arena.
 */
#ifndef AMA_DELIVERY_SELECTION_INTERNAL_H
#define AMA_DELIVERY_SELECTION_INTERNAL_H
#include "selection.h"
/** Check the valid frontend's actual selected entry and explicit component/mode.
 * No allocation or effects; diagnostic uses actual source evidence when present.
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01)
 * separates root admission from runtime values.
 */
bool delivery_selection_validate(const CompilerFrontend *, const DeliveryRequest *, Diagnostic *);
#endif
