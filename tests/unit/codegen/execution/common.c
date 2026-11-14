/**
 * @file
 * @brief Create native companions, compile generated projects and observe exact execution output/diagnostics.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_CODEGEN_EXECUTION_COMMON
#define AMA_CODEGEN_EXECUTION_COMMON
#include "../../compiler/common.c"
#include <sys/wait.h>
#include <unistd.h>
#include "codegen/codegen.h"
#include "native/scaffold/internal.h"
#include "native/toolchain/symbols.h"

static NativeScaffold assembly;
static bool noninline_entry;
static bool wrong_entry_result;
static const char *forced_header;
static const char *host_launcher;
static bool weak_toolchain_options;
static bool malformed_native_entry;
static bool malformed_condition;
static bool trace_condition_checks;
/** Test-only metadata fault injection; never enabled for valid native contracts. */
static bool corrupt_return_link;
/** Inject malformed lifecycle transport only for the explicitly selected native fixture. */
static int malformed_lifecycle = -1;
static const char *malformed_lifecycle_module;
/** Return a coherent secondary cleanup error for the selected worker fixture. */
static bool fail_native_destroy;
/** Return Busy as an accepted destructor failure, distinct from admission refusal. */
static bool busy_native_destroy;
/** Test-only synchronous host barrier for genuinely overlapping generated contexts. */
static bool native_execution_gate;

/** Optional test-only entry fragment writer. A true result owns the complete
 * entry effects before common status completion; model and stream are borrowed
 * only during emission. Production compilation still checks the resulting ABI.
 */
typedef bool (*TestNativeEntryEmitter)(FILE *, const ScaffoldType *);
/** Optional test-only payload-hook body selector; return a static C fragment or
 * null for the ordinary address-sensitive fixture. Never retain model pointers.
 */
typedef const char *(*TestNativeHookEmitter)(const ScaffoldType *, AbiOperation);
static TestNativeEntryEmitter native_entry_override;
static TestNativeHookEmitter native_hook_override;

/**
 * Create the explicit native interface/body/ABI companion set used by this leaf. The caller
 * supplies its known contract; no native capability is inferred from a C name.
 */
