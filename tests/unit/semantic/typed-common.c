/**
 * @file
 * @brief Create explicit native capability fixtures and check resolved source forms.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TYPED_TEST_COMMON
#define AMA_TYPED_TEST_COMMON

#include "common.c"

/**
 * Create the explicit native interface/body/ABI companion set used by this leaf. The caller
 * supplies its known contract; no native capability is inferred from a C name.
 */
static void native_fixture(const char *name, const char *bindings) {
    char path[160];
    const char *suffixes[] = {"", ".h", ".c", ".abi"};
    for (size_t index = 0; index < 4; index++) {
        int length = snprintf(path, sizeof(path), "generated/program/packages/test/fixture/v1.0.0/src/%s%s", name, suffixes[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, index == 0 ? "@C\n" : index == 3 ? bindings : "/* authored native input */\n");
    }
}

/**
 * Create explicit copyable/noncopyable native fixtures and Ama worker declarations to
 * distinguish nominal type and capability checks.
 */
static void prepare_types(void) {
    fixture_package_prepare("generated/program", "fixture");
    const char *all = "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] position\n";
    native_fixture("value", all);
    native_fixture("other", all);
    native_fixture("resource", "+ initialize\n- destroy\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\n");
}

/**
 * Write/load one exact Ama fixture, then run native and typed resolution only after the
 * preceding stage succeeds.
 */
static SemanticStatus resolve_text(const char *text) {
    file("generated/program/packages/test/fixture/v1.0.0/src/main", text);
    setup_semantic();
    SemanticStatus status = load_semantic("generated/program");
    assert(module_count() != 0);
    if (status == SEMANTIC_OK) {
        status = semantic_native(&semantic);
    }
    if (status == SEMANTIC_OK) {
        status = semantic_resolve(&semantic);
    }
    return status;
}

/**
 * Require the exact expected semantic success partition and valid diagnostics on rejection;
 * always destroy semantic/source owners afterward.
 */
static inline void expect_resolved(const char *text, bool valid) {
    SemanticStatus status = resolve_text(text);
    if ((status == SEMANTIC_OK) != valid) {
        fprintf(stderr, "Unexpected type check: %s\n%s\n", semantic.error.message, text);
    }
    assert((status == SEMANTIC_OK) == valid);
    if (!valid) {
        assert(diagnostic_valid(&semantic.error));
    }
    destroy_semantic();
}

#endif
