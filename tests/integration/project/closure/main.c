#include "../common.c"

static void it_renders_the_complete_unicode_dependency_chain(void) {
    setup_test();
    ModuleRoot root = {"project", NULL, 0};
    ProjectStatus status = project_load(&project, &root, 1, ".main");
    assert(status == PROJECT_MISSING && !project.valid);
    expect_diagnostic("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_renders_the_complete_unicode_dependency_chain();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
