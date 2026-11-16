/** @file
 * @brief Own minimal real compiler-side plans for independently copied relocation reports.
 */
#ifndef AMA_RELOCATION_REPORT_TEST_COMMON
#define AMA_RELOCATION_REPORT_TEST_COMMON
#include "../common.c"
#include "tools/relocation/staging/staging.h"
static RelocationReferences references;
static RelocationStaging staging;
static ModuleRoot roots[2];
static RelocationRequest move_request;
/** Prepare a selected source and an unused external importer through ordinary analysis. */
static void prepare(const char *name) {
    setup(name);
    folder("project"); folder("project/old"); folder("imports");
    file("project/old/a","@!\n");
    file("imports/unused","@!\nvalue\n.old.a\n");
    roots[0] = (ModuleRoot){.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    roots[1] = (ModuleRoot){.path=path("imports"),.kind=MODULE_ROOT_PROJECT};
    move_request = (RelocationRequest){roots,2,path("project/old"),path("project/new"),NULL};
    assert(relocation_preview_build(&preview,&move_request));
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    relocation_staging_init(&staging,sources.allocator);
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references}));
    assert(module(".old.a") != NULL);
}
/** Retire compiler borrowers while preserving the independent report's allocator authority. */
static void retire_plan(void) {
    untouched(&move_request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    reset_preview();
}
/** Retire all fixture state only after independently owned reports are destroyed. */
static void done(void) { retire_plan(); finish(); }
#endif
