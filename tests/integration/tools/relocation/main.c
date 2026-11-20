/** @file
 * @brief Run real preview/apply/recovery commands with external importers, recognized settings, exact opaque bytes and initial-root collision refusal.
 */
#include <sys/stat.h>
#include "../../common.c"
#include "../../../unit/fixtures/package.h"

/** Fixture paths are finite authored test data; every formatting operation checks its complete result. */
typedef struct {
    const char *base;
    char root[512], source[512], destination[512], imports[512], recovery[512], configuration[512], outside[512], inside[512];
} Fixture;
/** No shell quoting or namespace inference is involved in constructing explicit test arguments. */
static void path(char *output, size_t capacity, const char *base, const char *suffix) {
    int length = snprintf(output,capacity,"%s/%s",base,suffix);
    assert(length > 0 && (size_t)length < capacity);
}
/** Copy exact maintained source/CRLF/native bytes without rewriting the oracle input. */
static void copy(const char *base, const char *name) {
    char before[1024], after[1024]; path(before,sizeof(before),"project",name); path(after,sizeof(after),base,name);
    FILE *input = fopen(before,"rb"), *output = fopen(after,"wb");
    assert(input != NULL && output != NULL);
    unsigned char bytes[4096]; size_t length;
    while ((length = fread(bytes,1,sizeof(bytes),input)) != 0) { assert(fwrite(bytes,1,length,output) == length); }
    assert(!ferror(input) && fclose(input) == 0 && fclose(output) == 0);
}
/** Generate only test-owned opaque/cache bytes; original source files remain immutable fixtures. */
static void bytes(const char *base, const char *name, const void *data, size_t length) {
    char output[1024]; path(output,sizeof(output),base,name);
    FILE *stream = fopen(output,"wb"); assert(stream != NULL && fwrite(data,1,length,stream) == length && fclose(stream) == 0);
}
/** Create a fresh domain; another configured ordinary root contains an otherwise unreachable importer. */
static Fixture fixture(const char *base) {
    Fixture value = {.base=base};
    assert(mkdir(base,0700) == 0);
    const char *folders[] = {"root", "importers", "recovery"};
    for (size_t index = 0; index < sizeof(folders)/sizeof(*folders); index++) {
        char directory[1024]; path(directory,sizeof(directory),base,folders[index]); assert(mkdir(directory,0700) == 0);
    }
    char package_root[1024];
    path(package_root, sizeof(package_root), base, "root");
    fixture_package_prepare(package_root, "fixture");
    path(package_root, sizeof(package_root), base, "importers");
    fixture_package_prepare(package_root, "imports");
    path(package_root, sizeof(package_root), base, "root/packages/test/fixture/v1.0.0/src/old");
    assert(mkdir(package_root, 0700) == 0);
    const char *files[] = {"root/packages/test/fixture/v1.0.0/src/main","root/packages/test/fixture/v1.0.0/src/old/a","root/packages/test/fixture/v1.0.0/src/old/draft","root/packages/test/fixture/v1.0.0/src/old/helper.c","root/packages/test/fixture/v1.0.0/src/settings.library","importers/packages/test/imports/v1.0.0/src/unreachable"};
    for (size_t index = 0; index < sizeof(files)/sizeof(*files); index++) { copy(base,files[index]); }
    const unsigned char opaque[] = {0,255,13,10,0,7};
    bytes(base,"root/packages/test/fixture/v1.0.0/src/old/asset.bin",opaque,sizeof(opaque));
    bytes(base,"root/packages/test/fixture/v1.0.0/src/cache.bin","outside generated cache",23);
    bytes(base,"root/packages/test/fixture/v1.0.0/src/old/build.bin","inside generated cache",22);
    path(value.root,sizeof(value.root),base,"root"); path(value.source,sizeof(value.source),base,"root/packages/test/fixture/v1.0.0/src/old");
    path(value.destination,sizeof(value.destination),base,"root/packages/test/fixture/v1.0.0/src/new"); path(value.imports,sizeof(value.imports),base,"importers");
    path(value.recovery,sizeof(value.recovery),base,"recovery"); path(value.configuration,sizeof(value.configuration),base,"root/packages/test/fixture/v1.0.0/src/settings.library");
    path(value.outside,sizeof(value.outside),base,"root/packages/test/fixture/v1.0.0/src/cache.bin"); path(value.inside,sizeof(value.inside),base,"root/packages/test/fixture/v1.0.0/src/old/build.bin");
    return value;
}
/** Standard JSON decoding independently checks result semantics and exact whole-project bytes. */
static void verify(const char *mode, const char *base) {
    assert(fflush(output) == 0 && fseek(output,0,SEEK_SET) == 0);
    pid_t child = fork(); assert(child >= 0);
    if (child == 0) {
        if (dup2(fileno(output),STDIN_FILENO) < 0) { _exit(127); }
        execlp("python3","python3","verify.py",mode,base,(char *)NULL); _exit(127);
    }
    int status; assert(waitpid(child,&status,0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Preview, publication and receipt replay are distinct explicit processes, with a checked final project afterwards. */
static void it_accepts_complete_preview_apply_and_repeated_recovery_processes(void) {
    Fixture value = fixture("generated/проект переноса");
    char *arguments[] = {AMA_EXECUTABLE,"--relocate-preview",value.root,value.source,"--destination",value.destination,
        "--root",value.imports,"--entry","/test/fixture/v1.0.0/src/old/a","--configuration",value.configuration,
        "--invalidate",value.outside,"--invalidate",value.inside,NULL,NULL,NULL};
    setup_test(); assert(run_compiler_arguments(arguments) == EXIT_SUCCESS); verify("preview",value.base); destroy_test();
    arguments[1] = "--relocate-apply"; arguments[16] = "--recovery"; arguments[17] = value.recovery;
    setup_test(); assert(run_compiler_arguments(arguments) == EXIT_SUCCESS); verify("apply",value.base); destroy_test();
    char receipt_path[1024]; path(receipt_path,sizeof(receipt_path),value.base,"receipt.txt");
    FILE *input = fopen(receipt_path,"rb"); assert(input != NULL);
    char journal[2048]; size_t length = fread(journal,1,sizeof(journal)-1,input);
    assert(!ferror(input) && length > 0 && length < sizeof(journal)-1 && fclose(input) == 0); journal[length] = 0;
    char *recover[] = {AMA_EXECUTABLE,"--relocate-recover",journal,NULL};
    for (unsigned repeat = 0; repeat < 2; repeat++) {
        setup_test(); assert(run_compiler_arguments(recover) == EXIT_SUCCESS); verify("recover",value.base); destroy_test();
    }
    char *check[] = {AMA_EXECUTABLE,"--check",value.root,"/test/fixture/v1.0.0/src/main","--root",value.imports,NULL};
    setup_test(); assert(run_compiler_arguments(check) == EXIT_SUCCESS);
    char message[1024]; read_stream(output,message,sizeof(message)); assert(strcmp(message,"checked /test/fixture/v1.0.0/src/main\n") == 0); destroy_test();
}
/** Newly covered files join original analysis; an initial conflict rejects even when the final identities would be distinct. */
static void it_refuses_initial_collisions_in_an_explicit_new_destination_root(void) {
    Fixture value = fixture("generated/initial-collision");
    char target[1024], old[1024], destination[1024]; path(target,sizeof(target),value.base,"target");
    assert(mkdir(target,0700) == 0);
    fixture_package_prepare(target, "fixture");
    path(old,sizeof(old),value.base,"target/packages/test/fixture/v1.0.0/src/old");
    path(destination,sizeof(destination),value.base,"target/packages/test/fixture/v1.0.0/src/new");
    assert(mkdir(old,0700) == 0);
    bytes(value.base,"target/packages/test/fixture/v1.0.0/src/old/a","@!\n",3);
    char *arguments[] = {AMA_EXECUTABLE,"--relocate-preview",value.root,value.source,"--destination",destination,
        "--destination-root",target,NULL};
    setup_test(); assert(run_compiler_arguments(arguments) != EXIT_SUCCESS); verify("collision",value.base); destroy_test();
}
/** All real command effects are verified against maintained input bytes and an ordinary frontend reload. */
void run(void) {
    it_accepts_complete_preview_apply_and_repeated_recovery_processes();
    it_refuses_initial_collisions_in_an_explicit_new_destination_root();
}
/** Test-owned copies retire with the normal leaf; handwritten project inputs never change. */
int main(void) { run(); return EXIT_SUCCESS; }
