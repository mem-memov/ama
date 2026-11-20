/** @file
 * @brief Distinguish actual static producer paths from atomic parameter/result/alias names during staging.
 * @see [T193 contract](../../../../../../docs/implementation/static-relocation.md)
 */
#include "semantic/specializations/identity/key.h"

/** Full owner encoding includes captures; source-only static producer names must not enter the returned Type key. */
static void same_returned_type(const SemanticProgram *original, const SemanticProgram *final) {
    const SemanticType *before = semantic_find_type(original,".box");
    const SemanticType *after = semantic_find_type(final,".box");
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
    folder("project");
    folder("project/old");
    file("project/value","@!\n");
    file("project/box","@<\nvalue\n...old.key-type\n@!\n");
    file("project/old/factory","@...\n...old.result-type\n.box\n ...old.key-type < .value\n");
    file("project/main","# .old.factory ...old.result-type\r\n@!\r\ndata\r\n.old.factory\r\n ...old.result-type\r\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/old"),path("project/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    bool valid = relocation_references_build(&references,&preview);
    if (!valid) { fprintf(stderr,"Static original relocation: %s\n",references.error.message); }
    assert(valid && references.ready);
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    valid = relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references,.entry=".main"});
    if (!valid) { fprintf(stderr,"Static final relocation: %s\n",staging.error.message); }
    assert(valid && staging.ready);
    exact(prepared(&staging,".main")->text,"# .old.factory ...old.result-type\r\n@!\r\ndata\r\n.new.factory\r\n ...old.result-type\r\n");
    exact(prepared(&staging,".old.factory")->text,"@...\n...old.result-type\n.box\n ...old.key-type < .value\n");
    assert(!semantic_find_type(&references.frontend.semantic,".old.factory"));
    assert(!semantic_find_type(&staging.frontend.semantic,".new.factory"));
    same_returned_type(&references.frontend.semantic,&staging.frontend.semantic);
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    finish();
}

/** Physical name+/ storage is transparent; nested constructors retain two independent complete captures after relocation. */
static void it_relocates_nested_constructor_origins_without_collapsing_concrete_captures(void) {
    setup("static-constructors");
    folder("project");
    folder("project/old");
    folder("project/old/storage+");
    file("project/old/storage","@!\n");
    file("project/left","@!\n");
    file("project/right","@!\n");
    file("project/old/storage+/box-kind","@<\nvalue\n...old.key-type\n@!\n");
    file("project/old/storage+/outer-kind","@<\ninner\n...inner-type\n@!\n");
    file("project/factory","@...\n...left\n.old.storage.outer-kind\n ...inner-type < .old.storage.box-kind\n  ...old.key-type < .left\n...right\n.old.storage.outer-kind\n ...inner-type < .old.storage.box-kind\n  ...old.key-type < .right\n");
    file("project/main","@!\nleft\n.factory\n ...left\nright\n.factory\n ...right\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/old"),path("project/new-root"),NULL};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references,.entry=".main"}));
    exact(prepared(&staging,".factory")->text,"@...\n...left\n.new-root.storage.outer-kind\n ...inner-type < .new-root.storage.box-kind\n  ...old.key-type < .left\n...right\n.new-root.storage.outer-kind\n ...inner-type < .new-root.storage.box-kind\n  ...old.key-type < .right\n");
    exact(prepared(&staging,".old.storage.box-kind")->text,"@<\nvalue\n...old.key-type\n@!\n");
    size_t old_boxes = 0, new_boxes = 0, old_outer = 0, new_outer = 0;
    for (const SemanticType *type = references.frontend.semantic.first; type; type = type->next) {
        old_boxes += strcmp(type->module->source->logical_path,".old.storage.box-kind") == 0;
        old_outer += strcmp(type->module->source->logical_path,".old.storage.outer-kind") == 0;
    }
    for (const SemanticType *type = staging.frontend.semantic.first; type; type = type->next) {
        new_boxes += strcmp(type->module->source->logical_path,".new-root.storage.box-kind") == 0;
        new_outer += strcmp(type->module->source->logical_path,".new-root.storage.outer-kind") == 0;
        assert(strncmp(type->module->source->logical_path,".old.",5) != 0);
    }
    assert(old_boxes == 2 && new_boxes == 2 && old_outer == 2 && new_outer == 2);
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    finish();
}
