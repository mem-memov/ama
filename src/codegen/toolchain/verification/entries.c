/**
 * @file
 * @brief Reject surviving native and Ama control-entry functions before linking.
 *
 * [STACK-01](../../../../spec/implementation/compiler.md#stack-01) requires Ama
 * bodies to remain labels and native entry calls to disappear. Under
 * [STACK-04](../../../../spec/implementation/compiler.md#stack-04), check exact
 * registered native names and IR selector names, never arbitrary helper prefixes.
 */
#include <limits.h>
#include <string.h>
#include "../internal.h"
#include "native/toolchain/symbols.h"

/** Format the emitter's exact selector name and inspect its complete symbol family.
 * Decimal digit count never exceeds the size_t bit count, so this fixed buffer
 * includes every possible selector and its terminator without a guessed limit.
 */
static AmaSymbolCheck control_entry(AmaByteView listing, size_t selector) {
    char name[sizeof("ama_label_") + sizeof(size_t) * CHAR_BIT];
    int length = snprintf(name, sizeof(name), "ama_label_%zu", selector);
    if (length <= 0 || (size_t)length >= sizeof(name)) { return AMA_SYMBOLS_MALFORMED; }
    AmaByteView entry = {(const unsigned char *)name, (size_t)length};
    return ama_check_entry_symbols(listing, &entry, 1);
}

/** Inspect all resolved entry identities, permitting unrelated ordinary native helpers.
 * The finite type/control inventories come from checked IR. Native spelling storage
 * belongs to scaffold scratch; selector spelling borrows only the synchronous call.
 * Any failure prevents executable publication; this phase never executes the object.
 */
bool codegen_verify_entries(NativeScaffold *scaffold, AmaByteView listing) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) {
            const char *name = scaffold_join(scaffold, type->native_prefix->internal_name, "", "_entry");
            if (name == NULL) { return false; }
            AmaByteView entry = {(const unsigned char *)name, strlen(name)};
            if (ama_check_entry_symbols(listing, &entry, 1) != AMA_SYMBOLS_CLEAR) {
                return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE,
                    "A native entry was not inlined; no executable was published", type->body_path);
            }
        }
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            if (control_entry(listing, node->selector) != AMA_SYMBOLS_CLEAR) {
                return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE,
                    "An Ama control entry survived as a C function; no executable was published", NULL);
            }
        }
    }
    return true;
}
