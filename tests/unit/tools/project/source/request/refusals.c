/** @file
 * @brief Check malformed command arities, exact flag conflicts and raw root/exclusion refusals.
 * @see [T277 entry review](../../../../../../docs/implementation/source-operations.md#t277-entry-review-after-accepted-t276)
 */
/** Fixed positional/option grammar has no truncation, last-value or default-mode
 * escape. Each refusal owns/destructs genuine allocated scratch. */
static void it_refuses_incomplete_unknown_repeated_and_conflicting_source_arguments(void) {
    setup_test(); const char *base[] = {"ama-project", "copy", "source", "destination", "--preview", "--root", "root"};
    for (int count = 0; count < 7; count++) {
        ProjectSourceRequestStatus expected = count < 4 ? PROJECT_SOURCE_REQUEST_ARGUMENTS
            : count == 4 ? PROJECT_SOURCE_REQUEST_MODE : count == 5 ? PROJECT_SOURCE_REQUEST_ROOTS : PROJECT_SOURCE_REQUEST_MISSING_VALUE;
        expect_source_request(count, base, expected);
    }
    expect_source_request(-1, base, PROJECT_SOURCE_REQUEST_ARGUMENTS);
    expect_source_request(7, NULL, PROJECT_SOURCE_REQUEST_ARGUMENTS);
    const char *arguments[16]; memcpy(arguments, base, sizeof(base));
    for (size_t index = 0; index < 7; index++) {
        const char *saved = arguments[index]; arguments[index] = NULL;
        expect_source_request(7, arguments, PROJECT_SOURCE_REQUEST_ARGUMENTS); arguments[index] = saved;
    }
    arguments[1] = "init"; expect_source_request(7, arguments, PROJECT_SOURCE_REQUEST_COMMAND); arguments[1] = "copy";
    arguments[2] = ""; expect_source_request(7, arguments, PROJECT_SOURCE_REQUEST_EMPTY_VALUE); arguments[2] = "source";
    arguments[3] = ""; expect_source_request(7, arguments, PROJECT_SOURCE_REQUEST_EMPTY_VALUE); arguments[3] = "destination";
    arguments[0] = ""; expect_source_request(7, arguments, PROJECT_SOURCE_REQUEST_ARGUMENTS); arguments[0] = base[0];
    const char *unknown[] = {"--unknown", "--preview=true", "extra operand", "--copy", "--root=root"};
    for (size_t i = 0; i < sizeof(unknown)/sizeof(*unknown); i++) {
        arguments[7] = unknown[i]; expect_source_request(8, arguments, PROJECT_SOURCE_REQUEST_OPTION);
    }
    const char *valued[] = {"--root", "--standard-root", "--destination-root", "--destination-standard-root", "--compiler", "--exclude", "--destination-exclude"};
    for (size_t i = 0; i < sizeof(valued)/sizeof(*valued); i++) {
        arguments[7] = valued[i]; expect_source_request(8, arguments, PROJECT_SOURCE_REQUEST_MISSING_VALUE);
        arguments[8] = ""; arguments[9] = "path";
        expect_source_request(i < 5 ? 9 : 10, arguments, PROJECT_SOURCE_REQUEST_EMPTY_VALUE);
        if (i >= 5) { arguments[8] = "root"; expect_source_request(9, arguments, PROJECT_SOURCE_REQUEST_MISSING_VALUE); }
    }
    const char *pairs[][2] = {{"--preview", "--preview"}, {"--apply", "--apply"},
        {"--preview", "--apply"}, {"--apply", "--preview"},
        {"--copy-dependencies", "--copy-dependencies"}, {"--extend-package", "--extend-package"},
        {"--copy-dependencies", "--extend-package"}, {"--extend-package", "--copy-dependencies"},
        {"--replace", "--replace"}};
    for (size_t i = 0; i < sizeof(pairs)/sizeof(*pairs); i++) {
        memcpy(arguments, base, sizeof(base));
        if (i < 4) { arguments[4] = pairs[i][0]; arguments[7] = pairs[i][1]; }
        else { arguments[7] = pairs[i][0]; arguments[8] = pairs[i][1]; }
        ProjectSourceRequestStatus expected = i == 2 || i == 3 ? PROJECT_SOURCE_REQUEST_MODE
            : i == 6 || i == 7 ? PROJECT_SOURCE_REQUEST_POLICY : PROJECT_SOURCE_REQUEST_REPEATED;
        expect_source_request(i < 4 ? 8 : 9, arguments, expected);
    }
    memcpy(arguments, base, sizeof(base)); arguments[7] = "--compiler"; arguments[8] = "one";
    arguments[9] = "--compiler"; arguments[10] = "two"; expect_source_request(11, arguments, PROJECT_SOURCE_REQUEST_REPEATED);
    const char *incompatible[] = {"--destination-root", "--destination-standard-root", "--destination-exclude"};
    for (size_t i = 0; i < 3; i++) {
        const char *deleted[] = {"ama-project", "delete", "source", "--preview", "--root", "root", incompatible[i], "destination", "path"};
        expect_source_request(i == 2 ? 9 : 8, deleted, PROJECT_SOURCE_REQUEST_SCOPE);
    }
    destroy_test();
}
/** Exact physical relative keys never navigate, collapse separators or bind to
 * another scope by prefix. Same raw path in O/F is allowed with the same family. */
