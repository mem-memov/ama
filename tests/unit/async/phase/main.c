/** @file
 * @brief Exhaust phase choices and rejection atomicity independently of generated execution.
 * [ASYNC-03](../../../../spec/language/async-and-parallelism.md#async-03)
 * requires one explicit phase, separate from failure and scheduler state.
 */
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define AMA_NATIVE(name) ama_phase_test_ ## name
#include "../../../../stdlib/modules/task/phase+/native/selection.h"
#undef AMA_NATIVE

/** Every accepted discriminator maps to its distinct basis vector; all other bytes are rejected. */
static void it_selects_exactly_one_phase_and_preserves_output_on_invalid_choices(void) {
    for (unsigned kind = 0; kind <= UCHAR_MAX; kind++) {
        bool predicates[3] = {true, true, true};
        bool selected = ama_phase_test_select(kind, predicates);
        if (kind < 3) {
            assert(selected);
            for (unsigned index = 0; index < 3; index++) {
                assert(predicates[index] == (index == kind));
            }
        } else {
            assert(!selected && predicates[0] && predicates[1] && predicates[2]);
        }
    }
    bool predicates[3] = {false, true, false};
    bool selected = ama_phase_test_select(UINT_MAX, predicates);
    assert(!selected && !predicates[0] && predicates[1] && !predicates[2]);
}

/** Execute one exact tool argument vector and require normal successful completion. */
static void require_process(char *const arguments[]) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        execvp(arguments[0], arguments);
        perror(arguments[0]);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/** Compile only the actual native phase component and generated declarations, without a label program. */
static void it_publishes_predicates_through_the_actual_native_write_view(void) {
    int created = mkdir("generated/project", 0700);
    assert(created == 0);
    char *author[] = {TEST_ROOT "/ama", "--native", "generated/project", "//task/phase",
        "--out", "generated/native", "--sdk", TEST_ROOT "/src", "--standard-root",
        TEST_ROOT "/stdlib/modules", NULL};
    require_process(author);
    FILE *source = fopen("generated/consumer.c", "wb");
    assert(source != NULL);
    int written = fputs(
        "#undef NDEBUG\n#include <assert.h>\n#include <limits.h>\n"
        "#include \"ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66.interface.h\"\n"
        "#include \"ama_n_3f995290b378db1c82dbfdb55c99332bfe86d8ee249afbcfca803ce322998c66.begin.h\"\n"
        "int main(void) { AMA_NATIVE(State) state={0};\n"
        "ama_n_0d83535743d147ae021af952a8f696ab4c0b3b1a4ed7b6e081f902fd8a585a84_State values[3]={{0}};\n"
        "ama_t_e2b7dab561a140d0cbf3b2d74847990bc8ed39d9e59aa2a0c3434205b0aaf614 fields[3]={{.payload=&values[0]},{.payload=&values[1]},{.payload=&values[2]}};\n"
        "ama_t_f14ae70b65c2b1e0e969f4ae06aed3cc57e808f832f94294aac1f39e559cc930 target={.payload=&state,"
        ".AMA_OUTPUT(0)=&fields[0],.AMA_OUTPUT(1)=&fields[1],.AMA_OUTPUT(2)=&fields[2]};\n"
        "for(unsigned kind=0;kind<3;kind++) { AmaStatus status=AMA_NATIVE(assign)(&target,kind); assert(status.code==AMA_OK);"
        "for(unsigned index=0;index<3;index++) assert(values[index].value==(index==kind)); }\n"
        "AmaStatus status=AMA_NATIVE(assign)(&target,UINT_MAX); assert(status.code==AMA_NATIVE_FAILURE);"
        "assert(values[0].value==0 && values[1].value==0 && values[2].value==1); return 0; }\n",
        source);
    int closed = fclose(source);
    assert(written != EOF && closed == 0);
    char *compile[] = {TEST_C_COMPILER, "-std=c11", "-O1", "-Wall", "-Wextra", "-Werror", "-pedantic-errors",
#ifdef TEST_SANITIZERS
        "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-fno-pie", "-no-pie",
#endif
        "-I" TEST_ROOT "/src", "-Igenerated/native", "generated/consumer.c",
        TEST_ROOT "/stdlib/modules/task/phase.c", "-o", "generated/consumer", NULL};
    require_process(compile);
    char *execute[] = {"generated/consumer", NULL};
    require_process(execute);
}
/** List the independent representation invariant explicitly. */
void run(void) {
    it_selects_exactly_one_phase_and_preserves_output_on_invalid_choices();
    it_publishes_predicates_through_the_actual_native_write_view();
}
/** Return after all valid phases and invalid boundary choices have been checked. */
int main(void) { run(); return 0; }
