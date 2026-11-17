/** @file
 * @brief Read canonical fenced examples with explicit module/body/ABI contexts, never name-based classification.
 */
#include "parser/abi/abi.h"

/** Each example has an explicitly declared surrounding context; no activation heuristic chooses a wrapper. */
typedef enum { EXAMPLE_MODULE, EXAMPLE_BODY, EXAMPLE_ABI, EXAMPLE_ABI_CONSTRUCTOR, EXAMPLE_ABI_CLONE, EXAMPLE_INPUTS, EXAMPLE_RESULTS, EXAMPLE_STATIC_BINDINGS, EXAMPLE_INVALID_BODY, EXAMPLE_INTERFACE } ExampleContext;
/** Fixed chapter/ordinal identifies one maintained text fence and its explicit parse context. */
typedef struct { const char *path; size_t ordinal; ExampleContext context; } DocumentedExample;
/** All Ama/ABI text fences; filesystem and architecture diagrams are explicitly excluded. */
static const DocumentedExample canonical_examples[] = {
    {"introduction.md", 1, EXAMPLE_MODULE},
    {"language/async-and-parallelism.md", 1, EXAMPLE_BODY},
    {"language/async-and-parallelism.md", 2, EXAMPLE_BODY},
    {"language/async-and-parallelism.md", 3, EXAMPLE_BODY},
    {"language/calls-and-results.md", 1, EXAMPLE_BODY},
    {"language/calls-and-results.md", 2, EXAMPLE_BODY},
    {"language/calls-and-results.md", 3, EXAMPLE_BODY},
    {"language/calls-and-results.md", 4, EXAMPLE_MODULE},
    {"language/calls-and-results.md", 5, EXAMPLE_BODY},
    {"language/calls-and-results.md", 6, EXAMPLE_BODY},
    {"language/calls-and-results.md", 7, EXAMPLE_BODY},
    {"language/calls-and-results.md", 8, EXAMPLE_BODY},
    {"language/calls-and-results.md", 9, EXAMPLE_BODY},
    {"language/calls-and-results.md", 10, EXAMPLE_BODY},
    {"language/calls-and-results.md", 11, EXAMPLE_BODY},
    {"language/calls-and-results.md", 12, EXAMPLE_BODY},
    {"language/calls-and-results.md", 13, EXAMPLE_MODULE},
    {"language/continuations.md", 1, EXAMPLE_MODULE},
    {"language/continuations.md", 2, EXAMPLE_MODULE},
    {"language/continuations.md", 3, EXAMPLE_BODY},
    {"language/continuations.md", 4, EXAMPLE_BODY},
    {"language/control-flow.md", 1, EXAMPLE_BODY},
    {"language/control-flow.md", 2, EXAMPLE_BODY},
    {"language/control-flow.md", 3, EXAMPLE_BODY},
    {"language/control-flow.md", 4, EXAMPLE_BODY},
    {"language/control-flow.md", 5, EXAMPLE_BODY},
    {"language/control-flow.md", 6, EXAMPLE_BODY},
    {"language/declarations.md", 1, EXAMPLE_BODY},
    {"language/declarations.md", 2, EXAMPLE_MODULE},
    {"language/declarations.md", 3, EXAMPLE_BODY},
    {"language/declarations.md", 4, EXAMPLE_BODY},
    {"language/declarations.md", 5, EXAMPLE_MODULE},
    {"language/declarations.md", 6, EXAMPLE_BODY},
    {"language/declarations.md", 7, EXAMPLE_BODY},
    {"language/declarations.md", 8, EXAMPLE_BODY},
    {"language/declarations.md", 9, EXAMPLE_MODULE},
    {"language/declarations.md", 10, EXAMPLE_BODY},
    {"language/declarations.md", 11, EXAMPLE_BODY},
    {"language/declarations.md", 12, EXAMPLE_MODULE},
    {"language/declarations.md", 13, EXAMPLE_MODULE},
    {"language/declarations.md", 14, EXAMPLE_BODY},
    {"language/declarations.md", 15, EXAMPLE_MODULE},
    {"language/declarations.md", 16, EXAMPLE_MODULE},
    {"language/lists.md", 1, EXAMPLE_BODY},
    {"language/lists.md", 2, EXAMPLE_BODY},
    {"language/lists.md", 3, EXAMPLE_BODY},
    {"language/lists.md", 4, EXAMPLE_BODY},
    {"language/modules-and-state.md", 1, EXAMPLE_BODY},
    {"language/modules-and-state.md", 2, EXAMPLE_MODULE},
    {"language/modules-and-state.md", 3, EXAMPLE_MODULE},
    {"language/source-and-names.md", 2, EXAMPLE_BODY},
    {"language/source-and-names.md", 3, EXAMPLE_MODULE},
    {"language/values-and-references.md", 1, EXAMPLE_BODY},
    {"language/values-and-references.md", 2, EXAMPLE_MODULE},
    {"language/values-and-references.md", 3, EXAMPLE_BODY},
    {"language/values-and-references.md", 4, EXAMPLE_BODY},
    {"language/values-and-references.md", 5, EXAMPLE_MODULE},
    {"grammar.md", 1, EXAMPLE_BODY},
    {"grammar.md", 2, EXAMPLE_BODY},
    {"grammar.md", 3, EXAMPLE_MODULE},
    {"grammar.md", 4, EXAMPLE_MODULE},
    {"grammar.md", 5, EXAMPLE_MODULE},
    {"grammar.md", 6, EXAMPLE_MODULE},
    {"grammar.md", 7, EXAMPLE_MODULE},
    {"grammar.md", 8, EXAMPLE_BODY},
    {"grammar.md", 9, EXAMPLE_INTERFACE},
    {"grammar.md", 10, EXAMPLE_BODY},
    {"grammar.md", 11, EXAMPLE_BODY},
    {"grammar.md", 12, EXAMPLE_MODULE},
    {"grammar.md", 13, EXAMPLE_BODY},
    {"grammar.md", 14, EXAMPLE_BODY},
    {"grammar.md", 15, EXAMPLE_INVALID_BODY},
    {"grammar.md", 16, EXAMPLE_BODY},
    {"grammar.md", 17, EXAMPLE_BODY},
    {"grammar.md", 18, EXAMPLE_BODY},
    {"grammar.md", 19, EXAMPLE_BODY},
    {"grammar.md", 20, EXAMPLE_ABI},
    {"language/templates.md", 1, EXAMPLE_INTERFACE},
    {"language/templates.md", 2, EXAMPLE_MODULE},
    {"language/templates.md", 3, EXAMPLE_BODY},
    {"language/templates.md", 4, EXAMPLE_BODY},
    {"language/templates.md", 5, EXAMPLE_MODULE},
    {"language/templates.md", 6, EXAMPLE_MODULE},
    {"language/templates.md", 7, EXAMPLE_MODULE},
    {"language/templates.md", 8, EXAMPLE_MODULE},
    {"language/templates.md", 9, EXAMPLE_MODULE},
    {"language/templates.md", 10, EXAMPLE_MODULE},
    {"language/templates.md", 11, EXAMPLE_BODY},
    {"language/templates.md", 12, EXAMPLE_INPUTS},
    {"language/templates.md", 13, EXAMPLE_INPUTS},
    {"language/templates.md", 14, EXAMPLE_INTERFACE},
    {"language/templates.md", 15, EXAMPLE_MODULE},
    {"language/templates.md", 16, EXAMPLE_BODY},
    {"language/templates.md", 17, EXAMPLE_INTERFACE},
    {"language/templates.md", 18, EXAMPLE_BODY},
    {"language/templates.md", 19, EXAMPLE_MODULE},
    {"language/templates.md", 20, EXAMPLE_MODULE},
    {"language/templates.md", 21, EXAMPLE_BODY},
    {"language/templates.md", 22, EXAMPLE_MODULE},
    {"language/templates.md", 23, EXAMPLE_BODY},
    {"language/templates.md", 24, EXAMPLE_MODULE},
    {"language/templates.md", 25, EXAMPLE_MODULE},
    {"language/static-results.md", 1, EXAMPLE_MODULE},
    {"language/static-results.md", 2, EXAMPLE_STATIC_BINDINGS},
    {"language/static-results.md", 3, EXAMPLE_BODY},
    {"language/static-results.md", 4, EXAMPLE_BODY},
    {"language/static-results.md", 5, EXAMPLE_BODY},
    {"language/static-results.md", 6, EXAMPLE_BODY},
    {"language/static-results.md", 7, EXAMPLE_MODULE},
    {"language/static-results.md", 8, EXAMPLE_BODY},
    {"language/static-results.md", 9, EXAMPLE_BODY},
    {"implementation/diagnostics.md", 1, EXAMPLE_BODY},
    {"implementation/c-library.md", 1, EXAMPLE_MODULE},
    {"implementation/c-library.md", 2, EXAMPLE_MODULE},
    {"implementation/c-library.md", 3, EXAMPLE_BODY},
    {"implementation/native-interface.md", 1, EXAMPLE_ABI_CONSTRUCTOR},
    {"implementation/native-interface.md", 2, EXAMPLE_ABI},
    {"implementation/native-interface.md", 3, EXAMPLE_ABI_CLONE},
    {"implementation/native-interface.md", 4, EXAMPLE_ABI},
};

