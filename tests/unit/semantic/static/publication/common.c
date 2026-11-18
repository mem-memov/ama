/** @file
 * @brief Own immutable publications before their borrowed guarded result fixtures.
 */
#ifndef AMA_STATIC_PUBLICATION_TEST_COMMON
#define AMA_STATIC_PUBLICATION_TEST_COMMON
#include "../results/common.c"
#include "semantic/static/publication/internal.h"

/** Publication descriptors expire before their source and solver dependencies. */
static StaticPublicationPlan publications;

/** Initialize the publication owner without semantic work or allocation. */
static void setup_publications(const char *caller, const char *target) {
    setup_guards(caller, target);
    static_result_init(&results, &guards, sources.allocator);
    size_t before = allocation_count;
    static_publication_init(&publications, &results, sources.allocator);
    assert(allocation_count == before && publications.status == STATIC_PUBLICATION_OK);
}

/** Only a registered checked root supplies a compilation context. */
static const StaticResultContext *publication_context(const StaticTermEnvironment *environment) {
    const StaticCreationPath *root;
    const StaticResultContext *context;
    assert(static_creation_root(&guards, environment, &root));
    assert(static_result_context(&results, root, &context));
    return context;
}

/** Teardown leaves the inherited counted fixture at zero live allocations. */
static void destroy_publications(void) {
    static_publication_destroy(&publications);
    assert(publications.results == NULL && publications.first == NULL && publications.first_alias == NULL);
    destroy_results();
}
#endif
