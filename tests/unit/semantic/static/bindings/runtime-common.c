/** @file
 * @brief Prepare ordinary runtime request fixtures separately from static graph syntax.
 */
#ifndef AMA_RUNTIME_REQUEST_TEST_COMMON
#define AMA_RUNTIME_REQUEST_TEST_COMMON
#include "common.c"
#include "semantic/static/bindings/normalize.h"

/** Runtime values keep their own names/storage forms and ordinary Type pairs. */
static const char *runtime_request_prefix = "@<\ninput&\n.type\nsource[]\n.type\n"
    "@>\nreturned\n.type\n@!\n";
static const char *runtime_request_producer = "@<\nfirst&\n.a\nsecond[]\n.b\n"
    "@>\nx&\n.a\ny[]\n.b\n@!\n";

/** Build one ordinary body without inserting any static interface definitions. */
static char *runtime_request_text(const char *body) {
    size_t size = strlen(runtime_request_prefix) + strlen(body) + 1;
    char *text = malloc(size);
    assert(text != NULL);
    strcpy(text, runtime_request_prefix);
    strcat(text, body);
    return text;
}
/** Prepare the actual runtime callee and exact two-definition source domain. */
static inline void setup_runtime_requests(const char *body) {
    char *text = runtime_request_text(body);
    setup_binding(text, runtime_request_producer);
    free(text);
}
/** Add only explicitly supplied standard/import root families. */
static inline void setup_runtime_roots(const char *body, const ModuleRoot *roots, size_t root_count) {
    char *text = runtime_request_text(body);
    setup_binding_roots(text, runtime_request_producer, roots, root_count);
    free(text);
}
#endif