/** Read one exact maintained Markdown fence; missing/reordered delimiters fail the fixture instead of guessing content. */
static char *example_text(DocumentedExample example) {
    char path[512];
    int written = snprintf(path, sizeof(path), "%s/%s", TEST_SPEC_ROOT, example.path);
    assert(written > 0 && (size_t)written < sizeof(path));
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    int sought = fseek(input, 0, SEEK_END);
    long length = ftell(input);
    assert(sought == 0 && length >= 0);
    rewind(input);
    char *document = malloc((size_t)length + 1);
    assert(document != NULL);
    size_t read = fread(document, 1, (size_t)length, input);
    int closed = fclose(input);
    assert(read == (size_t)length && closed == 0);
    document[length] = 0;
    size_t expected_fences = 0, actual_fences = 0;
    for (size_t index = 0; index < sizeof(canonical_examples) / sizeof(canonical_examples[0]); index++) {
        if (strcmp(canonical_examples[index].path, example.path) == 0
            && canonical_examples[index].ordinal > expected_fences) {
            expected_fences = canonical_examples[index].ordinal;
        }
    }
    for (const char *fence = document; (fence = strstr(fence, "```text\n")) != NULL; fence += strlen("```text\n")) {
        actual_fences++;
    }
    assert(actual_fences == expected_fences);
    char *start = document;
    for (size_t index = 0; index < example.ordinal; index++) {
        start = strstr(start, "```text\n");
        assert(start != NULL);
        start += strlen("```text\n");
    }
    char *end = strstr(start, "```");
    assert(end != NULL);
    const char *prefix = (example.context == EXAMPLE_BODY || example.context == EXAMPLE_INVALID_BODY) ? "@!\n"
        : example.context == EXAMPLE_INPUTS ? "@<\n"
        : example.context == EXAMPLE_RESULTS ? "@>\n"
        : example.context == EXAMPLE_STATIC_BINDINGS ? "@!\n.fixture\n"
        : example.context == EXAMPLE_ABI_CLONE ? "+ initialize\n- destroy\n" : "";
    const char *suffix = example.context == EXAMPLE_ABI_CONSTRUCTOR ? "- destroy\n"
        : (example.context == EXAMPLE_INPUTS || example.context == EXAMPLE_RESULTS || example.context == EXAMPLE_INTERFACE) ? "@!\n" : "";
    size_t prefix_length = strlen(prefix), body_length = (size_t)(end - start), suffix_length = strlen(suffix);
    size_t indentation = 0;
    if (example.context == EXAMPLE_STATIC_BINDINGS) {
        for (char *cursor = start; cursor < end; cursor++) {
            if (cursor == start || cursor[-1] == '\n') { indentation++; }
        }
    }
    char *text = malloc(prefix_length + body_length + indentation + suffix_length + 1);
    assert(text != NULL);
    memcpy(text, prefix, prefix_length);
    size_t used = prefix_length;
    for (char *cursor = start; cursor < end; cursor++) {
        if (example.context == EXAMPLE_STATIC_BINDINGS && (cursor == start || cursor[-1] == '\n')) {
            text[used++] = ' ';
        }
        text[used++] = *cursor;
    }
    memcpy(text + used, suffix, suffix_length + 1);
    free(document);
    return text;
}

