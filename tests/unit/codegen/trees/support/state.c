/** @file
 * @brief Detect implicit method clones and incorrect sibling/depth-first sequencing.
 */
/** Any clone of this fixture's worker is an observable failure; value argument
 * transfers still retain their ordinary value clone contract.
 */
static const char *forbid_method_clone(const ScaffoldType *type, AbiOperation operation) {
    if (operation != ABI_CLONE || strcmp(type->plan->type->module->source->logical_path, ".worker") != 0) { return NULL; }
    return " { (void)destination; (void)source; return ama_status_make(AMA_NATIVE_FAILURE,\"unexpected worker clone\",23); }\n";
}

/** Repeated selected methods mutate actual result instances; a nested subtree
 * completes before the parent's next sibling, and ordinary receivers are caller-local.
 */
static void it_executes_retained_owned_methods_depth_first_without_cloning(void) {
    directory("generated/project");
    file("generated/project/branch", "@>\nmethod\n.worker\n@!\n");
    file("generated/project/constructor", "@>\nleft\n.branch\nright\n.worker\n@!\n");
    native_hook_override = forbid_method_clone;
    execute_project("@!\nobject\n.constructor\nprobe\n.worker\nobject !\n left\n  method !\n   < |11\n   > result\n right !\n  < result\nobject\n left\n  method !\n   < |22\n right !\n  < |33\nprobe !\n < result\n", "11:1:0\n12:1:0\n22:2:0\n33:2:0\n12:1:0\n", true);
    native_hook_override = NULL;
}
