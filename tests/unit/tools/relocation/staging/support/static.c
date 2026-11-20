/** @file
 * @brief Distinguish actual static producer paths from atomic parameter/result/alias names during staging.
 * @see [T193 contract](../../../../../../docs/implementation/static-relocation.md)
 */
#include "semantic/specializations/identity/key.h"

/** Full owner encoding includes captures; source-only static producer names must not enter the returned Type key. */
static void same_returned_type(const SemanticProgram *original, const SemanticProgram *final) {
    const SemanticType *before = semantic_find_type(original,"/test/fixture/v1.0.0/src/box");
    const SemanticType *after = semantic_find_type(final,"/test/fixture/v1.0.0/src/box");
    assert(before && after && before->specialization && after->specialization);
    IdentityRegistry identities;
    identity_registry_init(&identities,sources.allocator);
    QualifiedIdentity left, right;
    assert(semantic_type_key(original,before,&identities.arena,&left) == IDENTITY_OK);
    assert(semantic_type_key(final,after,&identities.arena,&right) == IDENTITY_OK);
    const IdentityName *first = NULL, *second = NULL;
    assert(identity_register(&identities,left,&first) == IDENTITY_OK);
    assert(identity_register(&identities,right,&second) == IDENTITY_OK);
    assert(first == second);
    identity_registry_destroy(&identities);
}

/** Moving only the producer rewrites its use, not descriptive static names or the returned ordinary origin. */
static void it_relocates_static_producers_without_rewriting_atomic_names_or_returned_origins(void) {
    setup("static-producer");
    package("project", "fixture");
    folder("project/packages/test/fixture/v1.0.0/src/old");
    file("project/packages/test/fixture/v1.0.0/src/value","@!\n");
    file("project/packages/test/fixture/v1.0.0/src/box","@<\nvalue\n.old.key-type\n@!\n");
    file("project/packages/test/fixture/v1.0.0/src/old/factory","@...\n.old.result-type\n/test/fixture/v1.0.0/src/box\n .old.key-type < /test/fixture/v1.0.0/src/value\n");
    file("project/packages/test/fixture/v1.0.0/src/main","# .old.factory ...old.result-type\r\n@!\r\ndata\r\n/test/fixture/v1.0.0/src/old/factory\r\n .old.result-type\r\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    bool valid = relocation_references_build(&references,&preview);
    if (!valid) { fprintf(stderr,"Static original relocation: %s\n",references.error.message); }
    assert(valid && references.ready);
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    valid = relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references,.entry="/test/fixture/v1.0.0/src/main"});
    if (!valid) { fprintf(stderr,"Static final relocation: %s\n",staging.error.message); }
    assert(valid && staging.ready);
    exact(prepared(&staging,"/test/fixture/v1.0.0/src/main")->text,"# .old.factory ...old.result-type\r\n@!\r\ndata\r\n/test/fixture/v1.0.0/src/new/factory\r\n .old.result-type\r\n");
    exact(prepared(&staging,"/test/fixture/v1.0.0/src/old/factory")->text,"@...\n.old.result-type\n/test/fixture/v1.0.0/src/box\n .old.key-type < /test/fixture/v1.0.0/src/value\n");
    assert(!semantic_find_type(&references.frontend.semantic,"/test/fixture/v1.0.0/src/old/factory"));
    assert(!semantic_find_type(&staging.frontend.semantic,"/test/fixture/v1.0.0/src/new/factory"));
    same_returned_type(&references.frontend.semantic,&staging.frontend.semantic);
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    finish();
}

/** Physical name+/ storage is transparent; nested constructors retain two independent complete captures after relocation. */
static void it_relocates_nested_constructor_origins_without_collapsing_concrete_captures(void) {
    setup("static-constructors");
    package("project", "fixture");
    folder("project/packages/test/fixture/v1.0.0/src/old");
    folder("project/packages/test/fixture/v1.0.0/src/old/storage+");
    file("project/packages/test/fixture/v1.0.0/src/old/storage","@!\n");
    file("project/packages/test/fixture/v1.0.0/src/left","@!\n");
    file("project/packages/test/fixture/v1.0.0/src/right","@!\n");
    file("project/packages/test/fixture/v1.0.0/src/old/storage+/box-kind","@<\nvalue\n.old.key-type\n@!\n");
    file("project/packages/test/fixture/v1.0.0/src/old/storage+/outer-kind","@<\ninner\n.inner-type\n@!\n");
    file("project/packages/test/fixture/v1.0.0/src/factory","@...\n.left\n/test/fixture/v1.0.0/src/old/storage/outer-kind\n .inner-type < /test/fixture/v1.0.0/src/old/storage/box-kind\n  .old.key-type < /test/fixture/v1.0.0/src/left\n.right\n/test/fixture/v1.0.0/src/old/storage/outer-kind\n .inner-type < /test/fixture/v1.0.0/src/old/storage/box-kind\n  .old.key-type < /test/fixture/v1.0.0/src/right\n");
    file("project/packages/test/fixture/v1.0.0/src/main","@!\nleft\n/test/fixture/v1.0.0/src/factory\n .left\nright\n/test/fixture/v1.0.0/src/factory\n .right\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/new-root"),NULL};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references,.entry="/test/fixture/v1.0.0/src/main"}));
    exact(prepared(&staging,"/test/fixture/v1.0.0/src/factory")->text,"@...\n.left\n/test/fixture/v1.0.0/src/new-root/storage/outer-kind\n .inner-type < /test/fixture/v1.0.0/src/new-root/storage/box-kind\n  .old.key-type < /test/fixture/v1.0.0/src/left\n.right\n/test/fixture/v1.0.0/src/new-root/storage/outer-kind\n .inner-type < /test/fixture/v1.0.0/src/new-root/storage/box-kind\n  .old.key-type < /test/fixture/v1.0.0/src/right\n");
    exact(prepared(&staging,"/test/fixture/v1.0.0/src/old/storage/box-kind")->text,"@<\nvalue\n.old.key-type\n@!\n");
    size_t old_boxes = 0, new_boxes = 0, old_outer = 0, new_outer = 0;
    for (const SemanticType *type = references.frontend.semantic.first; type; type = type->next) {
        old_boxes += strcmp(type->module->source->logical_path,"/test/fixture/v1.0.0/src/old/storage/box-kind") == 0;
        old_outer += strcmp(type->module->source->logical_path,"/test/fixture/v1.0.0/src/old/storage/outer-kind") == 0;
    }
    for (const SemanticType *type = staging.frontend.semantic.first; type; type = type->next) {
        new_boxes += strcmp(type->module->source->logical_path,"/test/fixture/v1.0.0/src/new-root/storage/box-kind") == 0;
        new_outer += strcmp(type->module->source->logical_path,"/test/fixture/v1.0.0/src/new-root/storage/outer-kind") == 0;
        assert(strncmp(type->module->source->logical_path,"/test/fixture/v1.0.0/src/old/",strlen("/test/fixture/v1.0.0/src/old/")) != 0);
    }
    assert(old_boxes == 2 && new_boxes == 2 && old_outer == 2 && new_outer == 2);
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    finish();
}
