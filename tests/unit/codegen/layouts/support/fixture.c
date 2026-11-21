/** @file
 * @brief Own real four-form source fields and three qualified over-aligned native interfaces before generating the assembly.
 */
static NativeScaffold assembly;
/** Declare one native module with an identical actual State representation and explicit four-slot lifecycle ABI. */
static void layout_native(const char *name, const char *interface) {
    char path[160];
    int length = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    file(path, interface);
    length = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.h", name);
    assert(length > 0);
    file(path, "struct AMA_NATIVE(State) { _Alignas(64) unsigned char marker[17]; uintptr_t address; void *resource; size_t number; };\n");
    length = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.c", name);
    assert(length > 0);
    file(path, "/* Typed native bodies are written after their declaration identities are known. */\n");
    length = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.abi", name);
    assert(length > 0);
    file(path, "+ initialize\n- destroy\n| literal\n= clone\n");
}
/** Write all four storage forms, a same-representation distinct native module, a constant-only namespace and an actual typed native call. */
static void prepare_layout(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    layout_native("plain", "@C\n");
    layout_native("twin", "@C\n");
    layout_native("worker", "@<\ninput\n/test/fixture/v1.0.0/src/plain\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/palette", "@=\nvalue\n/test/fixture/v1.0.0/src/plain\n |C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@=\nshared\n/test/fixture/v1.0.0/src/plain\n /test/fixture/v1.0.0/src/palette/value\n@!\n"
        "first\n/test/fixture/v1.0.0/src/plain\n |A\nsecond\n/test/fixture/v1.0.0/src/plain\n first\nother\n/test/fixture/v1.0.0/src/twin\n |B\n"
        "items[]\n/test/fixture/v1.0.0/src/plain\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n |D\n"
        "reference&\n/test/fixture/v1.0.0/src/plain\n & first\nreferences&[]\n/test/fixture/v1.0.0/src/plain\n & first\n & second\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n < first\n");
}
/** Load complete static semantics/plans and initialize artifact ownership before any generated-byte publication. */
static void begin_layout(bool valid) {
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    bool checked = compiler_frontend_check(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main");
    assert(checked == valid);
    if (valid) {
        assert(frontend.plan.valid && frontend.plan.context_storage.pools == 4 && frontend.plan.context_storage.constants == 2);
        native_scaffold_init(&assembly, &frontend, sources.allocator);
    }
}
/** Select a known authored module by exact complete logical path, never by basename/hash guesses. */
static const ScaffoldType *layout_type(const char *logical) {
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, logical) == 0) { return type; }
    }
    assert(false);
    return NULL;
}
/** Resolve an expected authored field from original source bytes for test oracle construction. */
static const ScaffoldMember *layout_member(const ScaffoldType *type, const char *name) {
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        SourceSpan span = member->field->symbol->name;
        if (span.end - span.start == strlen(name) && memcmp(source_bytes(span.source) + span.start, name, strlen(name)) == 0) { return member; }
    }
    assert(false);
    return NULL;
}
