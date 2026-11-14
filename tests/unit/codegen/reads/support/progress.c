/** @file
 * @brief Expose retained-read value preservation and successful output prefixes through ordinary native destruction effects.
 */
/** Trace this known value representation's logical number after checking its final native address. */
static const char *retained_value_trace(const ScaffoldType *type, AbiOperation operation) {
    if (operation != ABI_DESTROY || strcmp(type->plan->type->module->source->logical_path, ".value") != 0) { return NULL; }
    return " { assert(state->address==state); return printf(\"dropvalue=%zu\\n\",state->number)<0 ? ama_status_make(AMA_IO_FAILURE,NULL,0) : ama_status_success(); }\n";
}

/** Reading keeps the producer's body-mutated result, commits the first transfer, then terminates on the next conversion error without rerunning its body or refusal handler. */
static void it_keeps_retained_results_and_the_successful_prefix_when_a_later_output_fails(void) {
    directory("generated/project");
    file("generated/project/producer", "@>\nfirst\n.value\n |23\nsecond\n.value\n |99\n@!\nchanged\n.value\n |31\nprobe\n.worker\nfirst\n changed\nprobe !\n");
    native_hook_override = retained_value_trace;
    execute_project("@!\nsource\n.producer\nearlier\n.value\n |3333\nbadindex\n.value\n |9999\nitems[]\n.value\n |1\nsource !\nsource\n first >> earlier\n second >> items[badindex]\n:\n source !\n",
        "dropvalue=23\ndropvalue=99\ndropvalue=23\ndropvalue=7\ndropvalue=7\ndropvalue=99\n7:1:0\ndropvalue=3333\n"
        "dropvalue=1\ndropvalue=9999\ndropvalue=31\ndropvalue=99\ndropvalue=8\ndropvalue=7\ndropvalue=31\ndropvalue=99\ndropvalue=31\n", false);
    expect_diagnostic("Index decoder failed");
    native_hook_override = NULL;
}
