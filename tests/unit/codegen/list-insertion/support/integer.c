/** @file
 * @brief Own a conforming test standard-root integer instead of modifying the library.
 */

static bool insertion_busy_value;

/** Complete only the known authored standard integer fixture after qualification.
 * Native entry/lifecycle names come from the real scaffold model.
 */
static void complete_insertion_native(NativeScaffold *model) {
    bool found = false;
    for (const ScaffoldType *type = model->types; type != NULL; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, "..integer") != 0) { continue; }
        assert(!found);
        found = true;
        const char *path = source_path(type->plan->type->module->source->text);
        assert(strncmp(path, "generated/standard/", sizeof("generated/standard/") - 1) == 0);
        executable_native(type);
    }
    assert(found);
}

/** Preserve the full machine-size range and reserved negative-index refusal. */
static const char *insertion_integer_hook(const ScaffoldType *type, AbiOperation operation) {
    const char *logical = type->plan->type->module->source->logical_path;
    if (insertion_busy_value && strcmp(logical, ".value") == 0 && operation == ABI_DEFAULT) {
        return " { *destination=(AMA_NATIVE(State)){.address=destination}; puts(\"construct\"); return ama_status_make(AMA_BUSY,NULL,0); }\n";
    }
    if (strcmp(logical, "..integer") != 0) { return NULL; }
    if (operation == ABI_INDEX) {
        return " { assert(state->address==state); if(state->negative) return ama_status_make(AMA_INVALID_INDEX,NULL,0); *result=state->number; return ama_status_success(); }\n";
    }
    if (operation == ABI_LITERAL) {
        return " { *destination=(AMA_NATIVE(State)){.address=destination}; size_t first=literal.length!=0 && literal.data[0]=='-' ? 1 : 0;"
            "if(first==literal.length) return ama_status_make(AMA_INVALID_LITERAL,NULL,0);"
            "for(size_t index=first;index<literal.length;index++) { unsigned char byte=literal.data[index];"
            "if(byte<'0'||byte>'9'||destination->number>(SIZE_MAX-(byte-'0'))/10) return ama_status_make(AMA_INVALID_LITERAL,NULL,0);"
            "destination->number=destination->number*10+(byte-'0'); }"
            "destination->negative=first!=0 && destination->number!=0; return ama_status_success(); }\n";
    }
    return NULL;
}

/** Declare every fixed native protocol and the exact typed host-size bridge. */
static void insertion_standard(void) {
    insertion_busy_value = false;
    directory("generated/standard");
    file("generated/standard/integer", "@C\n");
    file("generated/standard/integer.abi", "+ initialize\n- destroy\n| literal\n= clone\n? active\n[] index\n");
    file("generated/standard/integer.c", "/* Completed from this test-owned module's qualified model. */\n");
    file("generated/standard/integer.h", "struct AMA_NATIVE(State) { size_t number; size_t calls; const void *address; bool negative; };\n"
        "static inline AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination,size_t number) {"
        "*destination=(AMA_NATIVE(State)){.number=number,.address=destination}; return ama_status_success(); }\n");
    native_hook_override = insertion_integer_hook;
    native_fixture_completion = complete_insertion_native;
}