static void it_refuses_ambiguous_root_scope_and_noncanonical_exclusion_keys(void) {
    setup_test();
    const char *base[] = {"ama-project", "copy", "source", "destination", "--preview", "--root", "root"};
    const char *arguments[14]; memcpy(arguments, base, sizeof(base));
    const char *duplicates[] = {"--root", "--standard-root", "--destination-standard-root"};
    for (size_t i = 0; i < 3; i++) {
        arguments[7] = duplicates[i]; arguments[8] = "root"; expect_source_request(9, arguments, PROJECT_SOURCE_REQUEST_ROOTS);
    }
    arguments[7] = "--destination-root"; arguments[8] = "new"; arguments[9] = "--destination-root"; arguments[10] = "new";
    expect_source_request(11, arguments, PROJECT_SOURCE_REQUEST_ROOTS);
    arguments[9] = "--destination-standard-root"; expect_source_request(11, arguments, PROJECT_SOURCE_REQUEST_ROOTS);
    arguments[7] = "--exclude"; arguments[8] = "root";
    const char *invalid[] = {".", "..", "/absolute", "a/", "a//b", "./a", "a/../b", "a/./b", "/", "../a"};
    for (size_t i = 0; i < sizeof(invalid)/sizeof(*invalid); i++) {
        arguments[9] = invalid[i]; expect_source_request(10, arguments, PROJECT_SOURCE_REQUEST_EXCLUSION);
    }
    arguments[9] = "valid/path"; arguments[8] = "roo";
    ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
    assert(project_source_request_parse(&request, 10, arguments) == PROJECT_SOURCE_REQUEST_EXCLUSION && request.failed_argument == 8);
    project_source_request_destroy(&request);
    arguments[8] = "root"; arguments[10] = "--exclude"; arguments[11] = "root"; arguments[12] = "valid/path";
    expect_source_request(13, arguments, PROJECT_SOURCE_REQUEST_REPEATED);
    arguments[7] = "--destination-exclude"; expect_source_request(10, arguments, PROJECT_SOURCE_REQUEST_EXCLUSION);
    arguments[7] = "--exclude";
    const char *valid[] = {"a..b/file.ini", "spaces and symbols/$value", "opaque\xff"};
    for (size_t i = 0; i < sizeof(valid)/sizeof(*valid); i++) {
        arguments[9] = valid[i]; expect_source_request(10, arguments, PROJECT_SOURCE_REQUEST_OK);
    }
    const char *standard[] = {"ama-project", "delete", "physical source", "--preview", "--standard-root", "arbitrary name"};
    expect_source_request(6, standard, PROJECT_SOURCE_REQUEST_OK);
    const char *reverse[] = {"ama-project", "move", "source", "destination", "--apply",
        "--destination-standard-root", "same", "--root", "same"};
    expect_source_request(9, reverse, PROJECT_SOURCE_REQUEST_ROOTS);
    destroy_test();
}
