/** @file
 * @brief Own exact destructor traces and typed resource readers for compiled deletion.
 */
static bool trace_resource_destruction;

/** A technical Busy from destruction must not become the method's admission handler. */
static const char *deletion_native_hook(const ScaffoldType *type, AbiOperation operation) {
    if (trace_resource_destruction && operation == ABI_DESTROY
        && strcmp(type->plan->type->module->source->logical_path, ".resource") == 0) {
        return " { assert(state->address==state); printf(\"drop=%zu\\n\",state->number);"
            "return state->number==2 ? ama_status_make(AMA_BUSY,NULL,0) : ama_status_success(); }\n";
    }
    return insertion_integer_hook(type, operation);
}

/** Read the declared native input representation through its actual typed view. */
static bool deletion_native_entry(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".inspect") != 0) { return false; }
    int written = fputs("printf(\"value=%zu\\n\",parameters->AMA_INPUT(0)->payload->number);\n", output);
    assert(written != EOF);
    return true;
}

/** Prepare only owned test roots; maintained library companions are never overwritten. */
static void deletion_standard(void) {
    insertion_standard();
    directory("generated/project");
    native_fixture("resource", "@C\n");
    native_fixture("inspect", "@<\ninput\n.resource\n@C\n");
    trace_resource_destruction = false;
    native_hook_override = deletion_native_hook;
    native_entry_override = deletion_native_entry;
}
