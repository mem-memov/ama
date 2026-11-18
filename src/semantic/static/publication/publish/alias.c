/** @file
 * @brief Intern immutable canonical static aliases by exact lexical environment and symbol.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids alias rebinding.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial descriptors.
 */
#include <stdint.h>
#include "../internal.h"

/** Closed canonical values make equality exact pointer identity within the
 * borrowed term store. Equal aliases do not manufacture new nominal identities.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves nominal results.
 */
bool static_publication_alias(StaticPublicationPlan *plan, const StaticTermEnvironment *environment,
    const StaticSymbol *symbol, const StaticTerm *value) {
    for (const StaticPublishedAlias *alias = plan->first_alias; alias; alias = alias->next) {
        if (alias->environment != environment || alias->symbol != symbol) { continue; }
        if (alias->value == value) { return true; }
        return static_publication_fail(plan, STATIC_PUBLICATION_CONFLICT, symbol->site,
            "Immutable static alias has conflicting canonical values");
    }
    if (plan->alias_count == SIZE_MAX) {
        return static_publication_fail(plan, STATIC_PUBLICATION_OUT_OF_MEMORY, symbol->site,
            "Static alias publication count exceeds representable storage");
    }
    StaticPublishedAlias *alias = static_publication_allocate(plan, sizeof(*alias));
    if (alias == NULL) { return false; }
    *alias = (StaticPublishedAlias){.environment = environment, .symbol = symbol, .value = value, .next = plan->first_alias};
    plan->first_alias = alias;
    plan->alias_count++;
    return true;
}
