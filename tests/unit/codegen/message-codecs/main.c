/** @file
 * @brief Verify typed Ama publication, ordinary malformed-data failure and exact technical prefix cleanup.
 * Real native companions use the schema codec; generated implementation is never inspected.
 */
#include "../execution/common.c"
#include "support/entries.c"

/** Read maintained source bytes with checked capacity and explicit ownership. */
static void read_fixture(const char *path, char *buffer, size_t capacity) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t length = fread(buffer, 1, capacity - 1, input);
    assert(!ferror(input) && length < capacity - 1);
    int closed = fclose(input);
    assert(closed == 0);
    buffer[length] = 0;
}

/** Assemble the native codec once and wrap direct project allocations in the actual consumer. */
static void complete_message_native(NativeScaffold *scaffold) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        const char *path = type->plan->type->module->source->logical_path;
        if (strcmp(path, ".encode") != 0 && strcmp(path, ".decode") != 0) { continue; }
        FILE *output = fopen(type->body_path, "ab");
        assert(output != NULL);
        int written = fputs("\n#include \"native/messages/assembly.h\"\n", output);
        int closed = fclose(output);
        assert(written != EOF && closed == 0);
    }
    size_t count = scaffold->options.flag_count;
    const char **flags = source_arena_allocate(&scaffold->arena, (count + 1) * sizeof(*flags));
    assert(flags != NULL);
    if (count != 0) { memcpy(flags, scaffold->options.flags, count * sizeof(*flags)); }
    flags[count] = "-Wl,--wrap=malloc,--wrap=free";
    scaffold->options.flags = flags;
    scaffold->options.flag_count = count + 1;
}

/** Bind a fixed application schema through ordinary owned Ama fields and native ABI contracts. */
static void execute_message(const char *scenario, const char *expected, bool technical) {
    directory("generated/project");
    char bytes[16384];
    read_fixture("project/message", bytes, sizeof(bytes));
    file("generated/project/message", bytes);
    read_fixture("project/schema.h", bytes, sizeof(bytes));
    file("generated/project/schema.h", bytes);
    native_fixture("encode", "@<\nflag\n..std.condition\ntext\n..text\nnumber\n..integer\n@>\nwire\n..bytes\n@C\n");
    native_fixture("decode", "@<\nwire\n..bytes\ntechnical failure\n..std.condition\n@>\nvalue\n.message\nfailure\n..failure\n@C\n");
    native_fixture("inspect", "@<\nvalue\n.message\n@C\n");
    native_fixture("inspectfailure", "@<\nfailure\n..failure\n@C\n");
    const char *names[] = {"encode", "decode"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        char path[128];
        int written = snprintf(path, sizeof(path), "generated/project/%s.h", names[index]);
        assert(written > 0 && (size_t)written < sizeof(path));
        file(path, "#include \"schema.h\"\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; };\n");
    }
    char source[4096], consumer[8192] = {0}, path[128];
    int written = snprintf(path, sizeof(path), "project/%s", scenario);
    assert(written > 0 && (size_t)written < sizeof(path));
    read_fixture(path, source, sizeof(source));
    if (technical) { strcpy(consumer, "#define TEST_TECHNICAL_FAILURE\n"); }
    size_t used = strlen(consumer);
    read_fixture("support/consumer.c", consumer + used, sizeof(consumer) - used);
    host_launcher = consumer;
    native_entry_override = message_entry;
    native_fixture_completion = complete_message_native;
    execute_project(source, expected, true);
    if (technical) { expect_diagnostic("Typed receive failed"); }
    native_fixture_completion = NULL;
    native_entry_override = NULL;
    host_launcher = NULL;
}

/** Typed fields survive release of both native temporary decoding and the Ama envelope. */
static void it_constructs_independent_typed_ama_fields_before_publishing_a_message(void) {
    execute_message("roundtrip", "decoded\n", false);
}
/** Invalid wire data becomes one ordinary failure module with its owned description. */
static void it_reports_malformed_messages_as_ordinary_failure_values(void) {
    execute_message("reject", "rejected\n", false);
}
/** An accepted error after constructing a child cleans all generated/native allocations. */
static void it_unwinds_partial_typed_receive_construction_on_technical_failure(void) {
    execute_message("technical", "", true);
}

/** Run actual schema/native/runtime publication and both failure provenance paths. */
void run(void) {
    it_constructs_independent_typed_ama_fields_before_publishing_a_message();
    it_reports_malformed_messages_as_ordinary_failure_values();
    it_unwinds_partial_typed_receive_construction_on_technical_failure();
}
/** Return only after all compiled ownership checks succeed. */
int main(void) { run(); return EXIT_SUCCESS; }
