/** @file
 * @brief Prepare named graph request fixtures with an independently selected actual constructor.
 */
#ifndef AMA_BINDING_REQUEST_TEST_COMMON
#define AMA_BINDING_REQUEST_TEST_COMMON
#include "common.c"
#include "semantic/static/bindings/normalize.h"

/** Ordinary formals are Type slots introduced through runtime declarations. */
static const char *request_producer = "@<\nfirst&\n...a\nsecond[]\n...b\n"
    "@>\nx&\n...a\ny[]\n...b\n@!\n";

/** Add only the exclusive graph marker; callers supply complete graph statements. */
static char *request_text(const char *body) {
    size_t size = strlen("@...\n") + strlen(body) + 1;
    char *text = malloc(size);
    assert(text != NULL);
    strcpy(text, "@...\n");
    strcat(text, body);
    return text;
}

/** Select the actual other definition explicitly, never by inspecting a body string. */
static inline void setup_request_definitions(const char *body, const char *other) {
    char *text = request_text(body);
    setup_binding(text, other);
    free(text);
}
/** Use a known ordinary constructor for Type/Template applications. */
static inline void setup_requests(const char *body) {
    setup_request_definitions(body, request_producer);
}
/** Preserve real configured standard/import dependencies for named graph sources. */
static inline void setup_request_roots(const char *body, const ModuleRoot *roots, size_t root_count) {
    char *text = request_text(body);
    setup_binding_roots(text, request_producer, roots, root_count);
    free(text);
}
/** Select the exact original root occurrence in the graph, not a foreign label. */
static inline const StaticOccurrence *request_head(const char *text) {
    const StaticModule *module = binding_module(project.entry);
    for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (site->receiver == NULL && site->root && layout_text_is(site->site, text)) { return site; }
    }
    assert(false); return NULL;
}
/** Snapshot the explicitly supplied actual ordinary constructor's full residuals. */
static inline BindingInterface *request_interface(void) {
    BindingInterface *view = NULL;
    assert(binding_source_interface(&binding_plan, binding_module(project_find(&project, ".other")),
        BINDING_MODULE_SURFACE, NULL, 0, &view));
    return view;
}
#endif
