/**
 * @file
 * @brief Validate complete generated C assembly, source selectors, exact IR sites and allocation-failure unwind.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../../compiler/common.c"
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include "codegen/codegen.h"
#include "native/scaffold/internal.h"
#include "codegen/project/metadata/internal.h"

static NativeScaffold assembly;

/**
 * Create the explicit native/source model used by project assembly tests; publication remains
 * separate from in-memory generation.
 */
static void prepare_assembly(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@=\n常量\n/test/fixture/v1.0.0/src/value\n |native bytes\n@!\nfirst\n/test/fixture/v1.0.0/src/value\nsecond\n/test/fixture/v1.0.0/src/other\nreference&\n/test/fixture/v1.0.0/src/node\n");
    const char *names[] = {"value", "other"};
    for (size_t index = 0; index < 2; index++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s", names[index]);
        assert(length > 0);
        file(path, "@C\n");
        length = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.h", names[index]);
        assert(length > 0);
        file(path, index == 0 ? "#ifndef VALUE_H\n#define VALUE_H\nstruct AMA_NATIVE(State) { size_t number; };\n#endif\n"
            : "#ifndef OTHER_H\n#define OTHER_H\nstruct AMA_NATIVE(State) { size_t number; };\n#endif\n");
        length = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.c", names[index]);
        assert(length > 0);
        file(path, "/* Replaced by the test's handwritten native definition. */\n");
        length = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.abi", names[index]);
        assert(length > 0);
        file(path, "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n");
    }
    file("generated/project/packages/test/fixture/v1.0.0/src/node", "@!\nnext&\n/test/fixture/v1.0.0/src/node\n");
}

/**
 * Initialize the complete checked frontend and scaffold fixture owners in dependency order
 * before emission.
 */
