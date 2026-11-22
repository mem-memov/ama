/** @file
 * @brief Classify an independently checked entry by executable proof or exact open root key.
 * [TPL-03](../../../spec/language/templates.md#tpl-03) permits internal evidence
 * to determine parameters; [PKG-08](../../../spec/project/packages.md#pkg-08)
 * therefore cannot equate original formal count with executable test eligibility.
 * Only exact fixed-point root-key underdetermination denotes openness.
 * [TPL-10](../../../spec/language/templates.md#tpl-10)
 * Selected-root provenance separates root parameters from unrelated pending uses.
 * [TPL-11](../../../spec/language/templates.md#tpl-11)
 * [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04) separates
 * an executable plan certificate from physical input freshness.
 */
#ifndef AMA_DELIVERY_SOURCE_ENTRY_H
#define AMA_DELIVERY_SOURCE_ENTRY_H
#include "compiler/frontend.h"
/** Open means the original selected root key itself remains pending at the
 * exact equation fixed point, not an arbitrary compilation failure under
 * [TPL-10](../../../spec/language/templates.md#tpl-10).
 */
typedef enum { DELIVERY_ENTRY_EXECUTABLE, DELIVERY_ENTRY_OPEN, DELIVERY_ENTRY_INVALID } DeliveryEntryState;
/** Observe one normal independently attempted executable check. Completed plan
 * success certifies eligibility, independently of filesystem input sealing.
 * Captured probes retain composite frontend.valid false under
 * [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04).
 * A typed unresolved closure with an exact pending
 * original generic root key classifies an open helper. Every other failure,
 * including closed-root unknown fields, syntax/type/capability/native failures,
 * remains invalid with the frontend's original diagnostic. No diagnostic strings,
 * trial type arguments, filenames or default types select the result. Read-only,
 * O(registered uses) time, no allocation; source/frontend owners remain borrowed.
 * [PKG-08](../../../spec/project/packages.md#pkg-08) requires this semantic
 * distinction; [TPL-11](../../../spec/language/templates.md#tpl-11) preserves
 * original selected-root provenance independently of nested use paths.
 */
DeliveryEntryState delivery_source_entry_state(const CompilerFrontend *);
#endif
