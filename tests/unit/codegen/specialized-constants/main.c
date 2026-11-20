/** @file
 * @brief Verify full specialization/member identity survives shared storage and generated naming.
 */
#include "../../compiler/common.c"
#include "codegen/internal.h"
#include "semantic/specializations/members/key.h"

/** Actual scaffold naming owns encodings after the frontend has resolved every Type. */
static NativeScaffold scaffold;

/** Produce two concrete namespaces and reuse one of them through another read. */
static void setup_constants(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\n/test/fixture/v1.0.0/src/factory\n .numbers\n first[] >> xs[]\n/test/fixture/v1.0.0/src/factory\n .texts\n second[] >> ys[]\n/test/fixture/v1.0.0/src/factory\n .numbers\n second[] >> again[]\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/container\n .slot < //integer\n.texts\n/test/fixture/v1.0.0/src/container\n .slot < //text\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/container", "@=\nfirst[]\n.slot\nsecond[]\n.slot\n@<\ninput\n.slot\n@!\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    native_scaffold_init(&scaffold, &frontend, sources.allocator);
}

/** Reverse ownership order proves all partial scaffold and frontend resources are released. */
static void destroy_constants(void) {
    native_scaffold_destroy(&scaffold);
    destroy_frontend();
}

/** Equal owners reuse one table; different captures retain distinct member names and storage. */
static void it_distinguishes_specialized_constant_names_and_reuses_equal_owners(void) {
    setup_constants();
    const ProjectModule *container = project_find(&frontend.project, "/test/fixture/v1.0.0/src/container");
    const SemanticConstantPlan *constants[4] = {NULL, NULL, NULL, NULL};
    size_t count = 0;
    for (const SemanticConstantPlan *constant = frontend.semantic.constant_first; constant; constant = constant->next) {
        if (constant->symbol->owner->module != container) { continue; }
        assert(count < 4);
        constants[count++] = constant;
    }
    assert(count == 4);
    SourceArena temporary;
    source_arena_init(&temporary, sources.allocator);
    QualifiedIdentity keys[4];
    const char *names[4];
    for (size_t index = 0; index < 4; index++) {
        assert(semantic_constant_key(&frontend.semantic, constants[index]->symbol, &temporary, &keys[index]) == IDENTITY_OK);
        assert(keys[index].category == IDENTITY_CONSTANT && keys[index].member_count == 3);
        assert(strcmp(keys[index].module_path, "/test/fixture/v1.0.0/src/container") == 0);
        assert(strcmp(keys[index].members[0], "ama-static-key-v1") == 0);
        SourceSpan member = constants[index]->symbol->name;
        assert(strlen(keys[index].members[2]) == member.end - member.start);
        assert(memcmp(keys[index].members[2], source_bytes(member.source) + member.start, member.end - member.start) == 0);
        for (size_t earlier = 0; earlier < index; earlier++) {
            bool same_owner = constants[index]->symbol->owner == constants[earlier]->symbol->owner;
            assert((strcmp(keys[index].members[1], keys[earlier].members[1]) == 0) == same_owner);
        }
        names[index] = codegen_constant_name(&scaffold, constants[index]);
        assert(names[index]);
        assert(strcmp(names[index], codegen_constant_name(&scaffold, constants[index])) == 0);
        for (size_t earlier = 0; earlier < index; earlier++) { assert(strcmp(names[index], names[earlier]) != 0); }
    }
    source_arena_destroy(&temporary);
    size_t storage_count = 0;
    for (const SharedStorage *storage = frontend.plan.shared_first; storage; storage = storage->next) {
        if (storage->constant->symbol->owner->module != container) { continue; }
        assert(storage_count < 4 && storage->constant == constants[storage_count]);
        assert(storage->immutable && storage->form == STORAGE_OWNING_LIST);
        storage_count++;
    }
    assert(storage_count == 4);
    destroy_constants();
}

