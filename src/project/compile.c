#include "project/internal.h"
#include "graph/graph.h"
#include "symbols/table.h"
#include "codegen/intrinsics.h"
#include "codegen/emitter.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static void load_project(const char* root, const char* entry) {
    scan_dir(root, "");
    if (G_FileCount == 0) {
        fprintf(stderr, "error: no module files found under '%s'\n", root);
        exit(1);
    }
    printf("Found %d module file(s) under '%s'\n", G_FileCount, root);
    /* Include modules without declarations in the dependency graph. */
    for (int i = 0; i < G_FileCount; i++)
        ensure_graph_node(G_Files[i].rel_path);
    register_intrinsics();
    pass1_declarations();
    printf("Symbol table: %u name(s)\n", (unsigned)G_Table.count);
    validate_entry(entry);
}

static void begin_output(void) {
    /* Locals are discovered during emission; declare them after buffering bodies. */
    G_Out = tmpfile();
    if (!G_Out) {
        fprintf(stderr, "error: cannot create temporary output: %s\n", strerror(errno));
        exit(1);
    }
    backend_emit_intrinsics(G_Out);
}

static int emit_user_modules(const int* order, int count) {
    int emitted = 0;
    for (int i = 0; i < count; i++) {
        const char* path = G_Graph_path(order[i]);
        if (!path) {
            fprintf(stderr, "internal error: graph node %d has null path\n", i);
            exit(1);
        }
        if (is_intrinsic_path(path))
            continue;
        ModuleFile* module = find_module_file(path);
        if (!module) {
            fprintf(stderr, "internal error: module '%s' in graph has no source "
                    "file (was it removed during compilation?)\n", path);
            exit(1);
        }
        printf("  [%d] %s\n", ++emitted, path);
        pass2_module_code(module->rel_path, module->abs_path);
    }
    return emitted;
}

static void write_output(const char* out_path) {
    FILE* body = G_Out;
    if (fflush(body) != 0 || fseek(body, 0, SEEK_SET) != 0) {
        fprintf(stderr, "error: cannot rewind temporary output\n");
        exit(1);
    }
    FILE* out = fopen(out_path, "w");
    if (!out) {
        fprintf(stderr, "error: cannot open '%s': %s\n", out_path, strerror(errno));
        exit(1);
    }
    backend_emit_prologue(out);
    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), body)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) {
            fprintf(stderr, "error: cannot write '%s'\n", out_path);
            exit(1);
        }
    }
    int read_failed = ferror(body);
    int body_failed = fclose(body);
    int out_failed = fclose(out);
    if (read_failed || body_failed || out_failed) {
        fprintf(stderr, "error: failed to finish '%s'\n", out_path);
        exit(1);
    }
    G_Out = NULL;
}

void compile_project(const char* root, const char* entry, const char* out_path) {
    load_project(root, entry);
    int order[MAX_MODULES];
    int count = build_and_validate_project_graph(order);
    printf("Topological order: %d node(s)\n", count);
    begin_output();
    int emitted = emit_user_modules(order, count);
    backend_emit_main(G_Out, entry);
    write_output(out_path);
    printf("Wrote '%s' (%d user module(s) emitted)\n", out_path, emitted);
}
