/** @file
 * @brief Execute ordinary task phases, real yields and explicit nested waiting propagation.
 * [ASYNC-03](../../../../spec/language/async-and-parallelism.md#async-03)
 * keeps phase, actual host outcome and terminal application data distinct.
 */
#include "../execution/common.c"

/** Observe ordinary typed results; no C observer retains an Ama reference or view. */
static bool phase_observer(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/show") != 0) { return false; }
    int written = fputs(
        "test_phase=parameters->AMA_INPUT(0)->payload->value"
        "+2*parameters->AMA_INPUT(1)->payload->value+4*parameters->AMA_INPUT(2)->payload->value;\n"
        "test_registered=parameters->AMA_INPUT(3)->payload->value;"
        "test_failure=parameters->AMA_INPUT(4)->payload->failed;\n"
        "AmaByteView text; *status=ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(5)->payload,&text);"
        "if(status->code!=AMA_OK) return;"
        "printf(\"%u:%u:%u:\",test_phase,test_registered,test_failure);"
        "if(text.length!=0) fwrite(text.data,1,text.length,stdout); putchar('\\n');\n", output);
    assert(written != EOF);
    return true;
}

/** Keep the repeated ordinary result reads explicit in Ama, rather than inspecting private phase C fields. */
static const char display[] =
    "phase\n ready >> ready\n awaiting >> awaiting\n completed >> completed\n"
    "failure\n description >> description\nshow !\n < ready\n < awaiting\n < completed\n < registered\n < failure\n < description\n";

/** Install only a test-owned observation endpoint; actual library modules remain unchanged. */
static void phase_fixture(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/observer.h", "#ifndef AMA_PHASE_TEST_OBSERVER\n#define AMA_PHASE_TEST_OBSERVER\n"
        "static unsigned test_phase,test_registered,test_failure;\n#endif\n");
    native_fixture("show", "@<\nready\n//std/condition\nawaiting\n//std/condition\ncompleted\n//std/condition\n"
        "registered\n//std/condition\nfailure\n//failure\ndescription\n//text\n@C\n");
    forced_header = "generated/project/packages/test/fixture/v1.0.0/src/observer.h";
    native_entry_override = phase_observer;
}

/** Compile one bounded authored scenario with explicit resource-free phase observations. */
static void phase_project(const char *prefix, const char *middle, const char *suffix, const char *expected) {
    char source[16384];
    int length = snprintf(source, sizeof(source), "%s%s%s%s%s", prefix, display, middle, display, suffix);
    assert(length > 0 && (size_t)length < sizeof(source));
    execute_project(source, expected, true);
}

/** Every constructor is distinct, copies preserve phase and retained reads never reset it. */
static void it_constructs_copies_and_resets_ordinary_phase_values(void) {
    phase_fixture();
    phase_project("@!\nshow\n/test/fixture/v1.0.0/src/show\nregistered\n//std/condition\nfailure\n//failure\nphase\n//task/phase\n"
        "waiting\n//task/phase/awaiting\ndone\n//task/phase/completed\nready maker\n//task/phase/ready\n",
        "waiting !\n > phase\n",
        "copy\n phase\nphase !\ncopy\n ready >> ready\n awaiting >> awaiting\n completed >> completed\n"
        "show !\n < ready\n < awaiting\n < completed\n < registered\n < failure\n < description\n"
        "phase\n ready >> ready\n awaiting >> awaiting\n completed >> completed\n"
        "show !\n < ready\n < awaiting\n < completed\n < registered\n < failure\n < description\n"
        "done !\n > phase\nphase\n ready >> ready\n awaiting >> awaiting\n completed >> completed\n"
        "show !\n < ready\n < awaiting\n < completed\n < registered\n < failure\n < description\n"
        "ready maker !\n > phase\nphase\n ready >> ready\n awaiting >> awaiting\n completed >> completed\n"
        "show !\n < ready\n < awaiting\n < completed\n < registered\n < failure\n < description\n",
        "1:0:0:\n2:0:0:\n2:0:0:\n1:0:0:\n4:0:0:\n1:0:0:\n");
    forced_header = NULL;
}

