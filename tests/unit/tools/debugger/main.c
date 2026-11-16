/** @file
 * @brief Test exact command recognition, real context/native observations and cleanup on refused transport.
 * Fixture context admission and native resources are real; callback counters test command dispatch, not Ama execution semantics.
 */
#include "../../runtime/contexts/common.c"
#include "tools/debugger/internal.h"
#include <sys/wait.h>
#include <unistd.h>

/** Evaluate fixture effects before asserting, so disabled assertions cannot remove IO or cleanup. */
static void verify_condition(bool condition, size_t line) {
    if (!condition) { fprintf(stderr, "Debugger fixture check failed at line %zu\n", line); }
    assert(condition);
}
#define require(expression) verify_condition((expression), __LINE__)


/** One explicit host adapter owns no resource outside the real counted registry. */
typedef struct {
    uint64_t identity;
    AmaContextReference root;
    size_t runs, resumes, begins, disposals, previews;
    bool readers;
    bool disposed;
    AmaStatus reader_status;
    AmaStatus final_status;
} Host;
static Host *reader_host;

/** Produce four independent typed views of known fixture-owned native data, one per inspection. */
static AmaStatus native(const void *payload, AmaDebugNativeVisitor visit, void *context) {
    require(((const NativeValue *)payload)->number == 42);
    unsigned char integer[] = {255, 1};
    unsigned char text[] = {0xd0, 0x90, 0, 10};
    unsigned char bytes[] = {0, 255, 13, 10};
    unsigned kind = (unsigned)(reader_host->previews++ % 4);
    AmaDebugNativeValue value = {.kind=(AmaDebugValueKind)kind};
    if (kind == AMA_DEBUG_BOOLEAN) { value.flag = true; }
    if (kind == AMA_DEBUG_INTEGER) { value.negative = true; value.little_endian = true; value.bytes = (AmaByteView){integer, sizeof(integer)}; }
    if (kind == AMA_DEBUG_TEXT) { value.bytes = (AmaByteView){text, sizeof(text)}; }
    if (kind == AMA_DEBUG_BYTES) { value.bytes = (AmaByteView){bytes, sizeof(bytes)}; }
    (void)visit(context, &value);
    return reader_host->reader_status;
}
/** Dispatch counters are independent from production language execution, covered by real generated-project integration. */
static AmaContextOutcome execute(void *context, AmaAccessMode mode) {
    Host *host = context;
    require(!host->disposed);
    if (mode == AMA_ACCESS_ENTRY) { host->runs++; } else { require(mode == AMA_ACCESS_RESUME); host->resumes++; }
    return (AmaContextOutcome){AMA_HOST_YIELDED, host->root, ama_status_success()};
}
/** Admission is real, including catalog absence and optional host-authorized native layout knowledge. */
static AmaStatus begin(void *context, AmaDebugSession *session) {
    Host *host = context;
    host->begins++;
    static const AmaDebugNativeReader reader = {2, native};
    return ama_debug_begin(&registry, host->identity, NULL, host->readers ? &reader : NULL, host->readers ? 1 : 0, session);
}
/** Actual context disposal detects any inspection lease that escaped an error path. */
static AmaStatus dispose(void *context) {
    Host *host = context;
    host->disposals++;
    if (host->disposed) { return host->final_status; }
    AmaStatus status = ama_context_dispose(&registry, host->identity);
    require(status.code == AMA_OK);
    host->disposed = true;
    return host->final_status;
}
/** Create real address-sensitive native values and release setup admission before the command loop. */
static Host host_create(void) {
    setup_contexts();
    Host host = {.identity=create_context()};
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(host.identity, &lease);
    host.root = create_root(&lease, view);
    root_value(&lease, host.root)->native.number = 42;
    leave_context(&lease);
    return host;
}
/** The caller retains every stream until the borrowed command loop has completely returned. */
static int run_commands(Host *host, const char *commands, FILE *output) {
    FILE *input = tmpfile(), *errors = tmpfile();
    require(input != NULL && errors != NULL);
    require(fwrite(commands, 1, strlen(commands), input) == strlen(commands));
    require(fseek(input, 0, SEEK_SET) == 0);
    AmaDebugClientHost client = {host, execute, begin, dispose};
    reader_host = host;
    int result = ama_debug_client_run(&client, input, output, errors);
    require(fclose(input) == 0 && fclose(errors) == 0);
    return result;
}
/** Exact finite recognition has no whitespace folding, NUL termination shortcut or prefix acceptance. */
static void it_recognizes_only_complete_exact_command_lines(void) {
    const char *names[] = {"run", "resume", "inspect", "dispose", "quit"};
    AmaDebugClientCommand kinds[] = {AMA_DEBUG_CLIENT_RUN, AMA_DEBUG_CLIENT_RESUME,
        AMA_DEBUG_CLIENT_INSPECT, AMA_DEBUG_CLIENT_DISPOSE, AMA_DEBUG_CLIENT_QUIT};
    for (size_t index = 0; index < 5; index++) {
        for (unsigned suffix = 0; suffix < 256; suffix++) {
            FILE *input = tmpfile(); require(input != NULL);
            require(fputs(names[index], input) != EOF && fputc((int)suffix, input) != EOF && fputs("\nquit", input) != EOF);
            require(fseek(input, 0, SEEK_SET) == 0);
            require(ama_debug_client_read(input) == (suffix == '\n' ? kinds[index] : AMA_DEBUG_CLIENT_INVALID));
            if (suffix == '\n') { require(ama_debug_client_read(input) == AMA_DEBUG_CLIENT_INVALID); }
            require(ama_debug_client_read(input) == AMA_DEBUG_CLIENT_QUIT);
            require(ama_debug_client_read(input) == AMA_DEBUG_CLIENT_END);
            require(fclose(input) == 0);
        }
    }
    FILE *input = tmpfile(); require(input != NULL);
    for (size_t index = 0; index < 10000; index++) { require(fputc('r', input) != EOF); }
    require(fputs("\ninspect\n", input) != EOF && fseek(input, 0, SEEK_SET) == 0);
    require(ama_debug_client_read(input) == AMA_DEBUG_CLIENT_INVALID);
    require(ama_debug_client_read(input) == AMA_DEBUG_CLIENT_INSPECT);
    require(ama_debug_client_read(input) == AMA_DEBUG_CLIENT_END && fclose(input) == 0);
}
/** A standard JSON decoder checks all four preview encodings and unchanged observations across repeated inspection. */
static void it_streams_real_state_and_only_authorized_native_views(void) {
    Host host = host_create(); host.readers = true;
    FILE *output = fopen("generated/views.jsonl", "wb"); require(output != NULL);
    require(run_commands(&host, "inspect\ninspect\ninspect\ninspect\nquit\n", output) == EXIT_SUCCESS);
    require(host.begins == 4 && host.previews == 4 && host.runs == 0 && host.resumes == 0 && host.disposals == 1);
    require(fclose(output) == 0);
    pid_t child = fork(); require(child >= 0);
    if (child == 0) { execlp("python3", "python3", "verify.py", "generated/views.jsonl", (char *)NULL); _exit(127); }
    int status; require(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    finish_contexts();
}
/** Invalid commands cause no callback, a refused output stops later execution and disposal always observes a free gate. */
static void it_stops_on_failed_output_and_releases_inspection_before_cleanup(void) {
    Host host = host_create();
    FILE *output = tmpfile(); require(output != NULL);
    require(run_commands(&host, " run\nresume extra\nrun\nresume\nquit\nrun\n", output) == EXIT_FAILURE);
    require(host.runs == 1 && host.resumes == 1 && host.disposals == 1 && fclose(output) == 0);
    finish_contexts();
    host = host_create();
    output = fopen("/dev/full", "wb"); require(output != NULL);
    char execution_buffer[8192]; require(setvbuf(output, execution_buffer, _IOFBF, sizeof(execution_buffer)) == 0);
    require(run_commands(&host, "run\nresume\n", output) == EXIT_FAILURE);
    require(host.runs == 1 && host.resumes == 0 && host.disposals == 1);
    (void)fclose(output); finish_contexts();
    host = host_create();
    output = fopen("/dev/full", "wb"); require(output != NULL);
    char inspection_buffer[512]; require(setvbuf(output, inspection_buffer, _IOFBF, sizeof(inspection_buffer)) == 0);
    require(run_commands(&host, "inspect\nrun\n", output) == EXIT_FAILURE);
    require(host.begins == 1 && host.runs == 0 && host.disposals == 1);
    (void)fclose(output); finish_contexts();
}
/** Directory-backed read failure is real IO; no command or execution is inferred from EOF. */
static void it_disposes_after_input_failure_and_empty_eof(void) {
    Host host = host_create();
    FILE *input = fopen(".", "rb"), *output = tmpfile(), *errors = tmpfile();
    require(input != NULL && output != NULL && errors != NULL);
    AmaDebugClientHost client = {&host, execute, begin, dispose};
    require(ama_debug_client_run(&client, input, output, errors) == EXIT_FAILURE);
    require(host.disposals == 1 && host.runs == 0 && host.begins == 0);
    require(fclose(input) == 0 && fclose(output) == 0 && fclose(errors) == 0);
    finish_contexts();
    host = host_create(); output = tmpfile(); require(output != NULL);
    require(run_commands(&host, "", output) == EXIT_SUCCESS && host.disposals == 1);
    require(fclose(output) == 0); finish_contexts();
}
/** A failed preview or final host result belongs to the command transport, never the inspected Ama status. */
static void it_preserves_inspection_failure_separately_from_program_status(void) {
    Host host = host_create(); host.readers = true;
    host.reader_status = ama_status_make(AMA_NATIVE_FAILURE, "Preview refused", 15);
    FILE *output = fopen("generated/refused-view.jsonl", "wb"); require(output != NULL);
    require(run_commands(&host, "inspect\nquit\n", output) == EXIT_FAILURE);
    require(host.previews == 1 && host.disposals == 1 && fclose(output) == 0);
    pid_t child = fork(); require(child >= 0);
    if (child == 0) { execlp("python3", "python3", "verify.py", "generated/refused-view.jsonl", "failed-preview", (char *)NULL); _exit(127); }
    int status; require(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    finish_contexts();
    host = host_create(); host.final_status = ama_status_make(AMA_NATIVE_FAILURE, NULL, 0);
    output = tmpfile(); require(output != NULL);
    require(run_commands(&host, "dispose\nquit\n", output) == EXIT_FAILURE);
    require(host.disposals == 2 && host.disposed && fclose(output) == 0);
    finish_contexts();
}
/** Run exact transport boundaries separately from generated-project acceptance. */
void run(void) {
    it_recognizes_only_complete_exact_command_lines();
    it_streams_real_state_and_only_authorized_native_views();
    it_stops_on_failed_output_and_releases_inspection_before_cleanup();
    it_disposes_after_input_failure_and_empty_eof();
    it_preserves_inspection_failure_separately_from_program_status();
}
/** Counted fixture destruction proves no context or native resource escaped any failed transport. */
int main(void) { run(); return EXIT_SUCCESS; }
