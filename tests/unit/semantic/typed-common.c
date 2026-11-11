#ifndef AMA_TYPED_TEST_COMMON
#define AMA_TYPED_TEST_COMMON

#include "common.c"

static void native_fixture(const char *name, const char *bindings) {
    char path[160];
    const char *suffixes[] = {"", ".h", ".c", ".abi"};
    for (size_t index = 0; index < 4; index++) {
        int length = snprintf(path, sizeof(path), "generated/program/%s%s", name, suffixes[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, index == 0 ? "@C\n" : index == 3 ? bindings : "/* authored native input */\n");
    }
}

static void prepare_types(void) {
    directory("generated/program");
    const char *all = "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] position\n";
    native_fixture("value", all);
    native_fixture("other", all);
    native_fixture("resource", "+ initialize\n- destroy\n");
    file("generated/program/worker", "@!\n");
}

static SemanticStatus resolve_text(const char *text, SemanticLibraries libraries) {
    file("generated/program/main", text);
    setup_semantic();
    SemanticStatus status = load_semantic("generated/program");
    assert(module_count() != 0);
    if (status == SEMANTIC_OK) {
        status = semantic_native(&semantic);
    }
    if (status == SEMANTIC_OK) {
        status = semantic_resolve(&semantic, libraries);
    }
    return status;
}

static inline void expect_resolved(const char *text, bool valid) {
    SemanticStatus status = resolve_text(text, (SemanticLibraries){0});
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