/** First entry, two resumes and restart agree with actual host outcomes; awaiting false failure is unfinished. */
static void it_matches_real_yields_and_preserves_terminal_failure_description(void) {
    phase_fixture();
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);"
        "assert(result.outcome==AMA_HOST_YIELDED && test_phase==1 && test_failure==0);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME);"
        "assert(result.outcome==AMA_HOST_YIELDED && test_phase==2 && test_registered==1 && test_failure==0);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME);"
        "assert(result.outcome==AMA_HOST_COMPLETED && test_phase==4 && test_failure==1);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_ENTRY);"
        "assert(result.outcome==AMA_HOST_YIELDED && test_phase==1 && test_failure==0);\n"
        "AmaStatus disposed=ama_project_dispose(&host); assert(disposed.code==AMA_OK); return 0; }\n";
    char middle[4096], suffix[4096];
    int length = snprintf(middle, sizeof(middle), "%%\nregistered\n //std/condition/true\nwaiting !\n > phase\n");
    assert(length > 0 && (size_t)length < sizeof(middle));
    length = snprintf(suffix, sizeof(suffix), "%%\nregistered\n //std/condition/false\ndone !\n > phase\nmake failure !\n < |terminal description\n > failure\n%s", display);
    assert(length > 0 && (size_t)length < sizeof(suffix));
    phase_project("@!\nshow\n/test/fixture/v1.0.0/src/show\nregistered\n//std/condition\nfailure\n//failure\nphase\n//task/phase\n"
        "waiting\n//task/phase/awaiting\ndone\n//task/phase/completed\nmake failure\n//failure/make\n"
        "phase !\nfailure !\nregistered\n //std/condition/false\n",
        middle, suffix, "1:0:0:\n2:1:0:\n4:0:1:terminal description\n1:0:0:\n");
    host_launcher = NULL;
    forced_header = NULL;
}

/** A yielded child does not suspend its parent: the parent explicitly republishes phase and executes %. */
static void it_propagates_nested_waiting_only_through_explicit_parent_yield(void) {
    phase_fixture();
    file("generated/project/packages/test/fixture/v1.0.0/src/child", "@>\nphase\n//task/phase\n@!\nwaiting\n//task/phase/awaiting\ndone\n//task/phase/completed\n"
        "waiting !\n > phase\n%\ndone !\n > phase\n");
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);"
        "assert(result.outcome==AMA_HOST_YIELDED && test_phase==2 && test_registered==1 && test_failure==0);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME);"
        "assert(result.outcome==AMA_HOST_COMPLETED && test_phase==4 && test_failure==0);\n"
        "AmaStatus disposed=ama_project_dispose(&host); assert(disposed.code==AMA_OK); return 0; }\n";
    phase_project("@!\nshow\n/test/fixture/v1.0.0/src/show\nregistered\n//std/condition\n |1\nfailure\n//failure\nchild\n/test/fixture/v1.0.0/src/child\nchild !\n > phase\n",
        "%\nchild %\n > phase\nregistered\n //std/condition/false\n", "", "2:1:0:\n4:0:0:\n");
    host_launcher = NULL;
    forced_header = NULL;
}

/** Completed is terminal: another resume refuses rather than replaying a completed task. */
static void it_refuses_resume_after_a_completed_phase(void) {
    phase_fixture();
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@>\nphase\n//task/phase\n@!\ndone\n//task/phase/completed\ndone !\n > phase\n");
    char source[4096];
    int length = snprintf(source, sizeof(source), "@!\nshow\n/test/fixture/v1.0.0/src/show\nregistered\n//std/condition\n"
        "failure\n//failure\ntask\n/test/fixture/v1.0.0/src/task\ntask !\n > phase\n%stask %%\n", display);
    assert(length > 0 && (size_t)length < sizeof(source));
    execute_project(source, "4:0:0:\n", false);
    expect_diagnostic("Ama runtime failure: 10");
    forced_header = NULL;
}

/** Register phase representation and actual control-flow compositions explicitly. */
void run(void) {
    it_constructs_copies_and_resets_ordinary_phase_values();
    it_matches_real_yields_and_preserves_terminal_failure_description();
    it_propagates_nested_waiting_only_through_explicit_parent_yield();
    it_refuses_resume_after_a_completed_phase();
}
/** Return only after actual generated code and host-boundary observations agree. */
int main(void) { run(); return EXIT_SUCCESS; }
