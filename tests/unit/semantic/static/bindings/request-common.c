/** @file
 * @brief Prepare actual producer metadata independently of reversed caller signatures for request tests.
 */
#ifndef AMA_BINDING_REQUEST_TEST_COMMON
#define AMA_BINDING_REQUEST_TEST_COMMON
#include "common.c"
#include "semantic/static/bindings/normalize.h"

/** Caller signatures deliberately reverse foreign slot declaration order;
 * actual producer order remains a,b and is selected as an independent fact.
 */
static const char *request_prefix = "@<\n...type\n...expected\n ...b\n ...a\n"
    "input&\n...type\nsource[]\n...type\n@>\n...return type\n...type\n@!\n";

/** Static and runtime producer interfaces interleave without sharing counters. */
static const char *request_producer = "@<\n...a\nfirst&\n...a\n...b\nsecond[]\n...b\n"
    "@>\n...out\n...a\nx&\n...a\n...second\n...b\ny[]\n...b\n@!\n";

/** Allocate temporary fixture text; its caller releases it after source capture. */
static char *request_text(const char *body) {
    size_t size = strlen(request_prefix) + strlen(body) + 1;
    char *text = malloc(size);
    assert(text != NULL);
    memcpy(text, request_prefix, strlen(request_prefix));
    memcpy(text + strlen(request_prefix), body, strlen(body) + 1);
    return text;
}

/** Materialize a real generic caller and selected producer in the narrow domain. */
static void setup_requests(const char *body) {
    char *text = request_text(body);
    setup_binding(text, request_producer);
    free(text);
}

/** Preserve real standard/import dependencies for syntax requiring those roots. */
static inline void setup_request_roots(const char *body, const ModuleRoot *roots, size_t root_count) {
    char *text = request_text(body);
    setup_binding_roots(text, request_producer, roots, root_count);
    free(text);
}

/** Find an actual body expression occurrence, not a foreign signature member. */
static inline const StaticOccurrence *request_head(const char *text) {
    const StaticModule *module = binding_module(project.entry);
    for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (site->receiver == NULL && site->root && layout_text_is(site->site, text)
            && (site->context == STATIC_CONTEXT_BODY || site->context == STATIC_CONTEXT_NAMESPACE)) { return site; }
    }
    assert(false);
    return NULL;
}

/** Build the independently selected producer's constructor snapshot. */
static inline BindingInterface *request_interface(void) {
    BindingInterface *view = NULL;
    assert(binding_source_interface(&binding_plan, binding_module(project_find(&project, ".other")),
        BINDING_MODULE_SURFACE, NULL, 0, &view));
    return view;
}
#endif
