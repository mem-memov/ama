/**
 * @file
 * @brief Layer semantic ownership over counted loaded project fixtures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_SEMANTIC_TEST_COMMON
#define AMA_SEMANTIC_TEST_COMMON

#include "../project/common.c"
#include "semantic/semantic.h"

static SemanticProgram semantic;

/**
 * Initialize the counted project fixture on which this semantic owner will depend.
 */
static void setup_semantic(void) {
    setup_project();
}

/**
 * Load the exact fixture project, initialize semantic ownership and run the production namespace
 * pass.
 */
static SemanticStatus load_semantic(const char *path) {
    ProjectStatus loaded = load_root(path);
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    return semantic_names(&semantic);
}

/**
 * Destroy semantic graphs before project/source owners, preserving borrow validity during all
 * destruction.
 */
static void destroy_semantic(void) {
    semantic_program_destroy(&semantic);
    destroy_project();
}

#endif
