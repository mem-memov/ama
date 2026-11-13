/**
 * @file
 * @brief Verify allocation-denied fallback rendering and output preflight failures.
 */

/** Fixture allocation authority counts every request and can deny all requests after source acquisition. */
typedef struct {
    size_t requests;
    size_t live;
    bool denied;
} RenderAllocationState;

/** Count real allocations independently of stdio; denial must be irrelevant to allocation-free rendering. */
static void *render_allocate(void *context, size_t size) {
    RenderAllocationState *state = context;
    state->requests++;
    if (state->denied) {
        return NULL;
    }
    void *allocation = malloc(size);
    if (allocation != NULL) {
        state->live++;
    }
    return allocation;
}

/** Release fixture-owned source storage and check exact allocation balance. */
static void render_release(void *context, void *allocation) {
    RenderAllocationState *state = context;
    if (allocation != NULL) {
        assert(state->live != 0);
        state->live--;
        free(allocation);
    }
}

/** A static out-of-memory diagnostic and existing source context render after all compiler allocation is disabled. */
static void it_renders_fallback_and_source_context_with_all_compiler_allocations_denied(void) {
    RenderAllocationState state = {0};
    SourceAllocator allocator = {&state, render_allocate, render_release};
    SourceManager manager;
    source_manager_init(&manager, allocator);
    const SourceText *text = NULL;
    const unsigned char bytes[] = "商品\r\n";
    SourceStatus added = source_manager_add(&manager, "original 名称", bytes, sizeof(bytes) - 1, &text);
    assert(added == SOURCE_OK);
    FILE *stream = tmpfile();
    assert(stream != NULL);
    Diagnostic diagnostic = {.code = DIAGNOSTIC_OUT_OF_MEMORY,
        .message = "Allocation failed"};
    size_t requests = state.requests;
    state.denied = true;
    DiagnosticRenderStatus rendered = diagnostic_render(stream, &diagnostic);
    assert(rendered == DIAGNOSTIC_RENDER_OK);
    diagnostic.primary.span = (SourceSpan){text, 0, 6};
    rendered = diagnostic_render(stream, &diagnostic);
    assert(rendered == DIAGNOSTIC_RENDER_OK);
    SourceStatus mapped = source_write_line_directive(stream, text, source_length(text));
    assert(mapped == SOURCE_OK && state.requests == requests);
    char output[1024];
    read_output(stream, output, sizeof(output));
    assert(strstr(output, "<input>: error[out-of-memory]: Allocation failed\n") != NULL);
    assert(strstr(output, "original 名称:1:1: error[out-of-memory]") != NULL);
    assert(strstr(output, "    1 | 商品\n") != NULL);
    assert(strstr(output, "#line 2 \"original 名称\"\n") != NULL);
    int closed = fclose(stream);
    assert(closed == 0);
    source_manager_destroy(&manager);
    assert(state.live == 0);
}

/** Mapping rejects invalid source/path metadata before writing and propagates immediate unbuffered IO errors. */
static void it_rejects_invalid_mapping_before_output_and_reports_immediate_io_failure(void) {
    setup_test();
    const SourceText *invalid_path = NULL;
    SourceStatus status = source_manager_add(&sources, "bad\xff", NULL, 0, &invalid_path);
    assert(status == SOURCE_OK);
    FILE *stream = tmpfile();
    assert(stream != NULL);
    status = source_write_line_directive(stream, invalid_path, 0);
    assert(status == SOURCE_INVALID_ARGUMENT);
    assert(ftell(stream) == 0);
    const SourceText *unsupported_path = NULL;
    status = source_manager_add(&sources, "root\rname/module", NULL, 0, &unsupported_path);
    assert(status == SOURCE_OK);
    status = source_write_line_directive(stream, unsupported_path, 0);
    assert(status == SOURCE_INVALID_ARGUMENT);
    assert(ftell(stream) == 0);
    status = source_write_line_directive(stream, source, source_length(source) + 1);
    assert(status == SOURCE_INVALID_ARGUMENT);
    assert(ftell(stream) == 0);
    int closed = fclose(stream);
    assert(closed == 0);
    stream = fopen("/dev/full", "wb");
    assert(stream != NULL);
    int configured = setvbuf(stream, NULL, _IONBF, 0);
    assert(configured == 0);
    status = source_write_line_directive(stream, source, 0);
    assert(status == SOURCE_IO_ERROR);
    (void)fclose(stream);
    destroy_test();
}
