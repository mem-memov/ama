#ifndef AMA_PROJECT_INTEGRATION_COMMON
#define AMA_PROJECT_INTEGRATION_COMMON

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "project/dependencies/project.h"
#include "diagnostics/render/render.h"

static Project project;

static void setup_test(void) {
    project_init(&project, source_system_allocator());
}

static void destroy_test(void) {
    project_destroy(&project);
}

static void expect_diagnostic(const char *expected_path) {
    FILE *actual = tmpfile();
    FILE *expected = fopen(expected_path, "rb");
    assert(actual != NULL && expected != NULL);
    DiagnosticRenderStatus rendered = diagnostic_render(actual, &project.error);
    assert(rendered == DIAGNOSTIC_RENDER_OK);
    rewind(actual);
    int left;
    int right;
    do {
        left = fgetc(actual);
        right = fgetc(expected);
        assert(left == right);
    } while (left != EOF);
    assert(!ferror(actual) && !ferror(expected));
    int closed_actual = fclose(actual);
    int closed_expected = fclose(expected);
    assert(closed_actual == 0 && closed_expected == 0);
}

#endif
