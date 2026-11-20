/** @file
 * @brief Own checked current-source project/standard root closures and independent scaffold artifact lifetimes.
 */
static NativeScaffold scaffold;
static char contract_directory[128];
static size_t contract_generation;
static const char *minimal_abi = "+ initialize\n- destroy\n";
static const char *project_peer = "/test/fixture/v1.0.0/src/магазин/товар данные";
static const char *standard_peer = "//магазин/товар данные";
static const char *original_body = "/* Authored companion: publication has no permission to replace it. */\n";
/** Write explicit interface/header/body/ABI inputs; generated output remains separate from all authored paths. */
static void contract_native(const char *path, const char *interface, const char *header) {
    file(path, interface);
    char companion[512];
    int length = snprintf(companion, sizeof(companion), "%s.h", path);
    assert(length > 0 && (size_t)length < sizeof(companion));
    file(companion, header);
    length = snprintf(companion, sizeof(companion), "%s.c", path);
    assert(length > 0 && (size_t)length < sizeof(companion));
    file(companion, original_body);
    length = snprintf(companion, sizeof(companion), "%s.abi", path);
    assert(length > 0 && (size_t)length < sizeof(companion));
    file(companion, minimal_abi);
}
/** Build a strict or explicitly proposed-ABI frontend with tagged roots; all module keys come from actual current source loading. */
static void begin_contract(const char *entry, bool authoring) {
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    bool checked = authoring ? compiler_frontend_authoring(&frontend, roots, 2, entry, minimal_abi)
        : compiler_frontend_check(&frontend, roots, 2, entry);
    assert(checked && frontend.valid && frontend.plan.valid);
    native_scaffold_init(&scaffold, &frontend, sources.allocator);
}
/** Build one complete in-memory declaration model; only explicitly requested publication writes artifacts. */
static bool build_contract(bool create, bool overwrite) {
    int length = snprintf(contract_directory, sizeof(contract_directory), "generated/native-contract-%zu", ++contract_generation);
    assert(length > 0 && (size_t)length < sizeof(contract_directory));
    return native_scaffold_build(&scaffold, (ScaffoldOptions){.directory = contract_directory, .sdk = TEST_SDK,
        .compiler = "gcc", .create = create, .overwrite = overwrite});
}
/** Find the exact checked source module in the scaffold, never substitute basename or frozen prefix bytes. */
static const ScaffoldType *contract_type(const char *logical) {
    for (const ScaffoldType *type = scaffold.types; type != NULL; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, logical) == 0) { return type; }
    }
    assert(false);
    return NULL;
}
/** Release provider rows before their borrowed identity/source owners and verify every allocation is reclaimed. */
static void end_contract(void) {
    native_scaffold_destroy(&scaffold);
    destroy_frontend();
}
/** Observe an authored input byte-for-byte after publication; compiler output must not silently replace it. */
static void assert_authored_bytes(const char *path, const char *expected) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    char bytes[2048];
    size_t count = fread(bytes, 1, sizeof(bytes), input);
    assert(!ferror(input) && count == strlen(expected) && memcmp(bytes, expected, count) == 0);
    int closed = fclose(input);
    assert(closed == 0);
}
