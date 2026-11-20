/** @file
 * @brief Reject transitive duplication of enrollment owners and in-flight native resources.
 */
/** Missing native clone capability propagates through ordinary records and even empty owning registries, for both copy modes. */
static void it_rejects_copying_enrollment_owners_and_inflight_native_operations(void) {
    const char *types[] = {"//task/enrollment", "/test/fixture/v1.0.0/src/kind/record", "/test/fixture/v1.0.0/src/scheduler", "//task/operation"};
    for (size_t type = 0; type < sizeof(types) / sizeof(*types); type++) {
        for (size_t continuation = 0; continuation < 2; continuation++) {
            boundary_project();
            char source[512];
            int length = snprintf(source, sizeof(source), "@!\nsource\n%s\ncopy\n %ssource\n",
                types[type], continuation ? "% " : "");
            assert(length > 0 && (size_t)length < sizeof(source));
            file("generated/project/packages/test/fixture/v1.0.0/src/main", source);
            setup_frontend();
            ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
                {TEST_STDLIB, NULL, 0, MODULE_ROOT_STANDARD}};
            bool checked = compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main");
            assert(!checked && frontend.error.message != NULL);
            assert(strstr(frontend.error.message, "independent copying") != NULL);
            assert(frontend.error.cause != NULL && strstr(frontend.error.cause, "no declared = copy capability") != NULL);
            destroy_frontend();
        }
    }
}
