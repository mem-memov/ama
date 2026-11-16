/** @file
 * @brief Verify relocation driver publication/acknowledgement boundaries, exact read-only plans and retained unknown outcome diagnostics.
 */
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../source/common.c"
#include "driver/driver.h"
#include "driver/commands/relocation/relocation.h"

static SourceArena paths;
static const char *base;
/** Fixture strings use independent allocation authority so production fault ordinals do not fail test setup. */
static const char *path(const char *suffix) {
    const char *value = source_arena_join(&paths,base,"/",suffix);
    assert(value != NULL);
    return value;
}
/** Exact fixture byte writes never invoke source generation or constructors. */
static void file(const char *suffix, const char *text) {
    FILE *stream = fopen(path(suffix),"wb");
    assert(stream != NULL && fwrite(text,1,strlen(text),stream) == strlen(text) && fclose(stream) == 0);
}
/** Own one complete project and explicit separate recovery storage. */
static void fixture(const char *name) {
    setup_test();
    source_arena_init(&paths,source_system_allocator());
    base = source_arena_join(&paths,"generated","/",name);
    assert(base != NULL && mkdir(base,0700) == 0);
    assert(mkdir(path("project"),0700) == 0 && mkdir(path("project/old"),0700) == 0 && mkdir(path("recovery"),0700) == 0);
    file("project/old/a","@!\n");
    file("project/main","@!\nvalue\n.old.a\n");
}
/** Release test path authority only after every command owner has expired. */
static void finish(void) { source_arena_destroy(&paths); destroy_test(); }
/** No unacknowledged read-only command may move a source directory. */
static void original(void) {
    struct stat metadata;
    assert(lstat(path("project/old/a"),&metadata) == 0);
    assert(lstat(path("project/new"),&metadata) != 0 && errno == ENOENT);
}
/** The C harness controls execution; standard JSON decoding is an independent transport oracle. */
static void verify(const char *mode, const char *output) {
    pid_t child = fork(); assert(child >= 0);
    if (child == 0) { execlp("python3","python3","verify.py",mode,output,(char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child,&status,0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** A validated plan lists exact source edits but never creates a recovery journal or destination. */
static void it_exposes_a_complete_read_only_relocation_plan(void) {
    fixture("preview");
    char *arguments[] = {"ama","--relocate-preview",(char *)path("project"),(char *)path("project/old"),"--destination",(char *)path("project/new")};
    const char *json = path("preview.json");
    FILE *output = fopen(json,"wb"), *errors = tmpfile();
    assert(output != NULL && errors != NULL && driver_run(6,arguments,output,errors) == EXIT_SUCCESS);
    assert(fclose(output) == 0 && fclose(errors) == 0);
    verify("preview",json); original(); finish();
}
/** Recover the exact known directory from isolated test-owned storage, not by guessing a live application's journal location. */
static const char *receipt(void) {
    DIR *directory = opendir(path("recovery")); assert(directory != NULL);
    const char *found = NULL;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) { continue; }
        assert(found == NULL);
        found = source_arena_join(&paths,path("recovery"),"/",entry->d_name); assert(found != NULL);
    }
    assert(closedir(directory) == 0 && found != NULL);
    return found;
}
/** A failed result write cannot undo or mislabel an already committed source move; its terminal receipt remains recoverable. */
static void it_preserves_committed_outcome_when_result_output_is_refused(void) {
    fixture("committed-output");
    char *arguments[] = {"ama","--relocate-apply",(char *)path("project"),(char *)path("project/old"),"--destination",(char *)path("project/new"),
        "--recovery",(char *)path("recovery")};
    FILE *output = fopen("main.c","rb"), *errors = tmpfile();
    assert(output != NULL && errors != NULL && driver_run(8,arguments,output,errors) == EXIT_FAILURE);
    assert(fclose(output) == 0 && fseek(errors,0,SEEK_SET) == 0);
    char message[4096]; size_t length = fread(message,1,sizeof(message)-1,errors); message[length] = 0;
    assert(strstr(message,"committed") != NULL && strstr(message,"receipt") != NULL && fclose(errors) == 0);
    struct stat metadata;
    assert(lstat(path("project/old"),&metadata) != 0 && errno == ENOENT && lstat(path("project/new/a"),&metadata) == 0);
    const char *journal = receipt();
    char *recover[] = {"ama","--relocate-recover",(char *)journal};
    const char *json = path("recovery.json");
    output = fopen(json,"wb"); errors = tmpfile();
    assert(output != NULL && errors != NULL && driver_run(3,recover,output,errors) == EXIT_SUCCESS);
    assert(fclose(output) == 0 && fclose(errors) == 0);
    verify("recover",json); finish();
}
/** Missing/foreign journal authority cannot be reported as an uncommitted or rolled-back transaction. */
static void it_keeps_an_unreadable_recovery_outcome_explicitly_unknown(void) {
    fixture("unknown");
    char *arguments[] = {"ama","--relocate-recover",(char *)path("recovery")};
    const char *json = path("unknown.json");
    FILE *output = fopen(json,"wb"), *errors = tmpfile();
    assert(output != NULL && errors != NULL && driver_run(3,arguments,output,errors) == EXIT_FAILURE);
    assert(fclose(output) == 0 && fclose(errors) == 0);
    verify("unknown",json); original(); finish();
}
/** New read-only preparation owners exhaust a fixed input's allocation trace without relying on changing fixture identities. */
static void it_releases_every_failed_command_preparation_prefix(void) {
    fixture("preview-allocations");
    DriverOptions options; driver_options_init(&options,sources.allocator);
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    options.request = DRIVER_REQUEST_RELOCATE_PREVIEW; options.valid = true;
    options.roots = &root; options.root_count = 1;
    options.relocation_source = path("project/old"); options.destination = path("project/new");
    FILE *output = tmpfile(), *errors = tmpfile(); assert(output != NULL && errors != NULL);
    allocation_count = 0;
    assert(driver_relocation(&options,output,errors) == EXIT_SUCCESS);
    size_t measured = allocation_count;
    assert(live_allocations == 0 && fclose(output) == 0 && fclose(errors) == 0);
    fprintf(stderr,"relocation command preparation allocations: %zu\n",measured);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        output = tmpfile(); errors = tmpfile(); assert(output != NULL && errors != NULL);
        allocation_count = 0; failure_at = ordinal;
        assert(driver_relocation(&options,output,errors) == EXIT_FAILURE);
        failure_at = 0;
        assert(live_allocations == 0 && fclose(output) == 0 && fclose(errors) == 0);
        original();
    }
    driver_options_destroy(&options); finish();
}
/** Actual mutation, reporting and resource refusal boundaries are explicit independent tests. */
void run(void) {
    it_exposes_a_complete_read_only_relocation_plan();
    it_preserves_committed_outcome_when_result_output_is_refused();
    it_keeps_an_unreadable_recovery_outcome_explicitly_unknown();
    it_releases_every_failed_command_preparation_prefix();
}
/** Every successful leaf releases counted owners and removes generated fixtures through Make. */
int main(void) { run(); return EXIT_SUCCESS; }
