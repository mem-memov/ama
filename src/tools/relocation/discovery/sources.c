/** @file
 * @brief Classify every original/final source through the production parser and preserve its bytes.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * includes unused and external importers; [SYN-04](../../../../spec/language/source-and-names.md#syn-04)
 * leaves draft declarations opaque after global lexical checks.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"

/** Parser diagnostics borrow original source/static expectation text, not discarded syntax nodes. */
bool relocation_source_kind(RelocationPreview *preview, const SourceText *text, bool *active) {
    AstModule syntax;
    ast_module_init(&syntax,preview->arena.allocator);
    ParseStatus status = parser_module_parse(&syntax,text);
    if (status != PARSE_OK) { preview->diagnostic = syntax.error.diagnostic; }
    else { *active = syntax.sections.kind != MODULE_DRAFT; }
    ast_module_destroy(&syntax);
    return status == PARSE_OK || relocation_fail(preview,status == PARSE_OUT_OF_MEMORY
        ? RELOCATION_OUT_OF_MEMORY : RELOCATION_INVALID_SOURCE,source_path(text),NULL);
}

/** Add one fully copied record after successful byte/activity checks; no source is executed. */
static RelocationModule *append_module(RelocationPreview *preview, const SourceText *text,
    const char *identity, dev_t device, ino_t inode) {
    bool active;
    if (!relocation_source_kind(preview,text,&active)) { return NULL; }
    RelocationModule *module = source_arena_allocate(&preview->arena,sizeof(*module));
    if (module == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,source_path(text),NULL); return NULL; }
    *module = (RelocationModule){.before_path=source_path(text),.before_identity=identity,
        .text=text,.device=device,.inode=inode,.active=active};
    if (preview->last_module == NULL) { preview->modules = module; }
    else { preview->last_module->next = module; }
    preview->last_module = module;
    return module;
}

/** The catalog enumerates all roots, not an entry-point dependency closure. */
bool relocation_original_modules(RelocationPreview *preview) {
    ModuleStatus status = module_catalog_build(&preview->catalog,preview->before_roots,preview->before_root_count);
    if (!relocation_catalog_failure(preview,status)) { return false; }
    for (const ModuleSource *source = preview->catalog.first; source != NULL; source = source->next) {
        if (append_module(preview,source->text,source->logical_path,source->device,source->inode) == NULL) { return false; }
    }
    return true;
}

/** Newly configured candidates retain exact original bytes; UTF-8 is a source rule even for drafts. */
static RelocationModule *additional_module(RelocationPreview *preview, const char *before, const struct stat *metadata) {
    const SourceText *text = NULL;
    SourceStatus status = source_manager_load(&preview->additional_sources,before,&text);
    if (status != SOURCE_OK) {
        relocation_fail(preview,status == SOURCE_IO_ERROR ? RELOCATION_IO_ERROR : RELOCATION_OUT_OF_MEMORY,before,NULL);
        return NULL;
    }
    size_t offset;
    if (!utf8_validate(source_bytes(text),source_length(text),&offset)) {
        preview->diagnostic = (Diagnostic){.code=DIAGNOSTIC_INVALID_UTF8,.message="Source is not valid UTF-8",
            .primary={{text,offset,offset+1},NULL}};
        relocation_fail(preview,RELOCATION_INVALID_SOURCE,before,NULL);
        return NULL;
    }
    return append_module(preview,text,NULL,metadata->st_dev,metadata->st_ino);
}

/** Complete final identity equality and physical inode equality are separate collision checks. */
bool relocation_final_module(RelocationPreview *preview, const char *before, const char *after,
    const char *identity, const struct stat *metadata) {
    RelocationModule *selected = NULL;
    for (RelocationModule *module = preview->modules; module != NULL; module = module->next) {
        if (module->after_identity != NULL && strcmp(module->after_identity,identity) == 0) {
            return relocation_fail(preview,RELOCATION_COLLISION,after,module->after_path);
        }
        if (module->after_identity != NULL && module->device == metadata->st_dev && module->inode == metadata->st_ino) {
            return relocation_fail(preview,RELOCATION_ALIAS,after,module->after_path);
        }
        if (strcmp(module->before_path,before) == 0) { selected = module; }
    }
    if (selected == NULL) { selected = additional_module(preview,before,metadata); }
    if (selected == NULL) { return false; }
    selected->after_path = after;
    selected->after_identity = identity;
    return true;
}