static void native_fixture(const char *name, const char *interface) {
    char path[160];
    int length = snprintf(path, sizeof(path), "generated/project/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    file(path, interface);
    length = snprintf(path, sizeof(path), "generated/project/%s.h", name);
    assert(length > 0);
    file(path, "struct AMA_NATIVE(State) { size_t number; size_t calls; const void *address; };\n");
    length = snprintf(path, sizeof(path), "generated/project/%s.c", name);
    assert(length > 0);
    file(path, "/* Test writes the typed implementation after declaration generation. */\n");
    length = snprintf(path, sizeof(path), "generated/project/%s.abi", name);
    assert(length > 0);
    file(path, "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n");
}

/**
 * Emit the scenario-specific native body using the generated typed interface and exact naming
 * context; keep body effects explicit for execution assertions.
 */
static void executable_native(const ScaffoldType *type) {
    FILE *body = fopen(type->body_path, "wb");
    assert(body != NULL);
    bool emitted = scaffold_include(body, type->interface_name) && scaffold_include(body, type->begin_name);
    assert(emitted);
    int written = fputs("#include <assert.h>\n#include <stdio.h>\n", body);
    assert(written != EOF);
    if (strcmp(type->plan->type->module->source->logical_path,".recursive")==0) {
        written = fputs("static __attribute__((noinline)) size_t AMA_NATIVE(sum)(size_t number) { return number==0 ? 0 : number+AMA_NATIVE(sum)(number-1); }\n",body);
        assert(written != EOF);
    }
    written = fprintf(body, "%s %s AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const %s *parameters, %s *results, const AmaNativeServices *services, AmaStatus *status) {\n"
        "assert(state->address==state); (void)parameters; (void)results; (void)services;\n", noninline_entry ? "__attribute__((noinline))" : "AMA_NATIVE_ENTRY", wrong_entry_result ? "AmaStatus" : "void", type->names[SCAFFOLD_PARAMETERS]->internal_name,
        type->names[SCAFFOLD_RESULTS]->internal_name);
    assert(written >= 0);
    if (native_entry_override == NULL || !native_entry_override(body, type)) {
        if (strcmp(type->plan->type->module->source->logical_path,".showindex")==0) {
            written = fputs("printf(\"value=%zu\\n\",parameters->AMA_INPUT(0)->payload->value);\n",body);
            assert(written != EOF);
        } else if (strcmp(type->plan->type->module->source->logical_path,".recursive")==0) {
            written = fputs("size_t total=AMA_NATIVE(sum)(parameters->AMA_INPUT(0)->payload->number); results->AMA_OUTPUT(0)->payload->number=total; printf(\"sum=%zu\\n\",total);\n",body);
            assert(written != EOF);
        } else if (strcmp(type->plan->type->module->source->logical_path,".decrement")==0) {
            written = fputs("size_t number=parameters->AMA_INPUT(0)->payload->number; results->AMA_OUTPUT(0)->payload->number=number==0 ? 0 : number-1; printf(\"count=%zu\\n\",number);\n",body);
            assert(written != EOF);
        } else if (type->plan->type->counts[SYMBOL_PARAMETER] == 2) {
            written = fputs("printf(\"ordered=%zu,%zu,%zu\\n\",parameters->AMA_INPUT(0)->payload->number,parameters->AMA_INPUT(1)->payload->number,results->AMA_OUTPUT(0)->payload->number);\n",body);
            assert(written != EOF);
        } else if (type->plan->type->counts[SYMBOL_PARAMETER] != 0 && type->plan->type->counts[SYMBOL_RESULT] == 0) {
            written = fputs("printf(\"status=%u\\n\",(unsigned)parameters->AMA_INPUT(0)->payload->value.code);\n", body);
            assert(written != EOF);
        } else if (type->plan->type->counts[SYMBOL_PARAMETER] != 0) {
            written = fputs("if(parameters->AMA_INPUT(0)->payload->number==500) { puts(\"native-entered\"); *status=ama_status_make(AMA_NATIVE_FAILURE,\"Body failed\",11); return; }\nstate->calls++; results->AMA_OUTPUT(0)->payload->number=parameters->AMA_INPUT(0)->payload->number+state->calls;\n"
                "printf(\"%zu:%zu:%u\\n\",parameters->AMA_INPUT(0)->payload->number,state->calls,(unsigned)services->caller_refusal.code);\n", body);
            assert(written != EOF);
        }
    }
    if (corrupt_return_link && strcmp(type->plan->type->module->source->logical_path, ".worker") == 0) {
        written = fputs("fault_return_link();\n", body);
        assert(written != EOF);
    }
    if (native_execution_gate && strcmp(type->plan->type->module->source->logical_path, ".gate") == 0) {
        written = fputs("test_execution_gate();\n", body);
        assert(written != EOF);
    }
    written = fputs(malformed_native_entry ? "*status=ama_status_make(AMA_OK,NULL,1); }\n"
        : "*status=ama_status_success(); }\n", body);
    assert(written != EOF);
    for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
        const AbiBinding *binding = &type->plan->type->abi.bindings[operation];
        emitted = scaffold_emit_signature(body, operation, binding->function);
        assert(emitted);
        const char *implementation = operation == ABI_DEFAULT ? " { *destination=(AMA_NATIVE(State)){.address=destination}; return ama_status_success(); }\n"
            : operation == ABI_DESTROY ? " { assert(state->address==state); return ama_status_success(); }\n"
            : operation == ABI_LITERAL ? " { *destination=(AMA_NATIVE(State)){.address=destination}; for(size_t index=0;index<literal.length;index++) { if(literal.data[index]<'0'||literal.data[index]>'9') return ama_status_make(AMA_INVALID_LITERAL,\"Not a decimal value\",19); destination->number=destination->number*10+literal.data[index]-'0'; } return ama_status_success(); }\n"
            : operation == ABI_CLONE ? " { *destination=*source; destination->address=destination; return ama_status_success(); }\n"
            : operation == ABI_CONDITION ? " { *result=state->number!=0; return ama_status_success(); }\n"
            : " { if(state->number==9999) return ama_status_make(AMA_NATIVE_FAILURE,\"Index decoder failed\",20); if(state->number>=1000 && state->number<9999) { printf(\"index=%zu\\n\",state->number); *result=state->number-1000; } else { *result=state->number; } return ama_status_success(); }\n";
        if (native_hook_override != NULL) {
            const char *selected = native_hook_override(type, operation);
            if (selected != NULL) { implementation = selected; }
        }
        if (operation == ABI_CONDITION && trace_condition_checks) {
            implementation = " { static size_t checks; checks++; printf(\"truth=%zu\\n\",checks); *result=state->number!=0 && checks==1; return ama_status_success(); }\n";
        }
        if (operation == ABI_CONDITION && malformed_condition) {
            implementation = " { (void)state; *result=true; return ama_status_make(AMA_OK,NULL,1); }\n";
        }
        if ((int)operation == malformed_lifecycle && malformed_lifecycle_module != NULL
            && strcmp(type->plan->type->module->source->logical_path, malformed_lifecycle_module) == 0) {
            implementation = operation == ABI_DEFAULT
                ? " { *destination=(AMA_NATIVE(State)){.address=destination}; return ama_status_make(AMA_OK,NULL,1); }\n"
                : operation == ABI_DESTROY
                ? " { assert(state->address==state); return ama_status_make(AMA_OK,NULL,1); }\n"
                : operation == ABI_LITERAL
                ? " { (void)literal; *destination=(AMA_NATIVE(State)){.address=destination}; return ama_status_make(AMA_OK,NULL,1); }\n"
                : " { *destination=*source; destination->address=destination; return ama_status_make(AMA_OK,NULL,1); }\n";
        }
        if (fail_native_destroy && operation == ABI_DESTROY
            && strcmp(type->plan->type->module->source->logical_path, ".worker") == 0) {
            implementation = busy_native_destroy
                ? " { assert(state->address==state); return ama_status_make(AMA_BUSY,\"cleanup\",7); }\n"
                : " { assert(state->address==state); return ama_status_make(AMA_NATIVE_FAILURE,\"cleanup\",7); }\n";
        }
        written = fputs(implementation, body);
        assert(written != EOF);
    }
    emitted = scaffold_include(body, type->end_name);
    int closed = fclose(body);
    assert(emitted && closed == 0);
}

