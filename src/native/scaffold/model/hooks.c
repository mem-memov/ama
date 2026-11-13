/**
 * @file
 * @brief Validate compatible shared native helper roles before emitting C declarations.
 *
 * [PROTO-02](../../../../spec/implementation/native-interface.md#proto-02)
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03) gives
 * exact ..integer from_size its separate incompatible signature, never an ABI slot.
 * A single + and - helper may share its compatible signature; incompatible roles and reserved generated names fail.
 */
#include <string.h>
#include "native/scaffold/internal.h"

/** Compare complete validated native identifier byte spans, not hashes or declaration order.
 */
static bool same_name(SourceSpan left, SourceSpan right) {
    size_t length = left.end - left.start;
    return length == right.end - right.start
        && memcmp(source_bytes(left.source) + left.start, source_bytes(right.source) + right.start, length) == 0;
}

bool scaffold_hook_first(const ScaffoldType *type, AbiOperation operation) {
    const AbiBinding *bindings = type->plan->type->abi.bindings;
    for (AbiOperation previous = ABI_DEFAULT; previous < operation; previous++) {
        if (bindings[previous].present && same_name(bindings[previous].function, bindings[operation].function)) {
            return false;
        }
    }
    return true;
}

bool scaffold_hooks(NativeScaffold *scaffold, const ScaffoldType *type) {
    if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) {
        return true;
    }
    const AbiBinding *bindings = type->plan->type->abi.bindings;
    for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
        if (!bindings[operation].present) {
            continue;
        }
        SourceSpan name = bindings[operation].function;
        if (layout_text_is(name, "entry") || layout_text_is(name, "State") || layout_text_is(name, "EntrySignature")) {
            return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH,
                "ABI helper conflicts with a generated native role: entry, State or EntrySignature", source_path(name.source));
        }
        if (scaffold_size_bridge(type) && layout_text_is(name, "from_size")) {
            return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH,
                "The fixed standard integer from_size signature cannot implement an ABI-table slot", source_path(name.source));
        }
        for (AbiOperation previous = ABI_DEFAULT; previous < operation; previous++) {
            bool compatible = previous == ABI_DEFAULT && operation == ABI_DESTROY;
            if (bindings[previous].present && same_name(bindings[previous].function, name) && !compatible) {
                return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH,
                    "One native helper name cannot implement incompatible C signatures", source_path(name.source));
            }
        }
    }
    return true;
}
