/** @file
 * @brief Check minimum operations against immutable actual constructor metadata.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) retains additional
 * actual residuals and derives requirements from all uses, never the first actual.
 */
#ifndef AMA_STATIC_USAGE_H
#define AMA_STATIC_USAGE_H
#include "../../inventory.h"

/** Pending is missing evidence; a conflict dominates pending in another use.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) forbids default categories.
 */
typedef enum { STATIC_USAGE_SATISFIED, STATIC_USAGE_PENDING, STATIC_USAGE_CONFLICT } StaticUsageStatus;
/** Borrowed original failure location/message; no allocation or retained actual.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) preserves each operation's evidence.
 */
typedef struct { StaticUsageStatus status; SourceSpan site; const char *message; } StaticUsageResult;

/** Check every recorded head use in one scoped bare-forwarding class. Borrow a complete
 * inventory, a member symbol and known/deferred actual metadata. Never mutate
 * actual captures, residuals, source metadata or prior checks. Additional actual
 * slots are legal. Pending never hides a separately known conflict.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) fixes the requirement
 * set from the whole definition independently of actual admission order.
 */
StaticUsageResult static_usage_check(const StaticInventory *, const StaticSymbol *,
    StaticCategory, const StaticSignature *);
#endif
