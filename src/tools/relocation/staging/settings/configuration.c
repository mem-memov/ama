/** @file
 * @brief Rewrite only parsed library module fields and validate both configuration models.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * covers documented identity settings; [EMBED-01](../../../../../spec/implementation/c-library.md#embed-01)
 * supplies the ordinary explicit library declaration model.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"
#include "../../internal.h"

/** Resolve a documented configuration module field in the same augmented original domain as Ama sources. */
static const RelocationModule *target(RelocationStaging *staging, const char *name) {
    for (const RelocationModule *module = staging->references->preview->modules; module != NULL; module = module->next) {
        const char *original = module->before_identity == NULL ? module->after_identity : module->before_identity;
        if (strcmp(original,name) == 0) { return module; }
    }
    return NULL;
}
/** Insert one parsed field replacement in original byte order; record separators are never selected. */
static bool add_edit(RelocationStaging *staging, RelocationPathEdit **edits, size_t *count,
    SourceSpan site, const char *replacement, const RelocationModule *module) {
    RelocationPathEdit *item = source_arena_allocate(&staging->arena,sizeof(*item));
    if (item == NULL) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Library field replacement allocation failed",site);
    }
    *item = (RelocationPathEdit){.original=site,.replacement=replacement,.target=module};
    RelocationPathEdit **position = edits;
    while (*position != NULL && (*position)->original.start < site.start) { position = &(*position)->next; }
    item->next = *position;
    *position = item;
    (*count)++;
    return true;
}
/** Resolve only declared module fields and preserve their original spelling when the identity is unchanged. */
static bool edit(RelocationStaging *staging, RelocationPathEdit **edits, size_t *count, SourceSpan site, const char *name) {
    const RelocationModule *module = target(staging,name);
    if (module == NULL) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_REFERENCE,"A library configuration names an unavailable original module",site);
    }
    return strcmp(name,module->after_identity) == 0 || add_edit(staging,edits,count,site,module->after_identity,module);
}
/** Preserve each explicit foreign-file referent across relocation and future invocation directories. */
static bool physical_edit(RelocationStaging *staging, RelocationPathEdit **edits, size_t *count, const LibrarySetting *setting) {
    if (strcmp(setting->kind,"link-file") != 0 && strcmp(setting->kind,"dependency-file") != 0) { return true; }
    bool existed;
    const char *before = relocation_staging_setting_path(staging,setting->value,false,&existed);
    if (before == NULL) { return false; }
    const char *after = relocation_project_to(&staging->arena,staging->references->preview,before);
    if (after == NULL) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Library dependency placement allocation failed",setting->value_site);
    }
    RelocationStagedFile *file = relocation_staging_capture(staging,before,after,NULL);
    if (file == NULL) { return false; }
    if (!S_ISREG(file->metadata.st_mode)) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"A library file dependency must be a regular file",setting->value_site);
    }
    file->protected_input = true;
    return strcmp(setting->value,after) == 0 || add_edit(staging,edits,count,setting->value_site,after,NULL);
}
/** Ordinary configuration/type/schema errors retain their original or final source locations. */
static bool checked(RelocationStaging *staging, LibraryBuild *build, const SourceText *text, const SemanticProgram *program) {
    if (library_configuration_parse(build,text) && library_configuration_validate(build,program)) { return true; }
    staging->status = build->error.code == DIAGNOSTIC_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY : RELOCATION_INVALID_SOURCE;
    staging->error = build->error;
    return false;
}
/** Capture recognized configuration explicitly; preserve flags and rebase only documented identity/file fields. */
bool relocation_staging_configuration(RelocationStaging *staging, const char *input) {
    bool existed;
    const char *before = relocation_staging_setting_path(staging,input,false,&existed);
    if (before == NULL) { return false; }
    const char *after = relocation_project_to(&staging->arena,staging->references->preview,before);
    if (after == NULL) { return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Library configuration placement allocation failed",(SourceSpan){0}); }
    for (const RelocationStagedConfiguration *other = staging->configurations; other != NULL; other = other->next) {
        if (strcmp(other->file->before,before) == 0) {
            return relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"A library configuration is supplied more than once",(SourceSpan){0});
        }
    }
    for (const RelocationStagedSource *source = staging->first; source != NULL; source = source->next) {
        if (strcmp(source->module->before_path,before) == 0) {
            return relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"A library configuration cannot also be an Ama source candidate",(SourceSpan){0});
        }
    }
    RelocationStagedFile *file = relocation_staging_capture(staging,before,after,NULL);
    if (file == NULL) { return false; }
    if (!S_ISREG(file->metadata.st_mode)) {
        return relocation_staging_fail(staging,RELOCATION_INVALID_PATH,"A library configuration must be a regular file",(SourceSpan){0});
    }
    file->protected_input = true;
    RelocationStagedConfiguration *configuration = source_arena_allocate(&staging->arena,sizeof(*configuration));
    if (configuration == NULL) { return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Library configuration owner allocation failed",(SourceSpan){0}); }
    *configuration = (RelocationStagedConfiguration){.file=file,.next=staging->configurations};
    library_build_init(&configuration->before,staging->arena.allocator);
    library_build_init(&configuration->after,staging->arena.allocator);
    staging->configurations = configuration;
    if (!checked(staging,&configuration->before,file->original,&staging->references->semantic)) { return false; }
    RelocationPathEdit *edits = NULL;
    size_t count = 0;
    for (const LibraryExport *exported = configuration->before.exports; exported != NULL; exported = exported->next) {
        if (!edit(staging,&edits,&count,exported->module_site,exported->module)) { return false; }
    }
    for (const LibrarySchema *schema = configuration->before.schemas; schema != NULL; schema = schema->next) {
        if (schema->kind != LIBRARY_LIST && !edit(staging,&edits,&count,schema->argument_site,schema->argument)) { return false; }
    }
    for (const LibrarySetting *setting = configuration->before.settings; setting != NULL; setting = setting->next) {
        if (!physical_edit(staging,&edits,&count,setting)) { return false; }
    }
    if (count > SIZE_MAX/sizeof(RelocationPathEdit *)) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Too many library field replacements",(SourceSpan){0});
    }
    const RelocationPathEdit **ordered = source_arena_allocate(&staging->arena,count*sizeof(*ordered));
    if (ordered == NULL) { return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Library field order allocation failed",(SourceSpan){0}); }
    size_t index = 0;
    for (RelocationPathEdit *item = edits; item != NULL; item = item->next) { ordered[index++] = item; }
    return relocation_staging_rewrite(staging,file->original,after,ordered,count,&file->replacement)
        && checked(staging,&configuration->after,file->replacement,&staging->semantic);
}
