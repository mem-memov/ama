/** @file
 * @brief Publish complete source binding records in deterministic original declaration order.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) gives inferred fields original receiver sites.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include <stdint.h>
#include "../../internal.h"

/** Prepare before linking; insertion preserves site order even when explicit
 * fields were hoisted before earlier source-inferred fields. Pointers stay stable.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes construction order separately from execution.
 */
bool static_runtime_add_field(StaticConnectionPlan *plan, StaticRuntimeModule *module,
    StaticRuntimeField value, const StaticRuntimeField **output) {
    if (module->field_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, value.site, "Runtime field count exceeds representable storage");
    }
    StaticRuntimeField *field = static_connection_allocate(plan, sizeof(*field));
    if (field == NULL) { return false; }
    *field = value;
    StaticRuntimeField **link = &module->first_field;
    while (*link && (*link)->site.start <= field->site.start) { link = &(*link)->next; }
    field->next = *link;
    *link = field;
    if (field->next == NULL) { module->last_field = field; }
    module->field_count++;
    *output = field;
    return true;
}

/** Keep the later declaration primary and the earlier introducing site related.
 * No payload allocation is needed, including a hoisted future explicit duplicate.
 * [NAME-04](../../../../../../spec/language/declarations.md#name-04) rejects duplicate base names.
 */
bool static_runtime_duplicate(StaticConnectionPlan *plan, const StaticRuntimeModule *module, SourceSpan later, SourceSpan earlier) {
    if (plan->status != STATIC_CONNECTION_OK) { return false; }
    (void)static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, later, "Runtime base name is declared more than once");
    plan->related = (DiagnosticRelated){.location = {.span = earlier}, .role = "earlier runtime declaration",
        .name = module->source->definition->source->logical_path};
    plan->error.related = &plan->related;
    plan->error.related_count = 1;
    return false;
}
