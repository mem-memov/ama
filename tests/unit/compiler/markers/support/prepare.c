/** @file
 * @brief Prepare exact independent marker, ordinary value and static-export sources.
 */
#ifndef AMA_MARKER_PREPARE
#define AMA_MARKER_PREPARE
#include "../../common.c"
#include "semantic/static/terms/store.h"

/** Every dependency is version-local; equal marker bytes deliberately have unequal paths. */
static void prepare_marker_project(const char *entry) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", entry);
    file("generated/project/packages/test/fixture/v1.0.0/src/main.ini", "");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly.ini", "~node = ./node\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/forward", "@...\n.result\n.input\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/node", "@...\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/edge", "@...\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
        "@...\n.node kind\n~node\n.same node kind\n.node kind\n.edge kind\n./edge\n./forward\n .input < .node kind\n .result > .forwarded\n.value\n./value\n");
}
#endif
