/** @file
 * @brief Bind each qualified constant to its exact normalized loading occurrence.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) supplies a
 * normalized namespace/member; [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * preserves the whole original explicit request rather than matching equal text.
 */
#include "../internal.h"

/** Borrow exact descriptor/target pairs after dependency admission, without another
 * normalization, parser split or source allocation. Implicit and differently
 * located edges cannot supply this proof. Failure records the original operand.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) and
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) require this provenance.
 */
bool static_bind_constant_occurrences(StaticInventory *inventory, StaticModule *module) {
    for (StaticConstantOccurrence *site = module->first_constant_source; site; site = site->next) {
        SourceSpan operand = site->source->qualified_path;
        for (const ProjectDependency *edge = module->definition->first; edge; edge = edge->next) {
            if (!edge->implicit && edge->constant_path && edge->requirement.source == operand.source
                && edge->requirement.start == operand.start && edge->requirement.end == operand.end) {
                site->origin = edge->target;
                site->path = edge->constant_path;
                break;
            }
        }
        if (site->origin == NULL || site->path == NULL) {
            return static_invalid(inventory, operand,
                "Qualified constant has no loaded dependency identity", NULL);
        }
    }
    return true;
}