/** Zero-input owners keep their exact old key; copied/nonshared symbols are refused. */
static void it_preserves_concrete_constant_identity_and_refuses_foreign_members(void) {
    setup_constants();
    const SemanticConstantPlan *zero = NULL;
    for (const SemanticConstantPlan *constant = frontend.semantic.constant_first; constant; constant = constant->next) {
        if (strcmp(constant->symbol->owner->module->source->logical_path, "//integer") == 0
            && constant->symbol->role_ordinal == 0) { zero = constant; break; }
    }
    assert(zero);
    const char *parts[] = {"zero"};
    QualifiedIdentity expected = {IDENTITY_CONSTANT, "//integer", parts, 1, zero->symbol->name};
    const IdentityName *name;
    assert(identity_register(&scaffold.identities, expected, &name) == IDENTITY_OK);
    assert(strcmp(name->internal_name, codegen_constant_name(&scaffold, zero)) == 0);
    SourceArena temporary;
    source_arena_init(&temporary, sources.allocator);
    QualifiedIdentity output = expected;
    assert(semantic_constant_key(&frontend.semantic, zero->symbol, &temporary, &output) == IDENTITY_OK);
    assert(output.member_count == 1 && strcmp(output.members[0], "zero") == 0);
    assert(strcmp(output.module_path, "//integer") == 0);
    output = expected;
    SemanticSymbol copied = *zero->symbol;
    assert(semantic_constant_key(&frontend.semantic, &copied, &temporary, &output) == IDENTITY_INVALID);
    assert(output.members == parts && output.module_path == expected.module_path);
    const SemanticSymbol *state = frontend.plan.entry->type->first;
    assert(state && state->role != SYMBOL_CONSTANT);
    assert(semantic_constant_key(&frontend.semantic, state, &temporary, &output) == IDENTITY_INVALID);
    assert(output.members == parts);
    assert(semantic_constant_key(NULL, zero->symbol, &temporary, &output) == IDENTITY_INVALID);
    assert(semantic_constant_key(&frontend.semantic, NULL, &temporary, &output) == IDENTITY_INVALID);
    assert(semantic_constant_key(&frontend.semantic, zero->symbol, NULL, &output) == IDENTITY_INVALID);
    assert(semantic_constant_key(&frontend.semantic, zero->symbol, &temporary, NULL) == IDENTITY_INVALID);
    bool relations_valid = frontend.semantic.relations_valid;
    frontend.semantic.relations_valid = false;
    assert(semantic_constant_key(&frontend.semantic, zero->symbol, &temporary, &output) == IDENTITY_INVALID);
    frontend.semantic.relations_valid = relations_valid;
    assert(output.members == parts && output.module_path == expected.module_path);
    source_arena_destroy(&temporary);
    destroy_constants();
}

/** Every temporary owner/member/encoding allocation can fail without publishing a generated name. */
static void it_unwinds_every_specialized_constant_naming_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_constants();
        const SemanticConstantPlan *target = frontend.semantic.constant_first;
        const ProjectModule *container = project_find(&frontend.project, "/test/fixture/v1.0.0/src/container");
        while (target && target->symbol->owner->module != container) { target = target->next; }
        assert(target);
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        const char *name = codegen_constant_name(&scaffold, target);
        if (!cut) { assert(name); total = allocation_count - before; assert(total); }
        else {
            assert(!name && scaffold.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
            Diagnostic original = scaffold.error;
            assert(!codegen_constant_name(&scaffold, target));
            assert(scaffold.error.message == original.message);
        }
        failure_at = 0;
        destroy_constants();
    }
    printf("specialized constant naming allocation refusal positions: %zu\n", total);
}

/** A finite nominal self edge with no values is harmless; a cycle of actual
 * shared-value reads cannot produce a startup order and must be rejected.
 */
static void it_distinguishes_empty_self_typed_constants_from_actual_value_dependency_cycles(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@=\nchildren[]\n/test/fixture/v1.0.0/src/main\n@!\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    assert(frontend.semantic.constants_valid && frontend.plan.shared_first);
    assert(frontend.plan.shared_first->immutable && frontend.plan.shared_first->form == STORAGE_OWNING_LIST);
    assert(frontend.plan.shared_first->constant->symbol->owner == frontend.plan.shared_first->constant->symbol->type);
    destroy_frontend();
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@=\nfirst\n//integer\n /test/fixture/v1.0.0/src/palette/value\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/palette", "@=\nvalue\n//integer\n /test/fixture/v1.0.0/src/main/first\n");
    setup_frontend();
    assert(!compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    assert(frontend.semantic.resolved && !frontend.semantic.relations_valid && !frontend.semantic.constants_valid);
    assert(diagnostic_valid(&frontend.error));
    destroy_frontend();
}

/** Run only independently asserted actual frontend/model handoffs. */
static void run(void) {
    it_distinguishes_specialized_constant_names_and_reuses_equal_owners();
    it_preserves_concrete_constant_identity_and_refuses_foreign_members();
    it_unwinds_every_specialized_constant_naming_allocation_refusal();
    it_distinguishes_empty_self_typed_constants_from_actual_value_dependency_cycles();
}

/** Finish successfully only after all assertions and counted teardown. */
int main(void) { run(); return 0; }
