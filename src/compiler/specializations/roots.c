/** @file
 * @brief Register only explicit original ordinary compilation roots with checked creation provenance.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) forbids arbitrary inference/default roots.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves original failed selections.
 * [TPL-14](../../../spec/language/static-results.md#tpl-14) excludes markers from runtime binding,
 * instance closure and executable Type metadata.
 */
#include "internal.h"

/** Exact inventory membership precedes module dereference. A static producer
 * cannot masquerade as an ordinary executable Type or acquire a default instance.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) keeps producer graphs compile-time only.
 */
bool compiler_specialization_roots(CompilerSpecialization *analysis, const ProjectModule *const *roots, size_t count) {
    if (count && !roots) {
        analysis->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH, .message = "Missing selected specialization roots"};
        return false;
    }
    for (size_t index = 0; index < count; index++) {
        const StaticModule *module = static_module_find(analysis->inventory, roots[index]);
        if (!module || module->definition->syntax.sections.kind == MODULE_STATIC
            || module->definition->syntax.sections.kind == MODULE_MARKER) {
            analysis->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
                .message = "Selected root must be an original ordinary module",
                .primary = {.span = module ? module->definition->syntax.sections.graph.marker->code : (SourceSpan){0}}};
            return false;
        }
        const StaticTermEnvironment *environment;
        if (!static_term_environment(&analysis->terms, module, NULL, &environment)) {
            analysis->error = analysis->terms.error;
            return false;
        }
        const StaticCreationPath *path;
        if (!static_creation_root(&analysis->guards, environment, &path)) {
            analysis->error = analysis->guards.error;
            return false;
        }
        const StaticResultContext *context;
        if (!static_result_context(&analysis->results, path, &context)) {
            analysis->error = analysis->results.error;
            return false;
        }
    }
    return true;
}
