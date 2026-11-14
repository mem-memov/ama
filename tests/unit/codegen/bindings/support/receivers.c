/** @file
 * @brief Observe placeholder positions, inferred receiver forms and sequential output commits in compiled programs.
 */
/** Empty output consumes its position; first-use receivers infer owned/reference storage from their declared source ports. */
static void it_executes_inferred_owned_reference_receivers_and_output_placeholders(void) {
    execute_project("@!\nfirst\n.worker\nprobe\n.worker\nfirst !\n < |5\n >\n > last\nprobe !\n < last\n", "5:1:0\n99:1:0\n", true);
    directory("generated/project");
    file("generated/project/producer", "@>\nme&\n.producer\n &\n@!\nprobe\n.worker\nprobe !\n");
    execute_project("@!\nsender\n.producer\nsender !\n > received&\nreceived& !\nsender !\n", "7:1:0\n7:2:0\n7:3:0\n", true);
}

/** A previous output updates the actual index used for the next receiver; the final read distinguishes it from an eagerly selected old element. */
static void it_resolves_each_output_receiver_after_the_previous_transfer_commits(void) {
    execute_project("@!\nfirst\n.worker\nprobe\n.worker\nindex\n.value\n |0\nitems[]\n.value\n |4\n |5\nfirst !\n input < |0\n output > index\n other > items[index]\nprobe !\n < items[index]\n", "0:1:0\n99:1:0\n", true);
}

/** Destructor effects expose the retained successful prefix before terminal cleanup; no mutable diagnostic pointer escapes a context lease. */
static const char *value_release_trace(const ScaffoldType *type, AbiOperation operation) {
    if (operation != ABI_DESTROY || strcmp(type->plan->type->module->source->logical_path, ".value") != 0) { return NULL; }
    return " { assert(state->address==state); return printf(\"dropvalue=%zu\\n\",state->number)<0 ? ama_status_make(AMA_IO_FAILURE,NULL,0) : ama_status_success(); }\n";
}

/** An index-conversion error after the first output preserves that committed value, skips the refusal handler and still destroys every live value once. */
static void it_retains_an_earlier_output_when_a_later_receiver_conversion_fails(void) {
    native_hook_override = value_release_trace;
    execute_project("@!\nworker\n.worker\nearlier\n.value\n |3333\nbadindex\n.value\n |9999\nitems[]\n.value\n |1\nworker !\n input < |22\n output > earlier\n other > items[badindex]\n:\n worker !\n",
        "dropvalue=7\ndropvalue=7\ndropvalue=99\n22:1:0\ndropvalue=3333\n"
        "dropvalue=1\ndropvalue=9999\ndropvalue=23\ndropvalue=99\ndropvalue=23\ndropvalue=22\n", false);
    expect_diagnostic("Index decoder failed");
    native_hook_override = NULL;
}