static void begin_assembly(void) {
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    bool checked = compiler_frontend_check(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main");
    assert(checked);
    native_scaffold_init(&assembly, &frontend, sources.allocator);
}

/**
 * Build the real project artifact set under the fixture options and return the production result
 * for assertions.
 */
static bool build_assembly(void) {
    bool built = project_delivery_build(&assembly, (ScaffoldOptions){.directory = "generated/output", .sdk = TEST_SDK}, &(DeliveryRequest){.mode = DELIVERY_EXECUTABLE, .component = "test.codegen"});
    if (!built && assembly.error.message != NULL) {
        fprintf(stderr, "Assembly: %s (%s)\n", assembly.error.message,
            assembly.error.primary.path == NULL ? "no path" : assembly.error.primary.path);
    }
    return built;
}

/**
 * Destroy scaffold before its borrowed frontend/source storage, then verify counted allocations
 * are released.
 */
static void end_assembly(void) {
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}

/**
 * Assert that the component composes the real runtime and qualified native sources in one c
 * unit. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_composes_the_real_runtime_and_qualified_native_sources_in_one_c_unit(void) {
    prepare_assembly();
    begin_assembly();
    bool built = build_assembly();
    assert(built);
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
        FILE *body = fopen(type->body_path, "wb");
        assert(body != NULL);
        bool emitted = scaffold_emit_body(&assembly, body, type);
        int closed = fclose(body);
        assert(emitted && closed == 0);
    }
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror",
            "-I" TEST_SDK, "-Igenerated/output", "-fsyntax-only", "generated/output/launcher.c", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    end_assembly();
}

/** Parse the actual owned manifest independently and verify exact graph edges,
 * names, transport omissions and complete nominal keys, including a self reference.
 */
static void it_emits_a_finite_exact_delivery_manifest_for_recursive_types(void) {
    prepare_assembly();
    file("generated/project/packages/test/fixture/v1.0.0/src/main",
        "@<\ninput name\n/test/fixture/v1.0.0/src/value\n"
        "@!\nreference&\n/test/fixture/v1.0.0/src/node\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value.cmake", "message(FATAL_ERROR \"Generation must not run CMake\")\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/value.extra", "declared resource\n");
    directory("generated/project/packages/test/fixture/v1.0.0/src/other.cmake");
    directory("generated/project/packages/test/fixture/v1.0.0/README.md");
    begin_assembly();
    bool built = build_assembly(); assert(built);
    bool published = native_scaffold_publish(&assembly); assert(published);
    pid_t process = fork(); assert(process >= 0);
    if (!process) {
        execlp("python3", "python3", "-c",
            "import json; from pathlib import Path; "
            "m=json.loads(Path('generated/output/delivery.json').read_bytes()); "
            "assert m['format']=='ama.delivery.v1' and m['mode']=='executable'; "
            "assert m['component']=='test.codegen'; "
            "decode=lambda x:bytes.fromhex(x['hex']); "
            "assert decode(m['working_directory'])==str(Path.cwd()).encode(); "
            "native=m['native']; assert len(native)==1 and native[0]['module'].endswith('/src/value'); "
            "nr=native[0]; assert nr['cmake']['state']=='present'; "
            "assert nr['version']=='/test/fixture/v1.0.0' and decode(nr['version_root']).endswith(b'/v1.0.0'); "
            "assert b'FATAL_ERROR' in decode(nr['cmake']['file']['content']); "
            "assert nr['readme']['state']=='unavailable' and nr['readme']['issue']=='shape'; "
            "assert len(m['guidance'])==1 and m['guidance'][0]['readme']==nr['readme']; "
            "assert all(decode(nr[k]['content'])==Path(decode(nr[k]['path']).decode()).read_bytes() for k in ['c','h','abi']); "
            "assets=nr['resources']; assert len(assets)==2; "
            "assert any(decode(x['snapshot']['file']['content'])==b'declared resource\\n' for x in assets if not x['directory']); "
            "files={decode(x['path']):x for x in m['generated']}; "
            "v=m['verification_entries']; assert len(v)==1 and v[0]['main'] is True; "
            "entry=decode(v[0]['path']); assert entry in files and files[entry]['entry']; "
            "assert sum(x['entry'] for x in files.values())==1; "
            "assert all(Path(p.decode()).stat().st_size==int(x['length']) for p,x in files.items()); "
            "db=json.loads(Path('generated/output/compile_commands.json').read_bytes()); "
            "assert db[0]['file'].encode()==entry; "
            "d=m['directory']; assert len(bytes.fromhex(d['hex']))==int(d['length']); "
            "nodes={x['id']:x for x in m['types']}; assert len(nodes)==len(m['types']); "
            "assert nr['types'] and all(nodes[i]['module']==nr['module'] for i in nr['types']); "
            "assert nodes[m['root']]['module'].endswith('/src/main'); "
            "assert nodes[m['root']]['fields'][0]['role']=='parameter'; "
            "assert m['inputs']==[{'name':'input name','ordinal':'0','text':True,'binary':None}]; "
            "assert all(len(bytes.fromhex(n['key']['hex']))==int(n['key']['length']) for n in nodes.values()); "
            "assert len({n['key']['hex'] for n in nodes.values()})==len(nodes); "
            "assert all(f['type'] in nodes for n in nodes.values() for f in n['fields']); "
            "n=next(n for n in nodes.values() if n['module'].endswith('/src/node')); "
            "assert len(n['fields'])==1 and n['fields'][0]['type']==n['id']; "
            "assert n['fields'][0]['reference'] and not n['fields'][0]['list']; "
            "assert n['fields'][0]['role']=='state'",
            (char *)NULL);
        _exit(127);
    }
    int status; pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    end_assembly();
    int removed = remove("generated/project/packages/test/fixture/v1.0.0/src/value.cmake"); assert(!removed);
    removed = remove("generated/project/packages/test/fixture/v1.0.0/src/value.extra"); assert(!removed);
    removed = rmdir("generated/project/packages/test/fixture/v1.0.0/src/other.cmake"); assert(!removed);
    removed = rmdir("generated/project/packages/test/fixture/v1.0.0/README.md"); assert(!removed);
}

/** Arbitrary byte records preserve NUL, invalid UTF-8 and all byte values exactly;
 * stream errors remain sticky through finalization rather than producing success.
 */
static void it_preserves_every_byte_in_delivery_metadata_and_reports_stream_failure(void) {
    unsigned char bytes[256]; for (size_t i = 0; i < sizeof(bytes); i++) { bytes[i] = (unsigned char)i; }
    FILE *output = tmpfile(); assert(output);
    ToolJsonWriter writer; tool_json_init(&writer, output);
    bool emitted = delivery_metadata_bytes(&writer, bytes, sizeof(bytes)); assert(emitted);
    emitted = tool_json_finish(&writer); assert(emitted);
    rewind(output); char actual[600] = {0}; size_t count = fread(actual, 1, sizeof(actual) - 1, output);
    char expected[600] = "{\"hex\":\""; size_t offset = strlen(expected);
    for (unsigned i = 0; i < 256; i++) { int n = snprintf(expected + offset, sizeof(expected) - offset, "%02x", i); assert(n == 2); offset += 2; }
    int n = snprintf(expected + offset, sizeof(expected) - offset, "\",\"length\":\"256\"}"); assert(n > 0);
    assert(count == strlen(expected) && !strcmp(actual, expected));
    int closed = fclose(output); assert(!closed);
    output = tmpfile(); assert(output); tool_json_init(&writer, output);
    emitted = delivery_metadata_bytes(&writer, NULL, 0); assert(emitted);
    emitted = tool_json_finish(&writer); assert(emitted); rewind(output);
    count = fread(actual, 1, sizeof(actual) - 1, output); actual[count] = 0;
    assert(!strcmp(actual, "{\"hex\":\"\",\"length\":\"0\"}"));
    closed = fclose(output); assert(!closed);
    output = fopen("/dev/full", "wb"); assert(output); tool_json_init(&writer, output);
    (void)delivery_metadata_bytes(&writer, bytes, sizeof(bytes));
    emitted = tool_json_finish(&writer); assert(!emitted && writer.status == TOOL_JSON_IO_ERROR);
    assert(!tool_json_raw(&writer, "{}")); (void)fclose(output);
}
/** Unsupported application packaging refuses before staging artifacts; shared
 * library assembly never acquires one-shot metadata or launcher outputs.
 */
static void it_keeps_application_admission_separate_from_stateful_shared_assembly(void) {
    prepare_assembly(); begin_assembly();
    DeliveryRequest request = {.mode = DELIVERY_SOURCE_INCLUSION, .component = "test.inclusion"};
    bool built = project_delivery_build(&assembly, (ScaffoldOptions){.directory = "generated/output", .sdk = TEST_SDK}, &request);
    assert(!built && !assembly.ready && !assembly.entry_artifact && assembly.error.code == DIAGNOSTIC_UNSUPPORTED_FEATURE);
    assert(!assembly.first && !assembly.published); end_assembly();
    begin_assembly();
    built = project_codegen_build(&assembly, (ScaffoldOptions){.directory = "generated/output", .sdk = TEST_SDK});
    assert(built && assembly.ready && !assembly.delivery && !assembly.launch_inputs && !assembly.entry_artifact);
    for (const ScaffoldArtifact *artifact = assembly.first; artifact; artifact = artifact->next) {
        assert(!strstr(artifact->path, "/launcher.c") && !strstr(artifact->path, "/compile_commands.json") && !strstr(artifact->path, "/delivery.json"));
    }
    end_assembly();
}

/**
 * Write a complete executable native fixture through the real signature/namespace emitters;
 * subsequent generated-program tests compile this authored body.
 */
static void emit_executable_native(const ScaffoldType *type) {
    FILE *body = fopen(type->body_path, "wb");
    assert(body != NULL);
    bool emitted = scaffold_include(body, type->interface_name) && scaffold_include(body, type->begin_name);
    assert(emitted);
    int written = fprintf(body, "AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const %s *parameters, %s *results, const AmaNativeServices *services, AmaStatus *status) { (void)state; (void)parameters; (void)results; (void)services; *status=ama_status_success(); }\n",
        type->names[SCAFFOLD_PARAMETERS]->internal_name, type->names[SCAFFOLD_RESULTS]->internal_name);
    assert(written >= 0);
    written = fputs("#include <assert.h>\n", body);
    assert(written != EOF);
    for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
        const AbiBinding *binding = &type->plan->type->abi.bindings[operation];
        emitted = scaffold_emit_signature(body, operation, binding->function);
        assert(emitted);
        const char *implementation = operation == ABI_DEFAULT ? " { destination->number=1; return ama_status_success(); }\n"
            : operation == ABI_DESTROY ? " { assert(state->number != 0); printf(\"%zu \", state->number); return ama_status_success(); }\n"
            : operation == ABI_LITERAL ? " { destination->number=literal.length; return ama_status_success(); }\n"
            : operation == ABI_CLONE ? " { destination->number=source->number; return ama_status_success(); }\n"
            : operation == ABI_CONDITION ? " { *result=state->number != 0; return ama_status_success(); }\n"
            : " { *result=state->number; return ama_status_success(); }\n";
        written = fputs(implementation, body);
        assert(written != EOF);
    }
    emitted = scaffold_include(body, type->end_name);
    int closed = fclose(body);
    assert(emitted && closed == 0);
}

