/** @file
 * @brief Locate a static receipt's original lexical receiver without matching names or changing its definition.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves receiver identity.
 */
#include "../internal.h"

/** Registered occurrences distinguish first alias definitions from later
 * same-spelling equality receivers without textual matching or source guessing.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves original scope.
 */
const StaticSymbol *static_publication_receiver(const StaticResultUse *use, const AstStaticBinding *binding) {
    for (const StaticOccurrence *site = use->source->environment->definition->first_occurrence; site; site = site->next) {
        if (site->expression == use->source->expression && site->receiver == binding) { return site->symbol; }
    }
    return NULL;
}
