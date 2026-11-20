/** @file
 * @brief Validate the exact standard integer interface and retain its typed native size-construction obligation.
 * [LIST-04](../../../spec/language/lists.md#list-04) requires ordinary copy,
 * literal and index protocols; [PROTO-03](../../../spec/implementation/native-interface.md#proto-03)
 * fixes from_size without adding a seventh ABI slot or guessing C definitions.
 */
#include "internal.h"
SemanticType *semantic_list_integer(SemanticProgram *program, SourceSpan site) {
    if (program->list_integer != NULL) { return program->list_integer; }
    SemanticType *type = semantic_find_type(program, "//integer");
    if (type == NULL || !type->executable || type->module->syntax.sections.kind != MODULE_NATIVE) {
        semantic_fail(program, site, "List interfaces require the exact native standard module //integer", NULL);
        return NULL;
    }
    const AbiOperation required[] = {ABI_CLONE, ABI_LITERAL, ABI_INDEX};
    for (size_t index = 0; index < sizeof(required) / sizeof(*required); index++) {
        AbiOperation operation = required[index];
        if (!type->abi.bindings[operation].present || (operation == ABI_CLONE && !type->cloneable)) {
            semantic_capability_error(program, type, operation, site, NULL,
                "The standard integer type lacks a required list-interface capability");
            return NULL;
        }
    }
    type->requires_size_bridge = true;
    program->list_integer = type;
    return type;
}
