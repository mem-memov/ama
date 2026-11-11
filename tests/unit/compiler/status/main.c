#include "../common.c"
#include "compiler/inspect/module.h"

static void it_reports_all_four_activation_states_without_loading_dependencies(void) {
    directory("generated/project");
    const char *texts[] = {"unfinished\t body\n", "@=\n", "@!\nfield\n.missing\n", "@C\n"};
    for (size_t index = 0; index < 4; index++) {
        file("generated/project/main", texts[index]);
        setup_frontend();
        ModuleInspection inspection;
        compiler_inspection_init(&inspection, sources.allocator);
        ModuleRoot root = {"generated/project", NULL, 0};
        bool valid = compiler_inspect_module(&inspection, &root, 1, ".main");
        assert(valid && inspection.syntax.sections.kind == (ParsedModuleKind)index);
        compiler_inspection_destroy(&inspection);
        destroy_frontend();
    }
}

static void it_releases_every_partial_inspection_allocation(void) {
    directory("generated/project");
    file("generated/project/main", "@!\n");
    setup_frontend();
    ModuleInspection inspection;
    compiler_inspection_init(&inspection, sources.allocator);
    ModuleRoot root = {"generated/project", NULL, 0};
    bool valid = compiler_inspect_module(&inspection, &root, 1, ".main");
    assert(valid);
    size_t total = allocation_count;
    compiler_inspection_destroy(&inspection);
    destroy_frontend();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_frontend();
        compiler_inspection_init(&inspection, sources.allocator);
        failure_at = failure;
        valid = compiler_inspect_module(&inspection, &root, 1, ".main");
        assert(!valid && inspection.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        compiler_inspection_destroy(&inspection);
        destroy_frontend();
    }
}

void run(void) {
    it_reports_all_four_activation_states_without_loading_dependencies();
    it_releases_every_partial_inspection_allocation();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
