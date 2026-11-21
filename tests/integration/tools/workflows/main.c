/** @file
 * @brief Exercise one complete owned workspace through source queries, a client document transaction, repeated verified builds and both relocation forms.
 * No external process can observe or edit this private fixture during document publication.
 */
#include "../debugger-client.c"
#include <dirent.h>
#include <sys/stat.h>

/** Bound fixture path construction by its actual destination and reject truncation. */
static void path(char *result, size_t capacity, const char *base, const char *suffix) {
    int length = snprintf(result, capacity, "%s/%s", base, suffix);
    tool_require(length > 0 && (size_t)length < capacity);
}
/** Copy a finite authored ordinary tree byte-for-byte, refusing unexpected filesystem forms. */
static void copy_tree(const char *source, const char *destination) {
    struct stat information;
    tool_require(lstat(source, &information) == 0);
    if (S_ISDIR(information.st_mode)) {
        tool_require(mkdir(destination, 0700) == 0);
        DIR *directory = opendir(source); tool_require(directory != NULL);
        struct dirent *entry;
        errno = 0;
        while ((entry = readdir(directory)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { continue; }
            char before[2048], after[2048];
            path(before, sizeof(before), source, entry->d_name);
            path(after, sizeof(after), destination, entry->d_name);
            copy_tree(before, after);
            errno = 0;
        }
        tool_require(errno == 0 && closedir(directory) == 0);
        return;
    }
    tool_require(S_ISREG(information.st_mode));
    FILE *input = fopen(source, "rb"), *output = fopen(destination, "wb");
    tool_require(input != NULL && output != NULL);
    unsigned char bytes[4096]; size_t length;
    while ((length = fread(bytes, 1, sizeof(bytes), input)) != 0) {
        tool_require(fwrite(bytes, 1, length, output) == length);
    }
    tool_require(!ferror(input) && fclose(input) == 0 && fclose(output) == 0);
}
/** Both configured ordinary roots and the explicitly selected standard library are inside the carried container. */
static void fixture(const char *base) {
    copy_tree("project", base);
    char directory[2048], destination[2048], source[2048];
    path(directory, sizeof(directory), base, "library"); tool_require(mkdir(directory, 0700) == 0);
    const char *files[] = {"integer", "integer.h", "integer.c", "integer.abi"};
    for (size_t index = 0; index < sizeof(files) / sizeof(*files); index++) {
        path(source, sizeof(source), TEST_STDLIB, files[index]);
        path(destination, sizeof(destination), directory, files[index]); copy_tree(source, destination);
    }
    path(destination, sizeof(destination), directory, "integer+"); tool_require(mkdir(destination, 0700) == 0);
    path(source, sizeof(source), TEST_STDLIB, "integer+/native");
    path(destination, sizeof(destination), directory, "integer+/native"); copy_tree(source, destination);
    path(destination, sizeof(destination), base, "app/packages/test/fixture/v1.0.0/src/old/asset.bin");
    FILE *output = fopen(destination, "wb"); tool_require(output != NULL);
    const unsigned char bytes[] = {0, 255, 13, 10, 7};
    tool_require(fwrite(bytes, 1, sizeof(bytes), output) == sizeof(bytes) && fclose(output) == 0);
    tool_require(mkdir("generated/recovery", 0700) == 0);
}
/** The independent decoder also acts as this fixture's exclusive document publisher, never as a production filesystem rename tool. */
static void verify(const char *mode, const char *base, const char *extra) {
    tool_require(fflush(output) == 0 && fseek(output, 0, SEEK_SET) == 0);
    pid_t child = fork(); tool_require(child >= 0);
    if (child == 0) {
        if (dup2(fileno(output), STDIN_FILENO) < 0) { _exit(127); }
        execlp("python3", "python3", "verify.py", mode, base, extra, (char *)NULL); _exit(127);
    }
    int status; tool_require(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** A complete command succeeds before any subsequent client observation or publication. */
static void command(char *const arguments[]) {
    int result = run_compiler_arguments(arguments);
    if (result != EXIT_SUCCESS) {
        char text[16384]; read_stream(errors, text, sizeof(text)); fprintf(stderr, "%s", text);
    }
    tool_require(result == EXIT_SUCCESS);
}
/** Compile from the current returned roots, then inspect and explicitly resume the current module twice. */
static void build_and_debug(const char *base, const char *entry, const char *name) {
    char root[2048], library[2048], directory[2048], binary[2048], responses[2048];
    path(root, sizeof(root), base, "app"); path(library, sizeof(library), base, "library");
    path(directory, sizeof(directory), "generated", name);
    path(binary, sizeof(binary), directory, "ama-debugger"); path(responses, sizeof(responses), directory, "responses.jsonl");
    setup_test();
    char *emit[] = {AMA_EXECUTABLE, "--emit", root, (char *)entry, "--out", directory,
        "--sdk", TEST_SDK, "--standard-root", library, "--compile", "--mode", "executable", "--component", "test.application", NULL};
    command(emit); destroy_test(); tool_build_debugger(directory, binary);
    setup_test();
    tool_require(tool_run_debugger(binary, responses, "run\ninspect\ninspect\nresume\ninspect\nresume\ninspect\nquit\n") == EXIT_SUCCESS);
    verify("debug", base, responses); destroy_test();
}
/** The real result receipt is passed back into the actual recovery command, twice, without a second source rewrite. */
static void recover(const char *receipt) {
    char journal[4096]; FILE *input = fopen(receipt, "rb"); tool_require(input != NULL);
    size_t length = fread(journal, 1, sizeof(journal) - 1, input);
    tool_require(!ferror(input) && length > 0 && length < sizeof(journal) - 1 && fclose(input) == 0); journal[length] = 0;
    for (unsigned index = 0; index < 2; index++) {
        setup_test(); char *arguments[] = {AMA_EXECUTABLE, "--relocate-recover", journal, NULL}; command(arguments); destroy_test();
    }
}
/** Query/rename all importers, change one logical path, then carry every root/native companion without changing identities and rebuild each result. */
static void it_composes_source_transactions_relocation_debugging_and_verified_rebuilds(void) {
    const char *base = "generated/исходная область", *moved = "generated/перенесённая область";
    fixture(base); setup_test(); verify("snapshot", base, ""); destroy_test();
    char root[2048], imports[2048], library[2048], source[2048], destination[2048], configuration[2048];
    path(root, sizeof(root), base, "app"); path(imports, sizeof(imports), base, "importers");
    path(library, sizeof(library), base, "library"); path(configuration, sizeof(configuration), base, "app/packages/test/fixture/v1.0.0/src/settings.library");
    setup_test();
    char *query[] = {AMA_EXECUTABLE, "--editor", root, "/test/fixture/v1.0.0/src/old/sequence", "--module", "/test/fixture/v1.0.0/src/palette", "--offset", "93",
        "--root", imports, "--standard-root", library, NULL};
    command(query); verify("editor", base, ""); destroy_test();
    setup_test();
    char *rename[] = {AMA_EXECUTABLE, "--rename-plan", root, "/test/fixture/v1.0.0/src/old/sequence", "--module", "/test/fixture/v1.0.0/src/palette", "--offset", "93",
        "--name", "размер шага", "--root", imports, "--standard-root", library, NULL};
    command(rename); verify("rename", base, ""); destroy_test();
    build_and_debug(base, "/test/fixture/v1.0.0/src/old/sequence", "before");
    path(source, sizeof(source), base, "app/packages/test/fixture/v1.0.0/src/old"); path(destination, sizeof(destination), base, "app/packages/test/fixture/v1.0.0/src/new");
    setup_test();
    char *move[] = {AMA_EXECUTABLE, "--relocate-preview", root, source, "--destination", destination,
        "--root", imports, "--standard-root", library, "--entry", "/test/fixture/v1.0.0/src/old/sequence", "--configuration", configuration,
        "--invalidate", "generated/before/program", NULL, NULL, NULL};
    command(move); verify("preview", base, ""); destroy_test();
    move[1] = "--relocate-apply"; move[16] = "--recovery"; move[17] = "generated/recovery";
    setup_test(); command(move); verify("logical", base, "generated/logical-receipt"); destroy_test(); recover("generated/logical-receipt");
    build_and_debug(base, "/test/fixture/v1.0.0/src/new/sequence", "after-logical");
    setup_test();
    char *carry[] = {AMA_EXECUTABLE, "--relocate-apply", root, (char *)base, "--destination", (char *)moved,
        "--root", imports, "--standard-root", library, "--entry", "/test/fixture/v1.0.0/src/new/sequence", "--configuration", configuration,
        "--invalidate", "generated/after-logical/program", "--recovery", "generated/recovery", NULL};
    command(carry); verify("container", moved, "generated/container-receipt"); destroy_test(); recover("generated/container-receipt");
    recover("generated/logical-receipt");
    build_and_debug(moved, "/test/fixture/v1.0.0/src/new/sequence", "after-container");
    setup_test(); verify("final", moved, ""); destroy_test();
}
/** Keep the complete cross-tool order visible in one explicitly named C/assert scenario. */
void run(void) { it_composes_source_transactions_relocation_debugging_and_verified_rebuilds(); }
/** All fixture sources remain handwritten; only private copies and generated products are published during the test. */
int main(void) { run(); return EXIT_SUCCESS; }
