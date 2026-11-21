/**
 * @file
 * @brief Partition exact INI alias, mask and repository syntax without filesystem evidence.
 * The parent leaf checks scalar/name spelling separately from scope and matching.
 */
/** Complete tilde names retain multiword/Unicode identity; only local targets may start with a package alias. */
static void it_checks_alias_keys_and_profile_specific_raw_targets(void) {
    const char *valid[] = {"~a", "~two words", "~_a-b", "~名字"};
    const char *invalid[] = {"", "~", "a", "~ a", "~a ", "~a  b", "~a/b", "~a.b", "~a--b", "~2a"};
    setup_test();
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        assert(configuration_alias_key(configuration_test_text(valid[index])));
    }
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        assert(!configuration_alias_key(configuration_test_text(invalid[index])));
    }
    const char *direct[] = {"/acme/pkg/v1.2.3/src", "//item", "./src", "../../other", "/a/../b"};
    for (size_t index = 0; index < sizeof(direct) / sizeof(*direct); index++) {
        SourceSpan text = configuration_test_text(direct[index]);
        assert(configuration_alias_target(text, false) && configuration_alias_target(text, true));
    }
    SourceSpan alias = configuration_test_text("~shared/items");
    assert(configuration_alias_target(alias, true) && !configuration_alias_target(alias, false));
    const char *bad[] = {"", "/", "///item", "/a/", "~", ".static", "...static", "a/b", "//a  b"};
    for (size_t index = 0; index < sizeof(bad) / sizeof(*bad); index++) {
        assert(!configuration_alias_target(configuration_test_text(bad[index]), true));
    }
    assert(!configuration_alias_key((SourceSpan){0}) && !configuration_alias_target((SourceSpan){0}, true));
    destroy_test();
}

/** Pattern syntax does not perform name lookup, select a latest version or normalize dot/parent components. */
static void it_checks_selector_components_without_resolving_or_cancelling_masks(void) {
    const char *valid[] = {"/acme/*/v2.4.14-*/src/mod1", "~shared", "~shared/*/src/item",
        "/名字/包/v1.0.0/src/多 字", "/acme/pkg/v*/tests/*", "/acme/pkg/v1.0.0/src/a*b*c"};
    const char *invalid[] = {"", "/", "//item", "./item", "../item", "/a/", "/a//b", "/a/./b",
        "/a/../b", "~a/../b", "~a/*/./b", ".type", "~", "~ a", "~a.b", "/a/ b", "/a /b",
        "/a/#b", "/a/[b]", "/a/\xc2\xa0x", "/a/\xff", "/a/\xc0\xaf"};
    setup_test();
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        assert(configuration_selector(configuration_test_text(valid[index])));
    }
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        assert(!configuration_selector(configuration_test_text(invalid[index])));
    }
    assert(!configuration_selector((SourceSpan){0}));
    destroy_test();
}

/** Namespace prefixes use Ama names; physical directory dot reduction checks containment without importing those name rules. */
static void it_separates_logical_repository_prefixes_from_physical_directory_containment(void) {
    const char *prefixes[] = {"/acme", "/acme/shop tools", "/名字/工具-集"};
    const char *bad_prefixes[] = {"", "/", "//acme", "acme", "/acme/", "/acme//pkg", "/acme/v1.2.3",
        "/acme/..", "/acme/*", "/acme/a--b", "/acme/a.b"};
    const char *directories[] = {".", "packages/acme", "a/../b", "a//b/", "./a/./b", "legacy (C)/code"};
    const char *bad_directories[] = {"", "/a", "../a", "..", "a/../../b", "a/../.."};
    setup_test();
    for (size_t index = 0; index < sizeof(prefixes) / sizeof(*prefixes); index++) {
        assert(configuration_repository_prefix(configuration_test_text(prefixes[index])));
    }
    for (size_t index = 0; index < sizeof(bad_prefixes) / sizeof(*bad_prefixes); index++) {
        assert(!configuration_repository_prefix(configuration_test_text(bad_prefixes[index])));
    }
    for (size_t index = 0; index < sizeof(directories) / sizeof(*directories); index++) {
        assert(configuration_repository_directory(configuration_test_text(directories[index])));
    }
    for (size_t index = 0; index < sizeof(bad_directories) / sizeof(*bad_directories); index++) {
        assert(!configuration_repository_directory(configuration_test_text(bad_directories[index])));
    }
    static const unsigned char nul[] = {'a', 0, 'b'};
    assert(!configuration_repository_directory(configuration_test_span(nul, sizeof(nul))));
    assert(!configuration_repository_prefix((SourceSpan){0}));
    assert(!configuration_repository_directory((SourceSpan){0}));
    SourceSpan a = configuration_test_text("same");
    SourceSpan b = configuration_test_text("same");
    assert(configuration_text_equal(a, b));
    assert(!configuration_text_equal(a, configuration_test_text("same longer")));
    assert(!configuration_text_equal(a, (SourceSpan){0}));
    assert(!configuration_text_is((SourceSpan){0}, "same"));
    assert(configuration_text_is(configuration_test_text(""), ""));
    destroy_test();
}