/**
 * Write this scenario's explicit source/native fixtures in test-owned storage before loading or
 * compiling them; the caller controls their lifetime and expected result.
 */
static void prepare_project(const char *main_source) {
    directory("generated/project");
    file("generated/project/main", main_source);
    native_fixture("value", "@C\n");
    native_fixture("worker", "@<\ninput\n.value\n |7\n@>\noutput\n.value\n input\nother\n.value\n |99\n@C\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT}, {TEST_STDLIB, NULL, 0, MODULE_ROOT_PROJECT}};
    bool checked = compiler_frontend_check(&frontend, roots, 2, ".main");
    if (!checked) { fprintf(stderr, "Frontend failed: %s\n", frontend.error.message); }
    assert(checked);
    native_scaffold_init(&assembly, &frontend, sources.allocator);
    ScaffoldOptions options = {.directory = "generated/output", .sdk = TEST_SDK};
    static const char *flags[9];
    size_t flag_count = 0;
#ifdef TEST_SANITIZERS
    flags[flag_count++] = "-fsanitize=address,undefined";
    flags[flag_count++] = "-fno-omit-frame-pointer";
    flags[flag_count++] = "-fno-pie";
    flags[flag_count++] = "-no-pie";
#endif
    if (forced_header != NULL) { flags[flag_count++] = "-include"; flags[flag_count++] = forced_header; }
    if (weak_toolchain_options) {
        flags[flag_count++] = "-Wno-error";
        flags[flag_count++] = "-flto";
        flags[flag_count++] = "-std=c99";
    }
    options.flags = flags;
    options.flag_count = flag_count;
    bool built = project_codegen_build(&assembly, options);
    assert(built);
    if (host_launcher != NULL) {
        for (ScaffoldArtifact *artifact = assembly.first; artifact != NULL; artifact = artifact->next) {
            if (strstr(artifact->path, "/launcher.c") == NULL) { continue; }
            SourceStatus status = source_manager_add(&assembly.texts, artifact->path,
                (const unsigned char *)host_launcher, strlen(host_launcher), &artifact->content);
            assert(status == SOURCE_OK);
        }
    }
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && strncmp(source_path(type->plan->type->module->source->text),
                "generated/project/", sizeof("generated/project/") - 1) == 0) { executable_native(type); }
    }
}

