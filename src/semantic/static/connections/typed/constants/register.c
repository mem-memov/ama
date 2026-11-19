/** @file
 * @brief Retain qualified constant transfer obligations independently of destination inference.
 * [CONST-03](../../../../../../spec/language/declarations.md#const-03) forbids using a receiver to infer its namespace.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) owns partial transfer records.
 */
#include <stdint.h>
#include "../../internal.h"

/** Registration preserves the original source/context identity; it never selects
 * a member or enters an ordinary namespace body. Exact repeated transfer facts
 * reuse their descriptor without allocating or supplying a new Type anchor.
 * [CONST-02](../../../../../../spec/language/declarations.md#const-02) fixes qualified namespace spelling.
 */
bool static_typed_defer_constant(StaticConnectionPlan *plan, const StaticResultContext *actual,
    const AstSource *syntax, const StaticTerm *type, bool reference, bool list, bool *resolved) {
    if (syntax->kind != AST_CONSTANT_SOURCE) { *resolved = false; return true; }
    const StaticConstantSource *source;
    if (!static_constant_source(plan->constants, actual, syntax, &source)) {
        (void)static_connection_dependencies(plan); return false;
    }
    for (const StaticTypedConstant *item = plan->first_constant; item; item = item->next) {
        if (item->source == source && item->type == type && item->reference == reference && item->list == list) { *resolved = item->equations_ready; return true; }
    }
    if (plan->constant_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, syntax->span, "Qualified transfer count exceeds representable storage");
    }
    StaticTypedConstant *record = static_connection_allocate(plan, sizeof(*record));
    if (!record) { return false; }
    *record = (StaticTypedConstant){.next = plan->first_constant, .source = source,
        .type = type, .reference = reference, .list = list};
    plan->first_constant = record;
    plan->constant_count++;
    *resolved = false;
    return true;
}
