/** @file
 * @brief Load complete analysis domains without fabricating an executable entry.
 */
/** Constants-only modules and an empty domain are valid for source tools, while entry-based compilation retains its checks. */
static void it_loads_constants_only_and_empty_analysis_sets_without_an_entry(void) {
    fixture_package_prepare("generated/analysis-set", "fixture");
    file("generated/analysis-set/packages/test/fixture/v1.0.0/src/names","@=\nflag\n/test/fixture/v1.0.0/src/value\n |1\n");
    file("generated/analysis-set/packages/test/fixture/v1.0.0/src/value","@!\n");
    setup_project();
    ModuleRoot root = {.path="generated/analysis-set",.kind=MODULE_ROOT_PROJECT};
    assert(module_catalog_build(&project.catalog,&root,1) == MODULE_OK);
    const char *required[] = {"/test/fixture/v1.0.0/src/names"};
    assert(project_load_catalog_set(&project,required,1) == PROJECT_OK && project.valid && project.entry == NULL);
    assert(module_count() == 2 && project_find(&project,"/test/fixture/v1.0.0/src/names") != NULL);
    destroy_project();
    setup_project();
    assert(module_catalog_build(&project.catalog,NULL,0) == MODULE_OK);
    assert(project_load_catalog_set(&project,NULL,0) == PROJECT_OK && project.valid && project.entry == NULL);
    assert(module_count() == 0);
    destroy_project();
}