/**
 * Load/check the generated fixture, emit/compile/verify/publish it, execute the resulting
 * process and compare exact stdout and expected success. Destroy scaffold before source owners.
 */
static void execute_project(const char *main_source, const char *expected, bool success) {
    prepare_project(main_source);
    bool compiled = project_codegen_compile(&assembly, stderr);
    assert(compiled);
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    pid_t process;
    int status;
    pid_t waited;
    int channel[2];
    int opened = pipe(channel);
    assert(opened == 0);
    process = fork();
    assert(process >= 0);
    if (process == 0) {
        close(channel[0]);
        if (dup2(channel[1], STDOUT_FILENO) < 0) { _exit(126); }
        close(channel[1]);
        if (freopen("generated/errors.txt", "wb", stderr) == NULL) { _exit(126); }
        execl("generated/output/program", "generated/output/program", (char *)NULL);
        _exit(127);
    }
    close(channel[1]);
    char bytes[4096] = {0};
    size_t count = 0;
    ssize_t received;
    while ((received = read(channel[0], bytes + count, sizeof(bytes) - count - 1)) > 0) { count += (size_t)received; }
    close(channel[0]);
    waited = waitpid(process, &status, 0);
    if (waited == process && !WIFEXITED(status)) {
        FILE *diagnostics = fopen("generated/errors.txt", "rb");
        if (diagnostics != NULL) {
            char chunk[1024];
            size_t length;
            while ((length = fread(chunk, 1, sizeof(chunk), diagnostics)) != 0) { (void)fwrite(chunk, 1, length, stderr); }
            (void)fclose(diagnostics);
        }
    }
    assert(waited == process && WIFEXITED(status));
    assert((WEXITSTATUS(status) == 0) == success);
    if (strcmp(bytes, expected) != 0) { fprintf(stderr, "Unexpected output: %s\n", bytes); }
    assert(strcmp(bytes, expected) == 0);
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}

/**
 * Observe the real structured or captured runtime diagnostic using this fixture contract.
 * Require the expected bytes/text and complete IO rather than relying on exit failure alone.
 */
static void expect_diagnostic(const char *part) {
    FILE *errors = fopen("generated/errors.txt", "rb");
    assert(errors != NULL);
    char bytes[8192] = {0};
    size_t count = fread(bytes, 1, sizeof(bytes)-1, errors);
    assert(!ferror(errors) && count < sizeof(bytes)-1);
    int closed = fclose(errors);
    assert(closed == 0);
    if (strstr(bytes, part) == NULL) { fprintf(stderr,"Missing diagnostic %s in %s\n",part,bytes); }
    assert(strstr(bytes, part) != NULL);
}

#endif
