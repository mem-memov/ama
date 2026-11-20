/** @file
 * @brief Detect implicit method clones and incorrect sibling/depth-first sequencing.
 */
/** Any clone of this fixture's worker is an observable failure; value argument
 * transfers still retain their ordinary value clone contract.
 */
static const char *forbid_method_clone(const ScaffoldType *type, AbiOperation operation) {
    if (operation != ABI_CLONE || strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/worker") != 0) { return NULL; }
    return " { (void)destination; (void)source; return ama_status_make(AMA_NATIVE_FAILURE,\"unexpected worker clone\",23); }\n";
}

/** Repeated selected methods mutate actual result instances; a nested subtree
 * completes before the parent's next sibling, and ordinary receivers are caller-local.
 */
static void it_executes_retained_owned_methods_depth_first_without_cloning(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/branch", "@>\nmethod\n/test/fixture/v1.0.0/src/worker\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/constructor", "@>\nleft\n/test/fixture/v1.0.0/src/branch\nright\n/test/fixture/v1.0.0/src/worker\n@!\n");
    native_hook_override = forbid_method_clone;
    execute_project("@!\nobject\n/test/fixture/v1.0.0/src/constructor\nprobe\n/test/fixture/v1.0.0/src/worker\nobject !\n left\n  method !\n   < |11\n   > result\n right !\n  < result\nobject\n left\n  method !\n   < |22\n right !\n  < |33\nprobe !\n < result\n", "11:1:0\n12:1:0\n22:2:0\n33:2:0\n12:1:0\n", true);
    native_hook_override = NULL;
}
