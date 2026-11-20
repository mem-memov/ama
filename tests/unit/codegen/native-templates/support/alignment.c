/** @file
 * @brief Require actual over-aligned native payload addresses in every specialized scalar/list operation.
 * @see [T190 contract](../../../../../docs/implementation/native-template-composition.md)
 */

/** These fixture payloads accept one decimal digit; assertions observe actual
 * compiled C alignment, never a layout guessed by the Ama compiler. */
static const char *aligned_resource_hook(const ScaffoldType *type, AbiOperation operation) {
    if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/resource") != 0) { return NULL; }
    switch (operation) {
    case ABI_DEFAULT:
        return " { assert((uintptr_t)destination%_Alignof(AMA_NATIVE(State))==0); *destination=(AMA_NATIVE(State)){.address=destination}; return ama_status_success(); }\n";
    case ABI_DESTROY:
        return " { assert((uintptr_t)state%_Alignof(AMA_NATIVE(State))==0 && state->address==state); return ama_status_success(); }\n";
    case ABI_LITERAL:
        return " { assert((uintptr_t)destination%_Alignof(AMA_NATIVE(State))==0 && literal.length==1 && literal.data[0]>='0' && literal.data[0]<='9'); *destination=(AMA_NATIVE(State)){.address=destination,.number=literal.data[0]-'0'}; return ama_status_success(); }\n";
    case ABI_CLONE:
        return " { assert((uintptr_t)destination%_Alignof(AMA_NATIVE(State))==0 && (uintptr_t)source%_Alignof(AMA_NATIVE(State))==0 && source->address==source); *destination=*source; destination->address=destination; return ama_status_success(); }\n";
    case ABI_CONDITION:
        return " { assert((uintptr_t)state%_Alignof(AMA_NATIVE(State))==0 && state->address==state); *result=state->number!=0; return ama_status_success(); }\n";
    case ABI_INDEX:
        return " { assert((uintptr_t)state%_Alignof(AMA_NATIVE(State))==0 && state->address==state); *result=state->number; return ama_status_success(); }\n";
    default: assert(false); return NULL;
    }
}

/** Over-alignment propagates through concrete template storage, parameters,
 * retained results, list elements, clone destinations and native typed views. */
static void it_preserves_actual_overaligned_native_layout_after_substitution(void) {
    prepare_capability_sources();
    file("generated/project/packages/test/fixture/v1.0.0/src/resource.h", "struct AMA_NATIVE(State) { _Alignas(64) unsigned char padding[65]; size_t number; size_t calls; const void *address; };\n");
    native_hook_override = aligned_resource_hook;
    execute_project(capability_program, "9:1:0\n", true);
    native_hook_override = NULL;
}