/** Parse every canonical Ama/ABI text example as currently written, with no semantic lookup or edits to specification files. */
static void it_parses_every_canonical_ama_and_abi_text_example_in_its_declared_context(void) {
    for (size_t index = 0; index < sizeof(canonical_examples) / sizeof(canonical_examples[0]); index++) {
        DocumentedExample example = canonical_examples[index];
        char *text = example_text(example);
        setup_tree();
        if (example.context <= EXAMPLE_BODY || example.context >= EXAMPLE_INPUTS) {
            ParseStatus status = parse_text(text);
            if (example.context == EXAMPLE_INVALID_BODY) {
                assert(status == PARSE_INVALID && !module.valid);
            } else {
                if (status != PARSE_OK) { fprintf(stderr, "%s fence %zu: %s\n", example.path, example.ordinal, module.error.diagnostic.message); }
                assert(status == PARSE_OK && module.valid);
                check_tree(text);
            }
        } else {
            const SourceText *source = NULL;
            SourceStatus added = source_manager_add(&sources, "native.abi", (const unsigned char *)text, strlen(text), &source);
            assert(added == SOURCE_OK);
            AbiTable table;
            abi_table_init(&table, sources.allocator);
            ParseStatus status = parser_abi_parse(&table, source);
            assert(status == PARSE_OK && table.valid);
            abi_table_destroy(&table);
        }
        destroy_tree();
        free(text);
    }
}
