/**
 * @file
 * @brief Check normalized roots, whole alias lookup and exact surviving source provenance.
 * Fixtures supply explicit logical bases; no filesystem provider is available.
 */
/** Test-only read-only target with a call counter observing the lookup boundary. */
typedef struct { const char *key; const ModulePathValue *target; size_t calls; } AliasTarget;
/** Match the entire original alias key without prefix fallback or recursive interpretation. */
static const ModulePathValue *lookup(void *context, SourceSpan key) {
    AliasTarget *alias = context;
    alias->calls++;
    return key.end - key.start == strlen(alias->key)
        && memcmp(source_bytes(key.source) + key.start, alias->key, strlen(alias->key)) == 0 ? alias->target : NULL;
}

/** Relative roots remain fixed and the initial ../ removes an actual defining-directory component. */
static void it_uses_explicit_directories_roots_and_initial_parent_components(void) {
    setup_paths();
    const ModulePathValue *module = canonical("/acme/pkg/v1.2.3/src/object/worker");
    const ModulePathValue *base = NULL;
    SourceIssue issue = {0};
    assert(module_path_parent(&paths, module, &base, &issue) == MODULE_PATH_OK);
    assert(strcmp(base->text, "/acme/pkg/v1.2.3/src/object") == 0);
    assert(strcmp(normalized("./sibling", base)->text, "/acme/pkg/v1.2.3/src/object/sibling") == 0);
    assert(strcmp(normalized("../other", base)->text, "/acme/pkg/v1.2.3/src/other") == 0);
    const ModulePathValue *standard = canonical("//item");
    assert(module_path_parent(&paths, standard, &base, &issue) == MODULE_PATH_OK);
    assert(strcmp(base->text, "//") == 0 && base->count == 0 && base->root == MODULE_ROOT_STANDARD);
    const ModulePathValue *next = normalized("./next", base);
    assert(strcmp(next->text, "//next") == 0 && next->root == MODULE_ROOT_STANDARD);
    next = normalized("/acme/pkg/v1.2.3/src/item", base);
    assert(next->root == MODULE_ROOT_PROJECT);
    next = normalized("/missing/../acme/pkg/v1.2.3/src/./item", NULL);
    assert(strcmp(next->text, "/acme/pkg/v1.2.3/src/item") == 0);
    const ModulePathValue *twice = normalized(next->text, NULL);
    assert(strcmp(next->text, twice->text) == 0 && next->root == twice->root && next->count == twice->count);
    const ModulePathValue *root = normalized("/a/..", NULL);
    assert(strcmp(root->text, "/") == 0 && root->count == 0 && root->parts == NULL);
    for (size_t index = 0; index < module->count; index++) { assert(module->parts[index].origin.source == NULL); }
    finish_paths();
}

/** A temporarily non-module prefix may be repaired by a later parent suffix without any lookup. */
static void it_defers_endpoint_validation_until_after_complete_reduction(void) {
    setup_paths();
    const ModulePathValue *prefix = normalized("/acme/pkg/v1.2.3/not source", NULL);
    assert(!module_path_canonical((const unsigned char *)prefix->text, prefix->length));
    AliasTarget alias = {"~base", prefix, 0};
    const ModulePathValue *value = NULL;
    SourceIssue issue = {0};
    assert(module_path_normalize(&paths, text("~base/../src/item"), NULL, lookup, &alias, &value, &issue) == MODULE_PATH_OK);
    assert(alias.calls == 1 && strcmp(value->text, "/acme/pkg/v1.2.3/src/item") == 0);
    assert(module_path_canonical((const unsigned char *)value->text, value->length));
    finish_paths();
}

/** Appended source components and retained alias-target components keep their actual, distinct owners. */
static void it_performs_one_whole_alias_lookup_and_preserves_surviving_origins(void) {
    setup_paths();
    const ModulePathValue *target = normalized("//名字/worker", NULL);
    AliasTarget alias = {"~two words", target, 0};
    SourceSpan use = text("~two words/../item state");
    const ModulePathValue *value = NULL;
    SourceIssue issue = {0};
    assert(module_path_normalize(&paths, use, NULL, lookup, &alias, &value, &issue) == MODULE_PATH_OK);
    assert(alias.calls == 1 && value->root == MODULE_ROOT_STANDARD && value->count == 2);
    assert(strcmp(value->text, "//名字/item state") == 0);
    assert(value->parts[0].origin.source == target->expression.source);
    assert(value->parts[0].origin.start == strlen("//") && value->parts[0].origin.end == strlen("//名字"));
    assert(value->parts[1].origin.source == use.source);
    assert(value->parts[1].origin.start == strlen("~two words/../") && value->parts[1].origin.end == use.end);
    assert(value->expression.source == use.source && value->expression.start == use.start && value->expression.end == use.end);
    alias.key = "~two";
    const ModulePathValue *unchanged = value;
    assert(module_path_normalize(&paths, use, NULL, lookup, &alias, &unchanged, &issue) == MODULE_PATH_UNKNOWN_ALIAS);
    assert(unchanged == value && alias.calls == 2);
    assert(issue.span.source == use.source && issue.span.end == strlen("~two words"));
    finish_paths();
}

/** Constant selection uses the final reduced component, including a complete path supplied only by an alias. */
static void it_selects_constants_after_normalization_and_rejects_incomplete_or_dotted_members(void) {
    setup_paths();
    const ModulePathValue *target = normalized("//palette/unused/../red value", NULL);
    AliasTarget alias = {"~constant", target, 0};
    const ModulePathValue *value = NULL;
    SourceIssue issue = {0};
    SourceSpan use = text("~constant");
    assert(module_path_normalize(&paths, use, NULL, lookup, &alias, &value, &issue) == MODULE_PATH_OK);
    ModuleConstantPath selected;
    assert(module_path_constant(&paths, value, &selected, &issue) == MODULE_PATH_OK);
    assert(strcmp(selected.module->text, "//palette") == 0 && selected.member.length == strlen("red value"));
    assert(memcmp(selected.member.bytes, "red value", selected.member.length) == 0);
    assert(selected.member.origin.source == target->expression.source && selected.expression.source == use.source);
    const char *invalid[] = {"//only", "/a/pkg/v1.0.0/src/red", "//palette/v1.2.3"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        value = normalized(invalid[index], NULL);
        unsigned char saved[sizeof(selected)];
        memcpy(saved, &selected, sizeof(selected));
        assert(module_path_constant(&paths, value, &selected, &issue) == MODULE_PATH_INVALID);
        assert(memcmp(saved, &selected, sizeof(selected)) == 0);
    }
    const ModulePathValue *namespace = canonical("/a/pkg/v1.0.0");
    const ModulePathValue *parent = value;
    assert(module_path_parent(&paths, namespace, &parent, &issue) == MODULE_PATH_INVALID && parent == value);
    assert(module_path_parent(&paths, NULL, &parent, &issue) == MODULE_PATH_INVALID);
    assert(module_path_constant(&paths, NULL, &selected, &issue) == MODULE_PATH_INVALID);
    finish_paths();
}
