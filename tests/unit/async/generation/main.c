/** @file
 * @brief Prove nonwrapping reservation and failed literal atomicity at small and host-size bounds.
 * [ASYNC-04](../../../../spec/language/async-and-parallelism.md#async-04)
 * requires fresh generations before publishing scheduler changes.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define AMA_NATIVE(name) ama_generation_test_ ## name
#include "../../../../stdlib/modules/task/generation+/native/advance.h"
#undef AMA_NATIVE

/** Exhaust each small finite state space and the actual maximum without publishing a reused number. */
static void it_reserves_fresh_successors_and_preserves_output_on_exhaustion(void) {
    for (size_t limit = 0; limit < 8; limit++) {
        for (size_t current = 0; current <= limit + 1; current++) {
            size_t next = SIZE_MAX;
            bool reserved = ama_generation_test_reserve(current, limit, &next);
            assert(reserved == (current < limit));
            if (reserved) { assert(next == current + 1 && next <= limit); }
            else { assert(next == SIZE_MAX); }
        }
    }
    size_t next = 17;
    bool reserved = ama_generation_test_reserve(SIZE_MAX - 1, SIZE_MAX, &next);
    assert(reserved && next == SIZE_MAX);
    reserved = ama_generation_test_reserve(SIZE_MAX, SIZE_MAX, &next);
    assert(!reserved && next == SIZE_MAX);
}

/** Counted decimal accepts the precise scalar range and rejects malformed/overflowing input before writes. */
static void it_parses_bounded_generation_literals_without_partial_replacement(void) {
    char maximum[3 * sizeof(size_t) + 2];
    int length = snprintf(maximum, sizeof(maximum), "%zu", SIZE_MAX);
    assert(length > 0 && (size_t)length + 1 < sizeof(maximum));
    size_t value = 17;
    bool parsed = ama_generation_test_parse((AmaByteView){(const unsigned char *)maximum, (size_t)length}, &value);
    assert(parsed && value == SIZE_MAX);
    maximum[length] = '0';
    parsed = ama_generation_test_parse((AmaByteView){(const unsigned char *)maximum, (size_t)length + 1}, &value);
    assert(!parsed && value == SIZE_MAX);
    const char *invalid[] = {"", "-1", "+1", " 1", "1 ", "1x", "1\n"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        parsed = ama_generation_test_parse((AmaByteView){(const unsigned char *)invalid[index], strlen(invalid[index])}, &value);
        assert(!parsed && value == SIZE_MAX);
    }
    parsed = ama_generation_test_parse((AmaByteView){NULL, 1}, &value);
    assert(!parsed && value == SIZE_MAX);
    const unsigned char counted[] = {'1', 0, '2'};
    parsed = ama_generation_test_parse((AmaByteView){counted, sizeof(counted)}, &value);
    assert(!parsed && value == SIZE_MAX);
    parsed = ama_generation_test_parse((AmaByteView){(const unsigned char *)"00001", 5}, &value);
    assert(parsed && value == 1);
    parsed = ama_generation_test_parse((AmaByteView){(const unsigned char *)"0", 1}, &value);
    assert(parsed && value == 0);
}

/** Execute a real compiler or native consumer with exact arguments and require normal success. */
static void require_process(char *const arguments[]) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) { execvp(arguments[0], arguments); perror(arguments[0]); _exit(127); }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/** Compile the actual primitive against its generated ABI; failed entry/literal never wraps its scalar. */
static void it_checks_actual_native_generation_lifecycle_and_exhaustion(void) {
    int created = mkdir("generated/project", 0700);
    assert(created == 0);
    char *author[] = {TEST_ROOT "/ama", "--native", "generated/project", "//task/generation",
        "--out", "generated/native", "--sdk", TEST_ROOT "/src", "--standard-root", TEST_ROOT "/stdlib/modules", NULL};
    require_process(author);
    FILE *source = fopen("generated/consumer.c", "wb");
    assert(source != NULL);
    int written = fputs("#undef NDEBUG\n#include <assert.h>\n#include <stdio.h>\n"
        "#include \"" TEST_ROOT "/stdlib/modules/task/generation.c\"\n"
        "#define GEN(n) ama_n_a7fa5ca88d01ea11806a55d1390a8bc45170135ae628f5b00b3cbe00d626ca33_ ## n\n"
        "int main(void) { GEN(State) state,copy; AmaStatus status=GEN(initialize)(&state); assert(status.code==AMA_OK && state.value==0);\n"
        "bool truth=true; status=GEN(condition)(&state,&truth); assert(status.code==AMA_OK && !truth);\n"
        "ama_t_92c9b7db521de1223828a1d2f47159c1e63a09333abd3792fa447b6b911a8a27 parameters={0};\n"
        "ama_t_c15fe5ca9d70e8a08dcdbc4a115e762360a139942a52483791e7d7ea5cf88cf9 results={0}; AmaNativeServices services={0};\n"
        "GEN(entry)(&state,&parameters,&results,&services,&status); assert(status.code==AMA_OK && state.value==1);\n"
        "status=GEN(clone)(&copy,&state); assert(status.code==AMA_OK && copy.value==1);\n"
        "GEN(entry)(&state,&parameters,&results,&services,&status); assert(status.code==AMA_OK && state.value==2 && copy.value==1);\n"
        "char maximum[3*sizeof(size_t)+2]; int length=snprintf(maximum,sizeof(maximum),\"%zu\",SIZE_MAX); assert(length>0);\n"
        "status=GEN(literal)(&state,(AmaByteView){(const unsigned char *)maximum,(size_t)length}); assert(status.code==AMA_OK && state.value==SIZE_MAX);\n"
        "GEN(entry)(&state,&parameters,&results,&services,&status); assert(status.code==AMA_NATIVE_FAILURE && state.value==SIZE_MAX);\n"
        "status=GEN(literal)(&state,(AmaByteView){(const unsigned char *)\"-1\",2}); assert(status.code==AMA_INVALID_LITERAL && state.value==SIZE_MAX);\n"
        "status=GEN(condition)(&state,&truth); assert(status.code==AMA_OK && truth);\n"
        "status=GEN(destroy)(&copy); assert(status.code==AMA_OK); status=GEN(destroy)(&state); assert(status.code==AMA_OK); return 0; }\n", source);
    int closed = fclose(source);
    assert(written != EOF && closed == 0);
    char *compile[] = {TEST_C_COMPILER, "-std=c11", "-O1", "-Wall", "-Wextra", "-Werror", "-pedantic-errors",
#ifdef TEST_SANITIZERS
        "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-fno-pie", "-no-pie",
#endif
        "-I" TEST_ROOT "/src", "-Igenerated/native", "generated/consumer.c", "-o", "generated/consumer", NULL};
    require_process(compile);
    char *execute[] = {"generated/consumer", NULL};
    require_process(execute);
}

/** Register arithmetic and parsing invariants explicitly. */
void run(void) {
    it_reserves_fresh_successors_and_preserves_output_on_exhaustion();
    it_parses_bounded_generation_literals_without_partial_replacement();
    it_checks_actual_native_generation_lifecycle_and_exhaustion();
}
/** Return after exact boundaries and invalid counted inputs have been checked. */
int main(void) { run(); return 0; }
