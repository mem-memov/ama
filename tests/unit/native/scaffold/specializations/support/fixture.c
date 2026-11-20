/** @file
 * @brief Own two real captured owners and a counted scaffold registry without running C emission.
 * @see [T188 contract](../../../../../../docs/implementation/specialized-execution.md)
 */
#include "../../../../compiler/common.c"
#include "native/scaffold/model/identity.h"
#include "semantic/specializations/identity/key.h"

/** Borrow concrete frontend owners; the scaffold is destroyed before their semantic/source metadata. */
static NativeScaffold scaffold;
static const SemanticType *owners[2];

/** Resolve two distinct bindings of one declaration and retain their exact semantic pointers. */
static void setup_owners(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\n/test/fixture/v1.0.0/src/factory\n .numbers\n first >> number\n/test/fixture/v1.0.0/src/factory\n .texts\n first >> text\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/container\n .slot < //integer\n.texts\n/test/fixture/v1.0.0/src/container\n .slot < //text\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/container", "@=\nfirst\n.slot\n@<\ninput\n.slot\n@!\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    size_t count = 0;
    const ProjectModule *definition = project_find(&frontend.project, "/test/fixture/v1.0.0/src/container");
    for (const SemanticType *type = frontend.semantic.first; type; type = type->next) {
        if (type->module == definition) { assert(count < 2); owners[count++] = type; }
    }
    assert(count == 2 && owners[0] != owners[1]);
    native_scaffold_init(&scaffold, &frontend, sources.allocator);
}

/** Release the registry before borrowed concrete owner metadata and check counted allocation balance. */
static void destroy_owners(void) {
    native_scaffold_destroy(&scaffold);
    destroy_frontend();
}
