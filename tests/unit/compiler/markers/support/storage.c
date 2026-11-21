/** @file
 * @brief Exercise all ordinary storage forms and misuse of marker producer interfaces.
 */
#include "prepare.c"

/** Refuse original storage even when references/lists are empty or nested code is unreachable. */
static void it_rejects_direct_and_exported_marker_storage_in_every_form(void) {
    const char *names[] = {"item", "item&", "item[]", "item&[]"};
    for (size_t selected = 0; selected < 2; selected++) {
        for (size_t form = 0; form < 4; form++) {
            char text[160];
            int written = snprintf(text, sizeof(text), "@!\n%s\n%s\n%s", names[form],
                selected ? "./assembly" : "./node", selected ? " .node kind\n" : "");
            assert(written > 0 && (size_t)written < sizeof(text));
            prepare_marker_project(text);
            setup_frontend();
            assert(!check_project() && !frontend.valid);
            assert(diagnostic_valid(&frontend.error));
            assert(strstr(frontend.error.message, "marker") != NULL);
            destroy_frontend();
        }
    }
}

/** A nominal marker has neither a producer interface nor public selected outputs or constants. */
static void it_rejects_marker_producer_selection_application_and_constant_reads(void) {
    const char *entries[] = {
        "@...\n./node\n .out > .received\n",
        "@...\n.kind\n./node\n .input < ./value\n",
        "@!\nitem\n./node\n .out\n",
        "@!\nitem\n./value\nitem\n ./node/member\n",
        "@=\nitem\n./node\n@!\n"
    };
    for (size_t index = 0; index < sizeof(entries) / sizeof(*entries); index++) {
        prepare_marker_project(entries[index]);
        setup_frontend();
        assert(!check_project() && !frontend.valid);
        assert(diagnostic_valid(&frontend.error));
        destroy_frontend();
    }
}

/** An annotation-only marker can load as source metadata but cannot be an executable root. */
static void it_rejects_marker_executable_roots(void) {
    prepare_marker_project("@...\n");
    setup_frontend();
    assert(!check_project() && !frontend.valid);
    assert(diagnostic_valid(&frontend.error));
    destroy_frontend();
}

/** A closed constructor can carry a marker as a Type, but storing its substituted
 * runtime output remains invalid for every field representation.
 */
static void it_rejects_substituted_marker_fields_and_uninstantiated_definitions(void) {
    const char *names[] = {"item", "item&", "item[]", "item&[]"};
    for (size_t form = 0; form < 4; form++) {
        prepare_marker_project("@!\nvalue\n./assembly\n .boxed\n");
        char generic[100];
        int size = snprintf(generic, sizeof(generic), "@>\n%s\n.slot\n@!\n", names[form]);
        assert(size > 0 && (size_t)size < sizeof(generic));
        file("generated/project/packages/test/fixture/v1.0.0/src/box", generic);
        file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
            "@...\n.boxed\n./box\n .slot < ./node\n");
        setup_frontend();
        assert(!check_project() && !frontend.valid);
        assert(diagnostic_valid(&frontend.error));
        assert(strstr(frontend.error.message, "marker") != NULL);
        destroy_frontend();
    }
    prepare_marker_project("@!\nunused&\n./bad\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/bad", "@!\nitem\n./node\n");
    setup_frontend();
    assert(!check_project() && !frontend.valid);
    assert(diagnostic_valid(&frontend.error));
    destroy_frontend();
}
