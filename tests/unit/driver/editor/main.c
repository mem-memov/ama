/** @file
 * @brief Exercise actual driver snapshots/rename plans, exact original bytes and every measured production allocation refusal.
 */
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../source/common.c"
#include "driver/driver.h"
#include "driver/commands/editor/editor.h"
#include "semantic/lists/interface.h"

static const char original[] = "@!\r\nстарое имя\r\n.cell\r\nстарое имя ! # старое имя  \r\n";
/** Maintain explicit original bytes; fixtures never ask a producer to predict its own semantic output. */
static void write_file(const char *path, const char *text) {
    FILE *stream = fopen(path,"wb");
    assert(stream != NULL && fwrite(text,1,strlen(text),stream) == strlen(text) && fclose(stream) == 0);
}
/** The selected module and its real dependency are stable across read-only command invocations. */
static void fixture(void) {
    assert(mkdir("generated/project",0700) == 0);
    write_file("generated/project/main",original);
    write_file("generated/project/cell","@!\n");
}
/** Source tooling never applies the editor's document transaction itself. */
static void unchanged(void) {
    FILE *stream = fopen("generated/project/main","rb");
    assert(stream != NULL);
    char bytes[sizeof(original)+1];
    size_t length = fread(bytes,1,sizeof(bytes),stream);
    assert(length == sizeof(original)-1 && memcmp(bytes,original,length) == 0 && fclose(stream) == 0);
}
/** The C harness owns execution/assertions; the standard JSON decoder independently checks transport semantics. */
static void verify(const char *mode, const char *path) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) { execlp("python3","python3","verify.py",mode,path,(char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child,&status,0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Checked metadata and Unicode/CRLF/name edits remain exact and all borrowed output streams remain caller-owned. */
static void it_exposes_checked_unicode_snapshots_and_versioned_rename_plans(void) {
    fixture();
    char offset[64];
    snprintf(offset,sizeof(offset),"%zu",strlen("@!\r\nстарое имя\r\n.cell\r\n"));
    char *query[] = {"ama","--editor","generated/project",".main","--offset",offset};
    FILE *output = fopen("generated/editor.json","wb"), *errors = tmpfile();
    assert(output != NULL && errors != NULL);
    assert(driver_run(6,query,output,errors) == EXIT_SUCCESS);
    assert(fclose(output) == 0 && fclose(errors) == 0);
    verify("editor","generated/editor.json");
    char *rename[] = {"ama","--rename-plan","generated/project",".main","--offset",offset,"--name","новое имя"};
    output = fopen("generated/rename.json","wb"); errors = tmpfile();
    assert(output != NULL && errors != NULL);
    assert(driver_run(8,rename,output,errors) == EXIT_SUCCESS);
    assert(fclose(output) == 0 && fclose(errors) == 0);
    verify("rename","generated/rename.json");
    unchanged();
}
/** Intentionally incomplete/invalid source exposes lexical observations and explicit state, never guessed semantic metadata. */
static void it_reports_draft_and_invalid_source_without_fabricating_definitions(void) {
    const char *bodies[] = {"multi word # trailing\r\n # indented\r\n|#  literal  \r\n", "@!\nvalue\n"};
    const char *modes[] = {"draft","invalid"};
    for (size_t index = 0; index < 2; index++) {
        write_file("generated/project/main",bodies[index]);
        char *arguments[] = {"ama","--editor","generated/project",".main","--offset","0"};
        FILE *output = fopen("generated/state.json","wb"), *errors = tmpfile();
        assert(output != NULL && errors != NULL);
        assert(driver_run(6,arguments,output,errors) == EXIT_SUCCESS);
        assert(fclose(output) == 0 && fclose(errors) == 0);
        verify(modes[index],"generated/state.json");
    }
    write_file("generated/project/main",original);
}
/** Nominal labels, intrinsic methods and related error sites preserve their distinct source/type boundaries. */
static void it_exposes_nominal_intrinsic_and_related_diagnostic_metadata(void) {
    write_file("generated/project/worker","@<\nargument\n.cell\n@>\nresult&\n.cell\n@!\n");
    const char *bodies[] = {
        "@!\nvalue\n.cell\nworker\n.worker\nworker !\n argument < value\n result& > received&\n",
        "@!\nvalues[]\n.cell\nvalues[]\n append !\n  > element&\n",
        "@!\nvalue\n.cell\nvalue\n.cell\n"
    };
    const char *needles[] = {"argument <", "append !", "value"};
    const char *modes[] = {"binding","intrinsic","related"};
    for (size_t index = 0; index < 3; index++) {
        write_file("generated/project/main",bodies[index]);
        const char *found = strstr(bodies[index],needles[index]);
        assert(found != NULL);
        char offset[64];
        snprintf(offset,sizeof(offset),"%zu",(size_t)(found-bodies[index]));
        char *arguments[] = {"ama","--editor","generated/project",".main","--offset",offset,"--standard-root",TEST_STANDARD_ROOT};
        FILE *output = fopen("generated/metadata.json","wb"), *errors = tmpfile();
        assert(output != NULL && errors != NULL);
        assert(driver_run(8,arguments,output,errors) == EXIT_SUCCESS);
        assert(fclose(output) == 0 && fclose(errors) == 0);
        verify(modes[index],"generated/metadata.json");
    }
    write_file("generated/project/main",original);
}
/** Rejection preserves originals and source-backed diagnostics remain alive through rendering. */
static void it_refuses_invalid_names_and_outside_source_offsets(void) {
    char *arguments[] = {"ama","--rename-plan","generated/project",".main","--offset","4","--name","bad  name"};
    FILE *output = tmpfile(), *errors = tmpfile();
    assert(output != NULL && errors != NULL);
    assert(driver_run(8,arguments,output,errors) == EXIT_FAILURE);
    assert(ftell(output) == 0 && ftell(errors) > 0);
    assert(fclose(output) == 0 && fclose(errors) == 0);
    char *query[] = {"ama","--editor","generated/project",".main","--offset","9999"};
    output = tmpfile(); errors = tmpfile();
    assert(output != NULL && errors != NULL);
    assert(driver_run(6,query,output,errors) == EXIT_FAILURE);
    assert(ftell(output) == 0 && ftell(errors) > 0);
    assert(fclose(output) == 0 && fclose(errors) == 0);
    unchanged();
}
/** Every displayed intrinsic spelling comes from the same descriptor that accepts its source name. */
static void it_names_intrinsic_methods_from_canonical_semantic_descriptors(void) {
    const char *names[] = {"append","insert","remove","clear","swap","length"};
    _Static_assert(sizeof(names)/sizeof(*names) == SEMANTIC_LIST_METHOD_COUNT,"Every canonical intrinsic spelling is checked");
    setup_test();
    for (size_t index = 0; index < SEMANTIC_LIST_METHOD_COUNT; index++) {
        const SourceText *source;
        assert(source_manager_add(&sources,"method",(const unsigned char *)names[index],strlen(names[index]),&source) == SOURCE_OK);
        SemanticListMethod method;
        assert(semantic_list_method((SourceSpan){source,0,source_length(source)},&method));
        assert((size_t)method == index && strcmp(semantic_list_method_name(method),names[index]) == 0);
    }
    assert(semantic_list_method_name((SemanticListMethod)-1) == NULL && semantic_list_method_name(SEMANTIC_LIST_METHOD_COUNT) == NULL);
    destroy_test();
}
/** A syntactically valid replacement that duplicates an existing declaration must fail with live candidate diagnostics. */
static void it_rejects_a_valid_name_that_changes_checked_workspace_identity(void) {
    write_file("generated/project/main","@!\nfirst\n.cell\nsecond\n.cell\nfirst !\n");
    char *arguments[] = {"ama","--rename-plan","generated/project",".main","--offset","3","--name","second"};
    FILE *output = tmpfile(), *errors = tmpfile();
    assert(output != NULL && errors != NULL);
    assert(driver_run(8,arguments,output,errors) == EXIT_FAILURE);
    assert(ftell(output) == 0 && ftell(errors) > 0);
    assert(fclose(output) == 0 && fclose(errors) == 0);
    write_file("generated/project/main",original);
}
/** Pure read-only commands can repeat against one fixed input, permitting exact allocation-ordinal enumeration without changing identity bytes. */
static void it_releases_every_failed_snapshot_and_rename_command_prefix(void) {
    for (unsigned rename = 0; rename < 2; rename++) {
        setup_test();
        DriverOptions options;
        driver_options_init(&options,sources.allocator);
        ModuleRoot root = {.path="generated/project",.kind=MODULE_ROOT_PROJECT};
        options.request = rename ? DRIVER_REQUEST_RENAME_PLAN : DRIVER_REQUEST_EDITOR;
        options.roots = &root; options.root_count = 1;
        options.entry = options.query_module = ".main";
        options.valid = true;
        options.rename_name = "новое имя"; options.has_offset = true; options.offset = 4;
        FILE *output = tmpfile(), *errors = tmpfile();
        assert(output != NULL && errors != NULL);
        allocation_count = 0;
        assert(driver_editor(&options,output,errors) == EXIT_SUCCESS);
        size_t measured = allocation_count;
        assert(live_allocations == 0 && fclose(output) == 0 && fclose(errors) == 0);
        fprintf(stderr,"source %s allocation checkpoints: %zu\n",rename ? "rename" : "editor",measured);
        for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
            output = tmpfile(); errors = tmpfile();
            assert(output != NULL && errors != NULL);
            allocation_count = 0; failure_at = ordinal;
            assert(driver_editor(&options,output,errors) == EXIT_FAILURE);
            failure_at = 0;
            assert(live_allocations == 0 && fclose(output) == 0 && fclose(errors) == 0);
        }
        driver_options_destroy(&options);
        destroy_test();
        unchanged();
    }
}
/** A read-only sink cannot turn completed source analysis into acknowledged command success. */
static void it_refuses_output_failure_without_applying_any_document_changes(void) {
    for (unsigned rename = 0; rename < 2; rename++) {
        FILE *output = fopen("main.c","rb"), *errors = tmpfile();
        assert(output != NULL && errors != NULL);
        char *arguments[] = {"ama",rename ? "--rename-plan" : "--editor","generated/project",".main","--offset","4","--name","новое имя"};
        assert(driver_run(rename ? 8 : 6,arguments,output,errors) == EXIT_FAILURE);
        assert(fclose(output) == 0 && fclose(errors) == 0);
        unchanged();
    }
}
/** Each independent source-tool effect and refusal is explicit in the C harness. */
void run(void) {
    it_exposes_checked_unicode_snapshots_and_versioned_rename_plans();
    it_reports_draft_and_invalid_source_without_fabricating_definitions();
    it_exposes_nominal_intrinsic_and_related_diagnostic_metadata();
    it_refuses_invalid_names_and_outside_source_offsets();
    it_names_intrinsic_methods_from_canonical_semantic_descriptors();
    it_rejects_a_valid_name_that_changes_checked_workspace_identity();
    it_releases_every_failed_snapshot_and_rename_command_prefix();
    it_refuses_output_failure_without_applying_any_document_changes();
}
/** Successful execution cleans fixture output through the ordinary leaf Makefile. */
int main(void) { run(); return EXIT_SUCCESS; }
