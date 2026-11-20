/** @file
 * @brief Compile complete expected-failure values and retained owned text descriptions.
 */
#include "../execution/common.c"

/** Build an outcome with explicit presence, or display a borrowed text value. */
static bool failure_entry(FILE *output, const ScaffoldType *type) {
    const char *logical = type->plan->type->module->source->logical_path;
    const char *body = NULL;
    if (strcmp(logical, "/test/fixture/v1.0.0/src/operation") == 0 || strcmp(logical, "/test/fixture/v1.0.0/src/other") == 0) {
        body = "if(parameters->AMA_INPUT(0)->payload->value!=0) { AmaByteView message;"
            "*status=ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(1)->payload,&message);"
            "if(status->code!=AMA_OK) return;"
            "*status=ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_set(results->AMA_OUTPUT(0),message);"
            "} else { *status=ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_clear(results->AMA_OUTPUT(0)); }"
            "if(status->code!=AMA_OK) return;\n";
    } else if (strcmp(logical, "/test/fixture/v1.0.0/src/show") == 0) {
        body = "AmaByteView bytes;*status=ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->payload,&bytes);"
            "if(status->code!=AMA_OK) return; fputs(\"text=\",stdout);"
            "if(bytes.length!=0) fwrite(bytes.data,1,bytes.length,stdout);putchar('\\n');\n";
    }
    if (body == NULL) { return false; }
    int written = fputs(body, output);
    assert(written != EOF);
    return true;
}

/** Author only test-owned native consumers; standard implementations are real. */
static void failure_standard(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("operation", "@<\nrequested\n//std/condition\nmessage\n//text\n@>\nfailure\n//failure\n@C\n");
    native_fixture("other", "@<\nrequested\n//std/condition\nmessage\n//text\n@>\nfailure\n//failure\n@C\n");
    native_fixture("show", "@<\nmessage\n//text\n@C\n");
    native_entry_override = failure_entry;
}

/** One result owns condition and description; successful reentry clears both. */
static void it_reads_and_copies_failure_descriptions_without_stale_success_data(void) {
    failure_standard();
    execute_project("@!\nop\n/test/fixture/v1.0.0/src/operation\nshow\n/test/fixture/v1.0.0/src/show\nop !\n < |1\n < |first failure\n > failure\nfailure\n?\n failure\n  description >> message\n show !\n  < message\n"
        "copy\n failure\nop !\n < |0\n > failure\nfailure\n:\n failure\n  description >> message\n show !\n  < message\n"
        "copy\n?\n copy\n  description >> message\n show !\n  < message\n",
        "text=first failure\ntext=\ntext=first failure\n", true);
}

/** Explicit invocation clears a failure value; retained condition/read never do. */
static void it_preserves_outcomes_during_reads_and_clears_on_explicit_entry(void) {
    failure_standard();
    execute_project("@!\nop\n/test/fixture/v1.0.0/src/operation\nshow\n/test/fixture/v1.0.0/src/show\nop !\n < |1\n < |message\n > failure\nfailure\n?\n failure\n  description >> message\n show !\n  < message\nfailure !\nfailure\n:\n failure\n  description >> message\n show !\n  < message\n",
        "text=message\ntext=\n", true);
}

/** Same result spellings in different native modules keep independent values. */
static void it_keeps_same_name_results_in_distinct_modules_independent(void) {
    failure_standard();
    execute_project("@!\nfirst\n/test/fixture/v1.0.0/src/operation\nsecond\n/test/fixture/v1.0.0/src/other\nshow\n/test/fixture/v1.0.0/src/show\nfirst !\n < |1\n < |one\n > first failure\nsecond !\n < |1\n < |второй\n > second failure\n"
        "first failure\n description >> first message\nsecond failure\n description >> second message\nshow !\n < first message\nshow !\n < second message\n",
        "text=one\ntext=второй\n", true);
}

/** Presence is independent of description length, while defaults are false/empty. */
static void it_uses_the_declared_discriminator_instead_of_message_text(void) {
    failure_standard();
    execute_project("@!\ninitial\n//failure\nop\n/test/fixture/v1.0.0/src/operation\nshow\n/test/fixture/v1.0.0/src/show\ninitial\n:\n initial\n  description >> message\n show !\n  < message\nop !\n < |1\n < |\n > failure\nfailure\n?\n failure\n  description >> message\n show !\n  < message\n",
        "text=\ntext=\n", true);
}

/** Pure Ama uses an ordinary constructor module, then retained public results. */
static void it_constructs_failure_values_through_the_actual_standard_module(void) {
    failure_standard();
    execute_project("@!\nmake\n//failure/make\nshow\n/test/fixture/v1.0.0/src/show\nmake !\n < |constructed\n > failure\nfailure\n?\n failure\n  description >> description\n show !\n  < description\n",
        "text=constructed\n", true);
}

/** Expected-failure results and successful reads never clear a caller refusal. */
static void it_keeps_application_failure_values_separate_from_last_refusal_data(void) {
    failure_standard();
    native_fixture("inspect", "@<\nstatus\n//std/status\n@C\n");
    execute_project("@!\nmissing&\n/test/fixture/v1.0.0/src/operation\nop\n/test/fixture/v1.0.0/src/operation\nreader\n//std/refusal\ninspect\n/test/fixture/v1.0.0/src/inspect\nshow\n/test/fixture/v1.0.0/src/show\nmissing& !\n:\n reader !\n  > before\nop !\n < |1\n < |expected\n > failure\nfailure\n?\n failure\n  description >> message\n show !\n  < message\nreader !\n > after\ninspect !\n < before\ninspect !\n < after\n",
        "text=expected\nstatus=7\nstatus=7\n", true);
}

/** An input literal allocates first; description preparation fails second.
 * The technical OOM is propagated, without outputs or acquisition handlers.
 */
static void it_propagates_description_allocation_failure_as_a_technical_error(void) {
    failure_standard();
    file("generated/text-allocation.h", "#include <stdlib.h>\n#include <stdio.h>\n"
        "static inline void *test_text_allocate(size_t size) { static size_t calls; calls++;"
        "if(calls==2) { puts(\"description-allocation-failed\"); return NULL; } return malloc(size); }\n"
        "#define AMA_TEXT_ALLOCATE(size) test_text_allocate(size)\n#define AMA_TEXT_RELEASE(pointer) free(pointer)\n");
    forced_header = "generated/text-allocation.h";
    execute_project("@!\nop\n/test/fixture/v1.0.0/src/operation\nshow\n/test/fixture/v1.0.0/src/show\nop !\n < |1\n < |message\n > failure\n:\n show !\n  < |handler must not run\n",
        "description-allocation-failed\n", false);
    expect_diagnostic("Ama runtime failure: 2");
    forced_header = NULL;
}

/** Exercise source inference, generated graph cloning, protocols and native helpers. */
void run(void) {
    it_reads_and_copies_failure_descriptions_without_stale_success_data();
    it_preserves_outcomes_during_reads_and_clears_on_explicit_entry();
    it_keeps_same_name_results_in_distinct_modules_independent();
    it_uses_the_declared_discriminator_instead_of_message_text();
    it_constructs_failure_values_through_the_actual_standard_module();
    it_keeps_application_failure_values_separate_from_last_refusal_data();
    it_propagates_description_allocation_failure_as_a_technical_error();
    native_entry_override = NULL;
}

/** Return after complete result states and exact description outputs match. */
int main(void) { run(); return EXIT_SUCCESS; }
