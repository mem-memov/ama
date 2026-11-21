/** @file
 * @brief Publish original field Type associations for exact source environments without inventing roots.
 * [TPL-04](../../../../../../spec/language/templates.md#tpl-04) preserves independently instantiated owner fields.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) separates metadata from actual expansion.
 * [TPL-14](../../../../../../spec/language/static-results.md#tpl-14) excludes markers from runtime binding,
 * instance closure and executable Type metadata.
 */
#include <stdint.h>
#include "../../internal.h"
#include "../../../terms/internal.h"

/** A supplied nominal environment already has an explicit owner expression.
 * Generic environments instead require actual registered provenance. Every
 * matching context has the same exact interned guard owner; no name-based or
 * first-candidate selection supplies a root.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) preserves selected compilation origins.
 */
static bool owner(StaticConnectionPlan *plan, const StaticTermEnvironment *environment, const StaticTerm **output) {
    if (environment->owner_type) { *output = environment->owner_type; return true; }
    const StaticGuardOwner *found = NULL;
    for (const StaticResultContext *context = plan->constants->publications->results->first_context; context; context = context->next) {
        const StaticGuardOwner *candidate = context->creation->owner;
        if (candidate->environment != environment) { continue; }
        if (found && found != candidate) {
            (void)static_connection_fail(plan, STATIC_CONNECTION_INVALID, context->creation->site, "One exact environment has inconsistent actual owner provenance");
            return false;
        }
        found = candidate;
    }
    if (found == NULL) {
        (void)static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Generic typed scope requires an explicitly registered actual root context");
        return false;
    }
    *output = found->value;
    return true;
}

/** Source-only fields introduce no actual context, definition readiness or body
 * equation expansion. All original field/term records remain owned on refusal.
 * [TPL-04](../../../../../../spec/language/templates.md#tpl-04) keeps symbolic scopes distinct until proven equal.
 */
bool static_connection_scope(StaticConnectionPlan *plan, const StaticTermEnvironment *environment, const StaticTypedScope **output) {
    if (!static_connection_dependencies(plan)) { return false; }
    StaticTermStore *terms = plan->constants->publications->results->guards->applications->terms;
    if (output == NULL || !static_term_has_environment(terms, environment) ||
        environment->definition->definition->syntax.sections.kind == MODULE_STATIC
        || environment->definition->definition->syntax.sections.kind == MODULE_MARKER) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Invalid exact typed environment or scope receiver");
    }
    for (const StaticTypedScope *scope = plan->first_scope; scope; scope = scope->next) {
        if (scope->environment == environment) { *output = scope; return true; }
    }
    if (plan->scope_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, (SourceSpan){0}, "Typed environment scope count exceeds representable storage");
    }
    const StaticTerm *value;
    if (!owner(plan, environment, &value)) { return false; }
    const StaticRuntimeModule *runtime;
    if (!static_connection_runtime(plan, environment->definition, &runtime)) { return false; }
    StaticTypedScope *scope = static_connection_allocate(plan, sizeof(*scope));
    if (scope == NULL) { return false; }
    *scope = (StaticTypedScope){.environment = environment, .owner = value, .runtime = runtime};
    if (!static_typed_fields(plan, scope)) { return false; }
    scope->next = plan->first_scope;
    plan->first_scope = scope;
    plan->scope_count++;
    *output = scope;
    return true;
}