/**
 * Assert that the component runs constant and root lifetimes from the emitted heap layout. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_runs_constant_and_root_lifetimes_from_the_emitted_heap_layout(void) {
    begin_assembly();
    bool built = build_assembly();
    assert(built);
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) { emit_executable_native(type); }
    }
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", "-std=c11", "-O2", "-pedantic", "-Wall", "-Wextra", "-Werror",
            "-I" TEST_SDK, "-Igenerated/output", "generated/output/launcher.c", "-o", "generated/program", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    int channel[2];
    int opened = pipe(channel);
    assert(opened == 0);
    process = fork();
    assert(process >= 0);
    if (process == 0) {
        close(channel[0]);
        if (dup2(channel[1], STDOUT_FILENO) < 0) { _exit(126); }
        close(channel[1]);
        execl("generated/program", "generated/program", (char *)NULL);
        _exit(127);
    }
    close(channel[1]);
    char bytes[100] = {0};
    size_t count = 0;
    ssize_t received;
    while ((received = read(channel[0], bytes + count, sizeof(bytes) - count - 1)) > 0) { count += (size_t)received; }
    close(channel[0]);
    waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    assert(strcmp(bytes, "1 1 12 ") == 0);
    end_assembly();
}

/**
 * Assert that the component unwinds every assembly emission allocation failure. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_unwinds_every_assembly_emission_allocation_failure(void) {
    begin_assembly();
    size_t start = allocation_count;
    bool built = build_assembly();
    assert(built);
    size_t count = allocation_count - start;
    end_assembly();
    for (size_t failure = 1; failure <= count; failure++) {
        begin_assembly();
        failure_at = allocation_count + failure;
        built = build_assembly();
        assert(!built && assembly.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        end_assembly();
    }
}

/**
 * Assert that the component refuses source map identity exhaustion without wrapping to zero. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_refuses_source_map_identity_exhaustion_without_wrapping_to_zero(void) {
    begin_assembly();
    frontend.plan.next_selector = SIZE_MAX;
    bool built = build_assembly();
    assert(!built && !assembly.ready && assembly.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
    end_assembly();
}

/** Corrupted required IR must fail composition before any artifact publication. */
static void it_rejects_missing_or_duplicate_required_control_sites_before_publication(void) {
    for (size_t mode = 0; mode < 3; mode++) {
        prepare_assembly();
        file("generated/project/packages/test/fixture/v1.0.0/src/callee", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\n");
        file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\nvalue\n/test/fixture/v1.0.0/src/value\nworker\n/test/fixture/v1.0.0/src/callee\nworker !\n < value\n > value\n");
        begin_assembly();
        PlannedType *owner = frontend.plan.entry;
        ControlNode *selected = NULL;
        for (ControlNode *node = owner->controls; node != NULL; node = node->allocated_next) {
            if (node->operation == (mode == 2 ? CONTROL_INSTALL_PARAMETER : CONTROL_EVALUATE_INPUT)) {
                selected = node;
                break;
            }
        }
        assert(selected != NULL);
        ControlNode *original = owner->controls;
        ControlOperation operation = selected->operation;
        ControlNode duplicate = *selected;
        if (mode == 1) {
            duplicate.allocated_next = original;
            owner->controls = &duplicate;
        } else {
            selected->operation = CONTROL_RELEASE_VALUES;
        }
        bool built = build_assembly();
        assert(!built && !assembly.ready && !assembly.publish_attempted);
        assert(assembly.error.code == DIAGNOSTIC_TYPE_MISMATCH);
        assert(strstr(assembly.error.message, mode == 2 ? "required interface operation" : "required input operation") != NULL);
        selected->operation = operation;
        owner->controls = original;
        end_assembly();
    }
}

#include "support/capabilities.c"
#include "support/translation.c"

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_preserves_every_byte_in_delivery_metadata_and_reports_stream_failure();
    it_keeps_application_admission_separate_from_stateful_shared_assembly();
    it_emits_a_finite_exact_delivery_manifest_for_recursive_types();
    it_builds_complete_traversal_adapters_after_valid_ir();
    it_compiles_the_self_contained_assembly_and_isolated_interfaces_in_either_include_order();
    it_composes_the_real_runtime_and_qualified_native_sources_in_one_c_unit();
    it_runs_constant_and_root_lifetimes_from_the_emitted_heap_layout();
    it_unwinds_every_assembly_emission_allocation_failure();
    it_refuses_source_map_identity_exhaustion_without_wrapping_to_zero();
    it_rejects_missing_or_duplicate_required_control_sites_before_publication();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
