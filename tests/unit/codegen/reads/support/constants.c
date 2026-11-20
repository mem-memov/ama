/** @file
 * @brief Distinguish immutable namespace reads from hidden instance construction or executable module bodies.
 */
/** A constructor marker makes any accidental default instance observable without using mutable native global accounting. */
static const char *alarm_constructor(const ScaffoldType *type, AbiOperation operation) {
    if (operation != ABI_DEFAULT || strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/alarm") != 0) { return NULL; }
    return " { *destination=(AMA_NATIVE(State)){.address=destination}; return fputs(\"unexpected-instance\\n\",stdout)==EOF ? ama_status_make(AMA_IO_FAILURE,NULL,0) : ama_status_success(); }\n";
}

/** Constant namespace and qualified member sources copy exact values; a constant dependency initializes before its importer without constructing a default namespace instance. */
static void it_reads_qualified_and_namespace_constants_without_constructing_or_executing_their_modules(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("alarm", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/palette", "@=\nred\n/test/fixture/v1.0.0/src/value\n |11\nblue\n/test/fixture/v1.0.0/src/value\n |22\n@!\nalarm\n/test/fixture/v1.0.0/src/alarm\nalarm !\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/other", "@=\ncopied\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/palette/blue\n");
    native_hook_override = alarm_constructor;
    execute_project("@!\nprobe\n/test/fixture/v1.0.0/src/worker\n/test/fixture/v1.0.0/src/palette\n >> first\n >> second\nprobe !\n < first\nprobe !\n < second\nprobe !\n < /test/fixture/v1.0.0/src/palette/red\n/test/fixture/v1.0.0/src/other\n >> third\nprobe !\n < third\n", "11:1:0\n22:2:0\n11:3:0\n22:4:0\n", true);
    native_hook_override = NULL;
}
