/** @file
 * @brief Verify actual entry-free source-domain dispatch, native nonexecution and failure output.
 * @see [T224 contract](../../../../docs/implementation/source-operations.md)
 */
#include "../common.c"
#include "../../fixtures/package.h"
#include <sys/wait.h>
#include <unistd.h>
#include "delivery/source/probe/protocol.h"
/** Write only explicitly owned fixture files, preserving exact supplied bytes. */
static void file(const char *path, const char *text) {
    FILE *stream = fopen(path, "wb"); assert(stream);
    size_t count = strlen(text);
    assert(fwrite(text, 1, count, stream) == count && fclose(stream) == 0);
}
/** Create independently useful active definitions and a deliberately unfinished
 * draft. #error native companions make accidental C compilation observable. */
static void prepare(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/unused", "@!\nvalue&\n./main\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/helper", "@<\nvalue\n.T\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/marker", "@...\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/draft", "unfinished !\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/native", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/native.abi", "+ initialize\n- destroy\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/native.c", "#error Must not compile native code during domain inspection\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/native.h", "#error Must not compile native headers during domain inspection\n");
}
/** Decode actual command output through Python's independent standard JSON parser. */
static void verify(const char *mode) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "verify.py", mode, (char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Create a captured request without creating any of its future source paths. */
static void snapshot_fixture(void) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "../../delivery/source/snapshot/generate.py", (char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** No entry operand is needed and unused/open/native definitions are retained. */
static void it_dispatches_the_complete_domain_without_an_entry_or_native_execution(void) {
    prepare(); setup_test();
    FILE *document = fopen("generated/domain.json", "wb"); assert(document);
    char *arguments[] = {"ama", "--source-domain", "generated/project"};
    int result = driver_run(3, arguments, document, errors);
    assert(result == EXIT_SUCCESS && fclose(document) == 0);
    char message[1024]; read_stream(errors, message, sizeof(message)); assert(!message[0]);
    verify("populated");
    destroy_test();
}
/** Root exclusions use ordinary catalog selection and invalid unused modules
 * cannot hide behind any selected executable entry. */
static void it_refuses_unused_errors_and_respects_explicit_exclusions(void) {
    prepare(); setup_test();
    file("generated/project/packages/test/fixture/v1.0.0/src/unused", "@!\nmissing !\n");
    char *arguments[] = {"ama", "--source-domain", "generated/project"};
    int result = driver_run(3, arguments, output, errors);
    assert(result == EXIT_FAILURE && ftell(output) == 0);
    char message[4096]; read_stream(errors, message, sizeof(message)); assert(strstr(message, "unused"));
    destroy_test();
    setup_test();
    char *excluded[] = {"ama", "--source-domain", "generated/project", "--exclude", "packages/test/fixture/v1.0.0/src/unused"};
    result = driver_run(5, excluded, output, errors); assert(result == EXIT_SUCCESS);
    destroy_test();
}
/** Empty version trees remain valid domains; missing or incomplete request/root
 * arguments and malformed source files fail without a success document. */
static void it_accepts_empty_projects_and_refuses_invalid_roots_or_requests(void) {
    fixture_package_prepare("generated/empty", "fixture"); setup_test();
    FILE *document = fopen("generated/domain.json", "wb"); assert(document);
    char *arguments[] = {"ama", "--source-domain", "generated/empty"};
    int result = driver_run(3, arguments, document, errors);
    assert(result == EXIT_SUCCESS && fclose(document) == 0); verify("empty"); destroy_test();
    const char *root[] = {"generated/not-present", ""};
    for (size_t index = 0; index < 2; index++) {
        setup_test(); arguments[2] = (char *)root[index];
        result = driver_run(3, arguments, output, errors);
        assert(result == EXIT_FAILURE && ftell(output) == 0); destroy_test();
    }
    setup_test(); result = driver_run(2, arguments, output, errors);
    assert(result == EXIT_FAILURE && ftell(output) == 0); destroy_test();
}
/** Complete checking does not convert output/diagnostic stream failures into
 * successful protocol acknowledgement or take ownership of those streams. */
static void it_propagates_source_domain_output_and_flush_failures(void) {
    prepare(); setup_test();
    char *arguments[] = {"ama", "--source-domain", "generated/project"};
    FILE *refusal = fopen("main.c", "rb"); assert(refusal);
    int result = driver_run(3, arguments, refusal, errors);
    assert(result == EXIT_FAILURE && fseek(refusal, 0, SEEK_SET) == 0 && fclose(refusal) == 0);
    FILE *full = fopen("/dev/full", "wb"); assert(full);
    result = driver_run(3, arguments, full, errors);
    assert(result == EXIT_FAILURE); (void)fclose(full);
    destroy_test();
}
/** Actual dispatch uses the explicit identity/byte table even though its code
 * and native paths are unpublished, preserving the ordinary checked result. */
static void it_dispatches_prepared_source_snapshot_without_root_or_native_path_probes(void) {
    snapshot_fixture(); setup_test();
    assert(access("not-published", F_OK) != 0);
    FILE *document = fopen("generated/domain.json", "wb"); assert(document);
    char *arguments[] = {"ama", "--source-domain", "--snapshot", "generated/base.json"};
    int result = driver_run(4, arguments, document, errors);
    assert(result == EXIT_SUCCESS && fclose(document) == 0 && access("not-published", F_OK) != 0);
    char message[1024]; read_stream(errors, message, sizeof(message)); assert(!message[0]);
    verify("snapshot"); destroy_test();
}
/** Request-file, schema, source, native and physical-alias failures never write
 * a success document or retry with a filesystem-root interpretation. */
static void it_refuses_invalid_source_snapshot_files_models_and_checked_code(void) {
    snapshot_fixture();
    const char *paths[] = {"generated/not-present.json", "generated", "generated/invalid-format.json",
        "generated/bad-source.json", "generated/bad-source-utf8.json", "generated/bad-native-abi.json",
        "generated/aliased-metadata.json"};
    for (size_t index = 0; index < sizeof(paths) / sizeof(*paths); index++) {
        setup_test();
        char *arguments[] = {"ama", "--source-domain", "--snapshot", (char *)paths[index]};
        int result = driver_run(4, arguments, output, errors);
        assert(result == EXIT_FAILURE && ftell(output) == 0);
        char message[4096]; read_stream(errors, message, sizeof(message)); assert(message[0]);
        destroy_test();
    }
}
/** Both completed probe states publish the same complete independently decoded domain. */
static void it_reports_exact_executable_and_open_captured_entry_completions(void) {
    snapshot_fixture();
    const char *entries[] = {"/vendor/pkg/v1.0.0/src/main", "/vendor/pkg/v1.0.0/src/helper"};
    for (size_t index = 0; index < 2; index++) {
        setup_test(); FILE *document = fopen("generated/domain.json", "wb"); assert(document);
        char *arguments[] = {"ama", "--source-domain", "--snapshot", "generated/base.json", "--probe-entry", (char *)entries[index]};
        int result = driver_run(6, arguments, document, errors);
        assert(result == (index ? DELIVERY_SOURCE_PROBE_OPEN_EXIT : EXIT_SUCCESS) && fclose(document) == 0);
        char message[1024]; read_stream(errors, message, sizeof(message)); assert(!message[0]);
        verify("snapshot"); destroy_test();
    }
    setup_test(); FILE *full = fopen("/dev/full", "wb"); assert(full);
    char *arguments[] = {"ama", "--source-domain", "--snapshot", "generated/base.json", "--probe-entry", (char *)entries[1]};
    assert(driver_run(6, arguments, full, errors) == EXIT_FAILURE);
    (void)fclose(full); destroy_test();
}
/** Missing/noncanonical operands, absent entries and real compiler errors never become Open. */
static void it_refuses_invalid_probe_requests_and_entry_errors_without_graph_output(void) {
    snapshot_fixture();
    char *arguments[] = {"ama", "--source-domain", "--snapshot", "generated/base.json", "--probe-entry", "//absent", "extra"};
    for (int count = 5; count <= 7; count++) {
        setup_test(); assert(driver_run(count, arguments, output, errors) == EXIT_FAILURE && ftell(output) == 0); destroy_test();
    }
    arguments[5] = ".invalid"; setup_test();
    assert(driver_run(6, arguments, output, errors) == EXIT_FAILURE && ftell(output) == 0); destroy_test();
    arguments[5] = "/vendor/pkg/v1.0.0/src/main"; arguments[3] = "generated/bad-source.json"; setup_test();
    assert(driver_run(6, arguments, output, errors) == EXIT_FAILURE && ftell(output) == 0); destroy_test();
}
/** Execute actual dispatch, schema, original-domain refusal and stream checks. */
void run(void) {
    it_dispatches_the_complete_domain_without_an_entry_or_native_execution();
    it_refuses_unused_errors_and_respects_explicit_exclusions();
    it_accepts_empty_projects_and_refuses_invalid_roots_or_requests();
    it_propagates_source_domain_output_and_flush_failures();
    it_dispatches_prepared_source_snapshot_without_root_or_native_path_probes();
    it_refuses_invalid_source_snapshot_files_models_and_checked_code();
    it_reports_exact_executable_and_open_captured_entry_completions();
    it_refuses_invalid_probe_requests_and_entry_errors_without_graph_output();
}
/** The C harness owns assertions and child status collection. */
int main(void) { run(); return 0; }
