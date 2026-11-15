/** @file
 * @brief Observe real typed task steps and reject queued execution independently of assertions.
 * [ASYNC-07](../../../../../spec/language/async-and-parallelism.md#async-07)
 * requires clearing membership before a task body; this fixture consumes generated native metadata.
 */
#ifndef AMA_TEST_TYPED_SCHEMA_TRACE_C
#define AMA_TEST_TYPED_SCHEMA_TRACE_C
/** A task observes membership after pop and before its body; stale queued=true is a technical test failure. */
static bool typed_schema_trace(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".trace") != 0) { return false; }
    int written = fputs("if(parameters->AMA_INPUT(1)->payload->value) { *status=ama_status_make(AMA_NATIVE_FAILURE,\"Task still queued\",17); return; }\n"
        "printf(\"%zu:%zu\\n\",parameters->AMA_INPUT(0)->payload->value,++state->calls);\n", output);
    assert(written != EOF);
    return true;
}

#endif
