/** @file
 * @brief Own a source-only open-template library physically separate from its consuming Ama project.
 * @see [T191 contract](../../../../docs/implementation/template-source-delivery.md)
 */
#ifndef AMA_SOURCE_DELIVERY_TEST_COMMON
#define AMA_SOURCE_DELIVERY_TEST_COMMON
#include "../common.c"

/** Repeated uses denote one Type/code identity; private counters belong to each
 * ordinary instance. The independent branch is observable to input snapshots. */
static const char *const delivery_program =
    "@!\nquiet\n/test/fixture/v1.0.0/src/independent\nfirst\n/test/library/v1.0.0/src/lib/assembly\n .result\nsecond\n/test/library/v1.0.0/src/lib/assembly\n .result\nsink\n/test/library/v1.0.0/src/lib/sink\nfirst !\n < |17\n > result\nsink !\n < result\nsecond !\n < |17\n > result\nsink !\n < result\nfirst !\n < |23\n > result\nsink !\n < result\n";

/** One closed export and one deliberately partial public output are source
 * metadata; only the closed export is selected by ordinary consumers. */
static const char *const delivery_assembly_source =
    "@...\n.result\n/test/library/v1.0.0/src/lib/forward\n .slot < /test/library/v1.0.0/src/lib/value\n.unused\n/test/library/v1.0.0/src/lib/forward\n";

/** Write the existing concrete native companion protocol without a compiled or
 * serialized template interface. Compiled cases complete these test-owned bodies. */
static void delivery_native(const char *name, const char *interface) {
    char path[192];
    const char *suffixes[] = {"", ".h", ".c", ".abi"};
    const char *contents[] = {interface,
        "struct AMA_NATIVE(State) { size_t number; size_t calls; const void *address; };\n",
        "/* Authored fixture body, completed only by compiled consumer cases. */\n",
        "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n"};
    for (size_t index = 0; index < sizeof(suffixes) / sizeof(suffixes[0]); index++) {
        int length = snprintf(path, sizeof(path), "generated/library/packages/test/library/v1.0.0/src/lib/%s%s", name, suffixes[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, contents[index]);
    }
}

/** The physical library root contributes explicit lib path components; its
 * directory name itself does not add an implicit logical namespace prefix. */
static void prepare_delivery_sources(void) {
    fixture_package_prepare("generated/project", "fixture");
    fixture_package_prepare("generated/library", "library");
    directory("generated/library/packages/test/library/v1.0.0/src/lib");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", delivery_program);
    file("generated/project/packages/test/fixture/v1.0.0/src/independent", "@!\n");
    file("generated/library/packages/test/library/v1.0.0/src/lib/forward", "@<\ninput\n.slot\n@>\noutput\n.slot\n input\n@!\nprivate\n/test/library/v1.0.0/src/lib/counter\nprivate !\noutput\n input\n");
    file("generated/library/packages/test/library/v1.0.0/src/lib/assembly", delivery_assembly_source);
    delivery_native("value", "@C\n");
    delivery_native("counter", "@C\n");
    delivery_native("sink", "@<\ninput\n/test/library/v1.0.0/src/lib/value\n@>\noutput\n/test/library/v1.0.0/src/lib/value\n@C\n");
}
#endif
