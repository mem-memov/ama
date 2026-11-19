/** @file
 * @brief Exercise unchanged and changed library source graphs against independently specified consumer sets.
 * @see [T191 contract](../../../../../docs/implementation/template-source-delivery.md)
 */

/** Original module consumers are source units, not separately cached specialized
 * machine code; a dirty entry rebuilds its one complete project assembly. */
static const char *const delivery_modules[] = {".main", ".independent", ".lib.assembly",
    ".lib.forward", ".lib.value", ".lib.counter", ".lib.sink"};

/** Append fixture bytes without copying or interpreting a source definition. */
static void append_delivery_bytes(const char *path, const char *text) {
    FILE *output = fopen(path, "ab");
    assert(output && fputs(text, output) != EOF && fclose(output) == 0);
}

/** Mutations are explicit source changes, not inferred modifications to virtual
 * files. Unused valid files stay outside the required source/input closure. */
static void change_delivery_input(size_t mutation) {
    switch (mutation) {
    case 0: break;
    case 1: append_delivery_bytes("generated/library/lib/forward", "# changed template definition\n"); break;
    case 2:
        file("generated/library/lib/assembly", "@...\n...result\n.lib.forward\n ...slot < .lib.value\n...unused\n.lib.value\n");
        break;
    case 3: append_delivery_bytes("generated/library/lib/value.h", "/* changed native layout source */\n"); break;
    case 4: file("generated/library/lib/value.c", "#error Frontend snapshots must not execute the C compiler\n"); break;
    case 5: file("generated/library/lib/value.abi", "+ initialize\n- destroy\n| literal\n= duplicate\n? condition\n[] index\n"); break;
    case 6: {
        file("generated/library/lib/alternate", delivery_assembly_source);
        const char *selected = strstr(delivery_program, ".lib.assembly");
        assert(selected);
        FILE *output = fopen("generated/project/main", "wb");
        assert(output && fwrite(delivery_program, 1, (size_t)(selected - delivery_program), output) == (size_t)(selected - delivery_program));
        assert(fputs(".lib.alternate", output) != EOF);
        assert(fputs(selected + strlen(".lib.assembly"), output) != EOF && fclose(output) == 0);
        break;
    }
    case 7: file("generated/library/lib/unrelated", "@!\n# unused source change\n"); break;
    case 8: append_delivery_bytes("generated/library/lib/counter.h", "/* changed private native dependency */\n"); break;
    case 9: file("generated/project/independent", "@!\n# changed independent branch\n"); break;
    default: assert(false);
    }
}

/** Each changed consumer set follows the declared source edges. In particular,
 * a concrete captured Type changes its assembly/sink/entry consumers, while the
 * open forward definition itself has no hidden direct edge to that Type. */
static void it_invalidates_exact_source_library_consumers_and_preserves_noop_snapshots(void) {
    static const bool expected[][7] = {
        {false,false,false,false,false,false,false},
        {true,false,true,true,false,false,false},
        {true,false,true,false,false,false,false},
        {true,false,true,false,true,false,true},
        {true,false,true,false,true,false,true},
        {true,false,true,false,true,false,true},
        {true,false,false,false,false,false,false},
        {false,false,false,false,false,false,false},
        {true,false,true,true,false,true,false},
        {true,true,false,false,false,false,false}
    };
    prepare_delivery_sources();
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/library", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(compiler_frontend_check(&frontend, roots, 3, ".main"));
    size_t live = live_allocations;
    for (size_t mutation = 0; mutation < sizeof(expected) / sizeof(expected[0]); mutation++) {
        prepare_delivery_sources();
        change_delivery_input(mutation);
        CompilerFrontend after;
        compiler_frontend_init(&after, sources.allocator);
        bool checked = compiler_frontend_check(&after, roots, 3, ".main");
        if (!checked) { assert(diagnostic_render(stderr, &after.error) == DIAGNOSTIC_RENDER_OK); }
        assert(checked && after.inputs.sealed);
        assert(project_inputs_compare(&after.inputs, &frontend.inputs) == SOURCE_OK);
        for (size_t index = 0; index < sizeof(delivery_modules) / sizeof(delivery_modules[0]); index++) {
            bool changed = false;
            assert(project_inputs_changed(&after.inputs, PROJECT_INPUT_MODULE, delivery_modules[index], &changed));
            if (changed != expected[mutation][index]) {
                fprintf(stderr, "mutation %zu: %s changed=%d expected=%d\n", mutation, delivery_modules[index], changed, expected[mutation][index]);
            }
            assert(changed == expected[mutation][index]);
        }
        assert(project_inputs_find(&after.inputs, PROJECT_INPUT_MODULE, ".lib.unrelated") == NULL);
        bool changed = false;
        if (mutation == 6) {
            assert(project_inputs_changed(&after.inputs, PROJECT_INPUT_MODULE, ".lib.alternate", &changed) && changed);
        } else { assert(project_inputs_find(&after.inputs, PROJECT_INPUT_MODULE, ".lib.alternate") == NULL); }
        compiler_frontend_destroy(&after);
        assert(live_allocations == live);
    }
    destroy_frontend();
}
