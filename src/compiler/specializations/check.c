/** @file
 * @brief Compose definition-time binding, explicit root registration and complete guarded Type closure.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) separates Type closure from ordinary admission and lowering.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) checks loaded definitions independently of runtime reachability.
 */
#include <assert.h>
#include "internal.h"

/** Stop at the first failed child with all successful/partial owners retained.
 * The root list alone selects actual ordinary bodies; mere source loading never
 * instantiates an open definition. A true result certifies Type closure only.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) preserves explicit selected provenance.
 */
bool compiler_specialization_check(CompilerSpecialization *analysis, const ProjectModule *const *roots, size_t count) {
    assert(!analysis->attempted);
    analysis->attempted = true;
    if (!compiler_specialization_definitions(analysis) || !compiler_specialization_roots(analysis, roots, count)) { return false; }
    if (!static_closure_resolve(&analysis->closure)) {
        analysis->error = analysis->closure.error;
        return false;
    }
    analysis->valid = true;
    return true;
}
