/** @file
 * @brief Provide observable native conditions and an executable two-link loop header.
 */
#ifndef AMA_LOOP_TEST_COMMON
#define AMA_LOOP_TEST_COMMON
#include "../execution/common.c"

/** Trace the actual operand without changing its truth. A bounded test-only
 * watchdog fails beyond the six conversions needed by the largest fixture,
 * instead of allowing a broken loop to hang.
 * Error deliberately writes true first: failed protocol outputs must be ignored.
 */
static const char *observe_condition(const ScaffoldType *type, AbiOperation operation) {
    if (operation != ABI_CONDITION || strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/value") != 0) { return NULL; }
    return " { static size_t checks; checks++; printf(\"truth=%zu\\n\",state->number); "
        "if(checks>6 || state->number==500) { *result=true; return ama_status_make(AMA_NATIVE_FAILURE,"
        "\"Condition conversion failed\",sizeof(\"Condition conversion failed\")-1); } "
        "*result=state->number!=0; return ama_status_success(); }\n";
}

/** The root emits a persistent prefix counter; its selected method returns the
 * decremented caller input, allowing the body to copy it after header release.
 */
static void prepare_loop_header(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("decrement", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/container", "@>\nchild\n/test/fixture/v1.0.0/src/decrement\n@!\ntrace\n/test/fixture/v1.0.0/src/worker\ntrace !\n < |100\n");
}
#endif
