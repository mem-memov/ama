/**
 * @file
 * @brief Stage complete project sources and retain only a verified linked executable.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * Use a private temporary directory; failure clears ready state and prevents publication. Temporary cleanup is best effort.
 */
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"

/**
 * Write each complete in-memory artifact into the private flat build directory. Check byte count
 * and close; first failure prevents compilation and publication.
 */
static bool codegen_stage_sources(NativeScaffold *scaffold, const char *stage) {
    for (const ScaffoldArtifact *artifact = scaffold->first; artifact != NULL; artifact = artifact->next) {
        const char *name = strrchr(artifact->path, '/');
        const char *path = scaffold_join(scaffold, stage, "/", name == NULL ? artifact->path : name+1);
        if (path == NULL) { return false; }
        FILE *file = fopen(path, "wb");
        if (file == NULL) { return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot stage the project assembly", path); }
        size_t count = source_length(artifact->content);
        bool written = fwrite(source_bytes(artifact->content), 1, count, file) == count;
        int closed = fclose(file);
        if (!written || closed != 0) { return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot finish the project assembly", path); }
    }
    return true;
}

/**
 * Load the verified linked executable bytes and append one executable artifact for later
 * explicit publication. Existing scaffold owns bytes/record even on subsequent failure.
 */
static bool codegen_retain_program(NativeScaffold *scaffold, const char *stage) {
    const char *path = scaffold_join(scaffold, stage, "/", "program");
    const char *output = scaffold_join(scaffold, scaffold->directory, "/", "program");
    if (path == NULL || output == NULL) { return false; }
    const SourceText *content = NULL;
    SourceStatus loaded = source_manager_load(&scaffold->texts, path, &content);
    if (loaded != SOURCE_OK) {
        return scaffold_error(scaffold, loaded == SOURCE_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_IO_FAILURE, "Cannot retain the verified executable", path);
    }
    ScaffoldArtifact *artifact = scaffold_allocate(scaffold, sizeof(*artifact));
    if (artifact == NULL) { return false; }
    *artifact = (ScaffoldArtifact){.path = output, .content = content, .executable = true};
    scaffold->last->next = artifact;
    scaffold->last = artifact;
    return true;
}

/**
 * Require ready unpublished source artifacts and no previously retained executable. Clear ready
 * while staging/compile/verify/link/retain proceeds; set it only after success and best-effort
 * temporary cleanup.
 */
bool project_codegen_compile(NativeScaffold *scaffold, FILE *errors) {
    if (!scaffold->ready || scaffold->publish_attempted || scaffold->last->executable) { return false; }
    scaffold->ready = false;
    char directory[] = "/tmp/ama-project-XXXXXX";
    if (mkdtemp(directory) == NULL) { return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot create native build storage", NULL); }
    bool built = codegen_stage_sources(scaffold, directory)
        && codegen_compile_object(scaffold, directory, errors) && codegen_verify_object(scaffold, directory, errors)
        && codegen_link_object(scaffold, directory, errors) && codegen_retain_program(scaffold, directory);
    DIR *stream = opendir(directory);
    if (stream != NULL) {
        struct dirent *entry;
        int descriptor = dirfd(stream);
        while ((entry = readdir(stream)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) { (void)unlinkat(descriptor, entry->d_name, 0); }
        }
        closedir(stream);
    }
    (void)rmdir(directory);
    scaffold->ready = built;
    return built;
}
