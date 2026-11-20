/** @file
 * @brief Reject wrong storage forms, constructor categories, unknown formals and noncopyable element requirements.
 */
/** Each failed application owns its actual complete frontend attempt; no default
 * Type, sample value or runtime Template is synthesized to accept the example. */
static void it_rejects_invalid_algorithm_categories_storage_and_copy_capabilities(void) {
    algorithm_fixture();
    native_fixture("limited","@C\n");
    file("generated/project/limited.abi","+ initialize\n- destroy\n");
    file("generated/project/wrong-category","@...\n.complete\n ...family < ..integer\n ...ready > ...result\n");
    file("generated/project/unknown-formal","@...\n...result\n.reverse-owned\n ...unknown < ..integer\n");
    file("generated/project/open-result","@...\n...result\n.reverse-owned\n");
    const char *invalid[]={
        "@!\nitems[]\n..integer\nreverse\n.reverse-references\nreverse !\n < items[]\n",
        "@!\nitems[]\n.limited\nreverse\n.reverse-owned\nreverse !\n < items[]\n",
        "@!\nvalue\n.wrong-category\n ...result\n",
        "@!\nvalue\n.unknown-formal\n ...result\n",
        "@!\nvalue\n.open-result\n ...result\n"
    };
    for (size_t i=0;i<sizeof(invalid)/sizeof(*invalid);i++) {
        file("generated/project/main",invalid[i]); setup_frontend();
        ModuleRoot roots[]={{"generated/project",NULL,0,MODULE_ROOT_PROJECT},
            {TEST_STDLIB,NULL,0,TEST_STDLIB_KIND}};
        bool valid=compiler_frontend_check(&frontend,roots,2,".main");
        assert(!valid && !frontend.valid && !frontend.plan_initialized && diagnostic_valid(&frontend.error));
        destroy_frontend();
    }
}
