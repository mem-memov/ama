/** @file
 * @brief Capture exact original file bytes and identity metadata without publishing replacements.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires revalidation against captured originals before application.
 */
#include <string.h>
#include "../internal.h"
/** Failure locations borrow retained input paths and require no diagnostic allocation. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static RelocationStagedFile *fail(RelocationStaging *staging, RelocationStatus status, const char *message, const char *path) {
    relocation_staging_fail(staging,status,message,(SourceSpan){0});
    if (staging->error.primary.path == NULL && staging->error.primary.span.source == NULL) { staging->error.primary.path = path; }
    return NULL;
}
/** Match existing records by exact verified physical path; inconsistent proposed placement rejects. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
RelocationStagedFile *relocation_staging_capture(RelocationStaging *staging, const char *before,
    const char *after, const struct stat *expected) {
    for (RelocationStagedFile *file = staging->files; file != NULL; file = file->next) {
        if (strcmp(file->before,before) != 0) { continue; }
        if (strcmp(file->after,after) != 0 || (expected != NULL && !relocation_staging_metadata(expected,&file->metadata))) {
            return fail(staging,RELOCATION_CHANGED_INPUT,"An input has inconsistent captured placements or metadata",before);
        }
        return file;
    }
    struct stat metadata;
    if (lstat(before,&metadata) != 0) {
        return fail(staging,RELOCATION_IO_ERROR,"An original input could not be inspected",before);
    }
    if (expected != NULL && !relocation_staging_metadata(expected,&metadata)) {
        return fail(staging,RELOCATION_CHANGED_INPUT,"An original input changed after discovery",before);
    }
    for (const RelocationStagedFile *other = staging->files; other != NULL; other = other->next) {
        if (other->metadata.st_dev == metadata.st_dev && other->metadata.st_ino == metadata.st_ino) {
            fail(staging,RELOCATION_ALIAS,"Distinct captured paths alias the same filesystem object",before);
            staging->related = (DiagnosticRelated){.location={.path=other->before},.role="other physical alias"};
            staging->error.related = &staging->related;
            staging->error.related_count = 1;
            return NULL;
        }
    }
    if (!S_ISREG(metadata.st_mode) && !S_ISDIR(metadata.st_mode) && !S_ISLNK(metadata.st_mode)) {
        return fail(staging,RELOCATION_UNSUPPORTED_PATH,"An original input has an unsupported file type",before);
    }
    const SourceText *original = NULL;
    if (S_ISREG(metadata.st_mode)) {
        SourceStatus status = source_manager_load(&staging->sources,before,&original);
        if (status != SOURCE_OK) {
            return fail(staging,status == SOURCE_OUT_OF_MEMORY || status == SOURCE_LIMIT
                ? RELOCATION_OUT_OF_MEMORY : RELOCATION_IO_ERROR,"Original input bytes could not be captured",before);
        }
    } else if (S_ISLNK(metadata.st_mode) && !relocation_staging_link(staging,before,&original)) { return NULL; }
    struct stat checked;
    if (lstat(before,&checked) != 0 || !relocation_staging_metadata(&metadata,&checked)) {
        return fail(staging,RELOCATION_CHANGED_INPUT,"An input changed while its bytes were captured",before);
    }
    RelocationStagedFile *file = source_arena_allocate(&staging->arena,sizeof(*file));
    if (file == NULL) {
        return fail(staging,RELOCATION_OUT_OF_MEMORY,"Original input record allocation failed",before);
    }
    *file = (RelocationStagedFile){.before=before,.after=after,.metadata=metadata,.original=original,.replacement=original};
    if (staging->last_file == NULL) { staging->files = file; }
    else { staging->last_file->next = file; }
    staging->last_file = file;
    return file;
}
