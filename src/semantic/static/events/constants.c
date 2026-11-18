/** @file
 * @brief Catalog original qualified value sources without treating them as static names.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) keeps namespace/member syntax.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial inventory records.
 */
#include <stdint.h>
#include "internal.h"

/** Append only original constant sources. No name binding, Type application or
 * value evaluation occurs; count/allocation refusal leaves the chain unchanged.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) preserves original source identity.
 */
static bool append(StaticInventory *inventory, StaticModule *module, const AstNode *node, const AstSource *source) {
    if (source == NULL || source->kind != AST_CONSTANT_SOURCE) { return true; }
    if (module->constant_source_count == SIZE_MAX) { return static_memory_failure(inventory); }
    StaticConstantOccurrence *site = static_allocate(inventory, sizeof(*site));
    if (site == NULL) { return false; }
    *site = (StaticConstantOccurrence){.source = source, .statement = node};
    if (module->last_constant_source) { module->last_constant_source->next = site; }
    else { module->first_constant_source = site; }
    module->last_constant_source = site;
    module->constant_source_count++;
    return true;
}

/** Each source belongs to exactly one initializer, assignment or input argument.
 * Nested owning statements are handled by the parent iterative AST walk, so
 * this operation consumes finite local edges and uses no recursive host calls.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) distinguishes sources from type heads.
 */
bool static_constant_occurrences(StaticInventory *inventory, StaticModule *module, const AstNode *node) {
    if (node->kind == AST_DECLARATION) {
        for (const AstInitializer *item = node->as.declaration.first; item; item = item->next) {
            if (!append(inventory, module, node, item->source)) { return false; }
        }
    } else if (node->kind == AST_ASSIGNMENT) {
        return append(inventory, module, node, node->as.assignment.source);
    } else if (node->kind == AST_CALL) {
        for (const AstArgument *item = node->as.call.first; item; item = item->next) {
            if (!append(inventory, module, node, item->source)) { return false; }
        }
    }
    return true;
}
