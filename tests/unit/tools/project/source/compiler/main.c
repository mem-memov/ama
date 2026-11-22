/** @file
 * @brief Exercise successful, unsuccessful and invalid compiler reply boundaries through real child processes.
 * @see [T273 contract](../../../../../../docs/implementation/source-protocol.md)
 */
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/compiler/client.h"
static DeliverySourceSnapshot snapshot;
static DeliverySourceSnapshotView view;
static SourceCompilerClient client;
static const char *executable;
/** Independently encode immutable local request/reply fixtures once for this run. */
static void generate(void) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "../../../../delivery/source/response/generate.py", (char *)NULL); _exit(127); }
    int status; assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Open independent result and input owners over immutable leaf-local fixtures. */
static void prepare(void) {
    setup_test(); delivery_source_snapshot_init(&snapshot, sources.allocator);
    const SourceText *input = NULL;
    assert(source_manager_load(&sources, "generated/base.json", &input) == SOURCE_OK);
    assert(delivery_source_snapshot_read(&snapshot, input) == SOURCE_JSON_OK);
    view = (DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, snapshot.companions};
    source_compiler_client_init(&client, sources.allocator);
}
/** Result and input have separate owners; both retire before allocator. */
static void finish(void) {
    source_compiler_client_destroy(&client); delivery_source_snapshot_destroy(&snapshot); destroy_test();
}
/** Controlled child outputs valid-looking JSON even when its status is failure. */
static int producer(const char *mode) {
    fputs("raw compiler diagnostic\n", stderr);
    if (!strcmp(mode, "signal")) { raise(SIGTERM); return 1; }
    if (!strcmp(mode, "malformed")) { fputs("{", stdout); return 0; }
    FILE *input = fopen("generated/captured-response.json", "rb"); assert(input);
    int byte; while ((byte = fgetc(input)) != EOF) { assert(fputc(byte, stdout) != EOF); }
    assert(!ferror(input) && fclose(input) == 0);
    return !strcmp(mode, "nonzero") ? 127 : 0;
}
/** Successful collection preserves diagnostics and owns graph independently. */
static void it_accepts_only_complete_successful_exact_reply(void) {
    prepare(); assert(source_compiler_client_run(&client, executable, "valid ; $(never execute)", &view) == SOURCE_COMPILER_OK);
    assert(client.valid && client.response.valid && client.diagnostics && client.process.exit_code == 0);
    assert(source_length(client.diagnostics) == strlen("raw compiler diagnostic\n"));
    assert(source_compiler_client_run(&client, executable, "valid", &view) == SOURCE_COMPILER_INVALID && client.valid);
    delivery_source_snapshot_destroy(&snapshot);
    assert(client.response.definitions && source_bytes(client.response.sources->source)[0] == '@'); finish();
}
/** Valid-looking output on nonzero exit/signal never even reaches schema decoding. */
static void it_rejects_failed_and_signalled_children_before_decoding(void) {
    prepare(); assert(source_compiler_client_run(&client, executable, "nonzero", &view) == SOURCE_COMPILER_CHILD);
    assert(!client.valid && !client.response.attempted && client.process.exit_code == 127 && !client.process.error); finish();
    prepare(); assert(source_compiler_client_run(&client, executable, "signal", &view) == SOURCE_COMPILER_CHILD);
    assert(!client.response.attempted && client.process.signal == SIGTERM); finish();
    prepare(); assert(source_compiler_client_run(&client, "/no/such/ama", "valid", &view) == SOURCE_COMPILER_CHILD);
    assert(client.process.error && !client.response.attempted); finish();
}
/** Normal child success alone does not establish protocol or capture equality. */
static void it_rejects_malformed_and_mismatched_successful_replies(void) {
    prepare(); assert(source_compiler_client_run(&client, executable, "malformed", &view) == SOURCE_COMPILER_PROTOCOL);
    assert(!client.valid && client.response.attempted && !client.response.valid); finish();
    prepare(); view.count--;
    assert(source_compiler_client_run(&client, executable, "valid", &view) == SOURCE_COMPILER_MISMATCH);
    assert(!client.valid && client.response.valid); finish();
}
/** Every result allocation refusal stays destructible and never grants authority. */
static void it_refuses_every_allocation_failure_without_retaining_partial_authority(void) {
    prepare(); size_t start = allocation_count;
    assert(source_compiler_client_run(&client, executable, "valid", &view) == SOURCE_COMPILER_OK);
    size_t count = allocation_count - start; finish(); assert(count);
    for (size_t index = 1; index <= count; index++) {
        prepare(); failure_at = allocation_count + index;
        SourceCompilerStatus status = source_compiler_client_run(&client, executable, "valid", &view);
        assert(status != SOURCE_COMPILER_OK && !client.valid); failure_at = 0; finish();
    }
}
/** Real descriptor exhaustion checks both missing streams and second-stream failure. */
static void it_refuses_temporary_stream_exhaustion_and_invalid_requests(void) {
    struct rlimit original; assert(getrlimit(RLIMIT_NOFILE, &original) == 0);
    FILE *probe = tmpfile(); assert(probe); int available = fileno(probe); assert(available >= 0);
    assert(fclose(probe) == 0);
    for (size_t index = 0; index < 2; index++) {
        prepare();
        assert(source_compiler_client_run(&client, NULL, "valid", &view) == SOURCE_COMPILER_INVALID);
        assert(!client.attempted);
        struct rlimit limited = original; limited.rlim_cur = index ? (rlim_t)available + 1 : 0;
        assert(setrlimit(RLIMIT_NOFILE, &limited) == 0);
        SourceCompilerStatus status = source_compiler_client_run(&client, executable, "valid", &view);
        assert(setrlimit(RLIMIT_NOFILE, &original) == 0);
        assert(status == SOURCE_COMPILER_PROCESS && !client.valid && !client.response.attempted); finish();
    }
}
/** The actual compiler consumes only the prepared table and exports admitted graph facts. */
static void it_accepts_real_compiler_snapshot_without_future_path_reads(void) {
    prepare();
    assert(source_compiler_client_run(&client, "../../../../../../ama",
        "generated/base.json", &view) == SOURCE_COMPILER_OK);
    assert(client.valid && client.response.definitions && !client.process.error && !client.process.signal);
    finish();
}
/** Execute explicit process admission and lifetime failure partitions. */
void run(void) {
    generate();
    it_accepts_only_complete_successful_exact_reply();
    it_rejects_failed_and_signalled_children_before_decoding();
    it_rejects_malformed_and_mismatched_successful_replies();
    it_refuses_every_allocation_failure_without_retaining_partial_authority();
    it_refuses_temporary_stream_exhaustion_and_invalid_requests();
    it_accepts_real_compiler_snapshot_without_future_path_reads();
}
/** This controlled executable also serves as its own shell-free compiler fixture. */
int main(int argc, char **argv) {
    if (argc == 4) { assert(!strcmp(argv[1], "--source-domain") && !strcmp(argv[2], "--snapshot")); return producer(argv[3]); }
    executable = argv[0]; run(); return 0;
}
