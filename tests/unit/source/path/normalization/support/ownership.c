/**
 * @file
 * @brief Check path failure publication, independent destination bytes and every measured allocation refusal.
 * Input and result arenas retire independently; source-origin owners outlive both.
 */
/** Syntax/scope/root refusal and each construction allocation preserve the caller's old output. */
static void it_preserves_outputs_for_syntax_scope_underflow_and_storage_refusals(void) {
    setup_paths();
    const ModulePathValue *base = canonical("//a");
    const char *invalid[] = {"", "a", "/", "//", "///a", "./", "../", "~", "~a//b", "//a/", ".static", "//a/\xff"};
    SourceIssue issue = {0};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        const ModulePathValue *value = base;
        size_t before;
        SourceSpan expression = text(invalid[index]);
        before = allocation_count;
        assert(module_path_normalize(&paths, expression, base, NULL, NULL, &value, &issue) == MODULE_PATH_INVALID);
        assert(value == base && allocation_count == before && source_span_valid(issue.span));
    }
    const ModulePathValue *value = base;
    assert(module_path_normalize(&paths, text("./a"), NULL, NULL, NULL, &value, &issue) == MODULE_PATH_MISSING_BASE);
    assert(value == base);
    assert(module_path_normalize(&paths, text("~missing/a"), base, NULL, NULL, &value, &issue) == MODULE_PATH_UNKNOWN_ALIAS);
    assert(value == base);
    assert(module_path_normalize(&paths, (SourceSpan){0}, base, NULL, NULL, &value, &issue) == MODULE_PATH_INVALID);
    assert(module_path_normalize(&paths, text("../../a"), base, NULL, NULL, &value, &issue) == MODULE_PATH_UNDERFLOW);
    assert(value == base);
    const ModulePathValue *bad = base;
    assert(module_path_from_canonical(&paths, (const unsigned char *)"//a/..", strlen("//a/.."), &bad, &issue) == MODULE_PATH_INVALID);
    assert(bad == base);
    ModulePathPart *stack = NULL;
    ModulePathFragment one = {(const unsigned char *)"x", 1, {0}};
    assert(path_normalization_stack(&paths, SIZE_MAX, one, &stack, &issue) == MODULE_PATH_LIMIT);
    assert(path_normalization_stack(&paths, SIZE_MAX / sizeof(ModulePathPart) + 1,
        (ModulePathFragment){0}, &stack, &issue) == MODULE_PATH_LIMIT);
    const ModulePathValue *stored = base;
    assert(path_normalization_store(&paths, (ModuleRootKind)0, NULL, 0, (SourceSpan){0},
        &stored, &issue) == MODULE_PATH_INVALID && stored == base);
    assert(path_normalization_store(&paths, MODULE_ROOT_PROJECT, NULL, SIZE_MAX, (SourceSpan){0},
        &stored, &issue) == MODULE_PATH_LIMIT && stored == base);
    const unsigned char byte = 'a';
    ModulePathPart excessive[] = {{&byte, SIZE_MAX, {0}}, {&byte, 1, {0}}};
    assert(path_normalization_store(&paths, MODULE_ROOT_PROJECT, excessive, 1, (SourceSpan){0},
        &stored, &issue) == MODULE_PATH_LIMIT && stored == base);
    excessive[0].length = SIZE_MAX - 1;
    assert(path_normalization_store(&paths, MODULE_ROOT_PROJECT, excessive, 2, (SourceSpan){0},
        &stored, &issue) == MODULE_PATH_LIMIT && stored == base);
    finish_paths();
    for (size_t operation = 0; operation < 4; operation++) {
        setup_paths();
        base = canonical("//palette/red");
        SourceSpan expression = text("../item");
        size_t before = allocation_count;
        ModuleConstantPath selected;
        const ModulePathValue *result = NULL;
        ModulePathStatus status = operation == 0 ? module_path_normalize(&paths, expression, base, NULL, NULL, &result, &issue)
            : operation == 1 ? module_path_from_canonical(&paths, (const unsigned char *)"//item", 6, &result, &issue)
            : operation == 2 ? module_path_parent(&paths, base, &result, &issue)
            : module_path_constant(&paths, base, &selected, &issue);
        assert(status == MODULE_PATH_OK);
        size_t count = allocation_count - before;
        finish_paths();
        for (size_t failure = 1; failure <= count; failure++) {
            setup_paths();
            base = canonical("//palette/red");
            expression = text("../item");
            failure_at = allocation_count + failure;
            result = base;
            selected = (ModuleConstantPath){.module = base};
            unsigned char saved[sizeof(selected)];
            memcpy(saved, &selected, sizeof(selected));
            status = operation == 0 ? module_path_normalize(&paths, expression, base, NULL, NULL, &result, &issue)
                : operation == 1 ? module_path_from_canonical(&paths, (const unsigned char *)"//item", 6, &result, &issue)
                : operation == 2 ? module_path_parent(&paths, base, &result, &issue)
                : module_path_constant(&paths, base, &selected, &issue);
            assert(status == MODULE_PATH_OUT_OF_MEMORY);
            assert(operation == 3 ? memcmp(saved, &selected, sizeof(selected)) == 0 : result == base);
            finish_paths();
        }
    }
}

/** Metadata, callback and selected-member input arenas may retire without invalidating destination bytes. */
static void it_owns_output_bytes_independently_of_metadata_alias_and_selection_inputs(void) {
    setup_paths();
    char metadata[] = "//palette/red";
    const ModulePathValue *owned = canonical(metadata);
    memset(metadata, 'x', sizeof(metadata) - 1);
    assert(strcmp(owned->text, "//palette/red") == 0);
    SourceArena provider;
    source_arena_init(&provider, sources.allocator);
    const ModulePathValue *target = NULL;
    SourceIssue issue = {0};
    SourceSpan declaration = text("//名字/palette/red");
    assert(module_path_normalize(&provider, declaration, NULL, NULL, NULL, &target, &issue) == MODULE_PATH_OK);
    AliasTarget alias = {"~value", target, 0};
    SourceSpan use = text("~value");
    const ModulePathValue *value = NULL;
    assert(module_path_normalize(&paths, use, NULL, lookup, &alias, &value, &issue) == MODULE_PATH_OK);
    assert(alias.calls == 1);
    ModuleConstantPath selected;
    assert(module_path_constant(&paths, target, &selected, &issue) == MODULE_PATH_OK);
    source_arena_destroy(&provider);
    assert(strcmp(value->text, "//名字/palette/red") == 0);
    assert(strcmp(selected.module->text, "//名字/palette") == 0);
    assert(selected.member.length == 3 && memcmp(selected.member.bytes, "red", 3) == 0);
    assert(selected.member.origin.source == declaration.source);
    for (size_t index = 0; index < value->count; index++) {
        assert(value->parts[index].bytes >= (const unsigned char *)value->text);
        assert(value->parts[index].bytes + value->parts[index].length <= (const unsigned char *)value->text + value->length);
    }
    finish_paths();
}
