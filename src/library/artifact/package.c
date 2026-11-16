/** @file
 * @brief Compile and isolate one complete component before static/shared packaging.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) forbids
 * ELF interposition between otherwise independent components. Localization acts
 * on definitions only; unresolved platform imports keep their original names.
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "package.h"
#include "process.h"
#include "verify.h"
#include "commands.h"
/** Validate every staging operand before opening files or starting a child. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
static bool options_valid(const LibraryPackageOptions *options) {
    if (options == NULL) { return false; }
    const char *paths[] = {options->compiler, options->archiver, options->object_copy,
        options->source, options->sdk, options->includes, options->raw_object,
        options->isolated_object, options->symbols, options->archive, options->shared};
    for (size_t index = 0; index < sizeof(paths) / sizeof(*paths); index++) {
        if (paths[index] == NULL || paths[index][0] == '\0') { return false; }
    }
    const char *files[] = {options->source, options->raw_object, options->isolated_object,
        options->symbols, options->archive, options->shared};
    for (size_t index = 0; index < sizeof(files) / sizeof(*files); index++) {
        for (size_t previous = 0; previous < index; previous++) {
            if (strcmp(files[index], files[previous]) == 0) { return false; }
        }
        if (index != 0) {
            struct stat status;
            if (lstat(files[index],&status)==0 || errno!=ENOENT) { return false; }
        }
    }
    return true;
}
bool library_package_build(const LibraryBuild *build, const LibraryPackageOptions *options, FILE *errors) {
    if (!build->valid || !library_header_target(build, NULL) || !options_valid(options)
        || options->assembly==NULL || !options->assembly->ready
        || options->assembly->frontend!=&build->frontend) { return false; }
    FILE *symbols = fopen(options->symbols, "wbx");
    if (symbols == NULL) { return false; }
    bool written = library_symbols_write(build, symbols);
    int closed = fclose(symbols);
    if (!written || closed != 0) { return false; }
    /* GNU objcopy's keep-global-symbols leaves undefined imports external and
     * changes every other definition to STB_LOCAL, including handwritten helpers. */
    size_t length = strlen(options->symbols);
    const char prefix[] = "--keep-global-symbols=";
    if (length > SIZE_MAX - sizeof(prefix)) { return false; }
    char *allowlist = malloc(sizeof(prefix) + length);
    if (allowlist == NULL) { return false; }
    memcpy(allowlist, prefix, sizeof(prefix) - 1);
    memcpy(allowlist + sizeof(prefix) - 1, options->symbols, length + 1);
    char *isolate[] = {(char *)options->object_copy, allowlist,
        (char *)options->raw_object, (char *)options->isolated_object, NULL};
    char *archive[] = {(char *)options->archiver, "rcsD", (char *)options->archive,
        (char *)options->isolated_object, NULL};
    bool built = library_package_compile(build, options, errors)
        && library_package_process(isolate, errors)
        && library_package_verify(build, options, errors)
        && library_package_process(archive, errors)
        && library_package_link(build, options, errors);
    free(allowlist);
    return built;
}
