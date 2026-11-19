/** @file
 * @brief Retain actual known constructor interfaces when source signature checks reject an application.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves definition and application provenance.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) uses actual metadata rather than guessed interfaces.
 */
#include <assert.h>
#include "internal.h"

bool static_signature_error(StaticInventory *inventory, SourceSpan site, const char *message,
    const StaticSymbol *subject, const ProjectModule *origin) {
    if (inventory->status != STATIC_OK) { return false; }
    (void)static_invalid(inventory, site, message, subject);
    if (origin) {
        DiagnosticRelated related = {{.span = {origin->source->text, 0, 0}},
            "actual constructor interface", origin->source->logical_path};
        bool added = diagnostic_add_context(&inventory->error, &inventory->error_context, related);
        assert(added);
        (void)added;
    }
    return false;
}
