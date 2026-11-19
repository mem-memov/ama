/** @file
 * @brief Load actual static graph producers and an ordinary two-input constructor for request-map tests.
 */
#ifndef AMA_STATIC_PRODUCER_REQUEST_TEST_COMMON
#define AMA_STATIC_PRODUCER_REQUEST_TEST_COMMON
#include "../common.c"

/** The two source applications differ only by which inputs were explicitly
 * supplied; each omitted foreign formal still needs its own lexical hole.
 */
static const char *request_caller = "@...\n.other\n ...left < ..integer\n ...out > ...first\n"
    ".other\n ...out > ...second\n...anchor\n..integer\n";
/** Both original inputs occur in output structure; a selected output does not
 * erase other public outputs or change input order.
 */
static const char *request_target = "@...\n...out\n.pair\n ...a < ...left\n ...b < ...right\n"
    "...forward\n...right\n";

/** Initialize source/term owners without any actual producer entry or equation. */
static void setup_producer_requests(const char *caller, const char *target) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/pair", "@<\nfirst\n...a\nsecond\n...b\n@!\n");
    setup_guards(caller, target);
    initialize_results();
}

#endif
