/** @file
 * @brief Catalog original owning statements independently of runtime Type resolution.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes names before solving Types.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial source metadata.
 */
#include <stdint.h>
#include "internal.h"

/** Borrow one original node from the caller's checked owning traversal. Prepare
 * before linking, retaining exact section role without sorting or name lookup.
 * Each successful append consumes one finite syntax owner; failure allocates no
 * diagnostic payload and leaves prior catalog records/count unchanged.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) preserves source classification provenance.
 */
bool static_statement_append(StaticInventory *inventory, StaticModule *module,
    const AstNode *syntax, StaticContext context) {
    if (module->statement_count == SIZE_MAX) { return static_memory_failure(inventory); }
    StaticStatement *statement = static_allocate(inventory, sizeof(*statement));
    if (statement == NULL) { return false; }
    *statement = (StaticStatement){.syntax = syntax, .context = context};
    if (module->last_statement) { module->last_statement->next = statement; }
    else { module->first_statement = statement; }
    module->last_statement = statement;
    module->statement_count++;
    return true;
}
