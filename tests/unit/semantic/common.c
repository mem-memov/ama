#ifndef AMA_SEMANTIC_TEST_COMMON
#define AMA_SEMANTIC_TEST_COMMON

#include "../project/common.c"
#include "semantic/semantic.h"

static SemanticProgram semantic;

static void setup_semantic(void) {
    setup_project();
}

static SemanticStatus load_semantic(const char *path) {
    ProjectStatus loaded = load_root(path);
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    return semantic_names(&semantic);
}

static void destroy_semantic(void) {
    semantic_program_destroy(&semantic);
    destroy_project();
}

#endif
