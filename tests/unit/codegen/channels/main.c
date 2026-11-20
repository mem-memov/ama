/** @file
 * @brief Compile isolated Ama producer/receiver contexts over real bounded native endpoints and typed decoding.
 * Host-owned native rights move into declared noncopyable modules, never hidden Ama references.
 */
#include "../execution/common.c"
#include "../message-codecs/support/entries.c"
#include "support/ports.c"

/** Read maintained fixture text with complete I/O and checked fixed capacity. */
static void read_fixture(const char *path, char *buffer, size_t capacity) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t length = fread(buffer, 1, capacity - 1, input);
    assert(!ferror(input) && length < capacity - 1);
    int closed = fclose(input);
    assert(closed == 0);
    buffer[length] = 0;
}

/** Include the creator-side SDK once and wrap actual generated/native allocations. */
static void complete_channel_native(NativeScaffold *scaffold) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        const char *path = type->plan->type->module->source->logical_path;
        if (strcmp(path, "/test/fixture/v1.0.0/src/sender") != 0 && strcmp(path, "/test/fixture/v1.0.0/src/receiver") != 0) { continue; }
        FILE *output = fopen(type->body_path, "ab");
        assert(output != NULL);
        int written = fputs("\n#include \"native/channels/assembly.h\"\n", output);
        int closed = fclose(output);
        assert(written != EOF && closed == 0);
    }
    size_t count = scaffold->options.flag_count;
    const char **flags = source_arena_allocate(&scaffold->arena, (count + 1) * sizeof(*flags));
    assert(flags != NULL);
    if (count != 0) { memcpy(flags, scaffold->options.flags, count * sizeof(*flags)); }
    flags[count] = "-Wl,--wrap=malloc,--wrap=calloc,--wrap=free";
    scaffold->options.flags = flags;
    scaffold->options.flag_count = count + 1;
}

/** Full is ordinary data; a native queue outlives sender teardown without sharing Ama values. */
static void it_sends_independent_typed_values_between_isolated_ama_contexts_and_drains_after_sender_disposal(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    char bytes[16384];
    read_fixture("../message-codecs/project/message", bytes, sizeof(bytes));
    file("generated/project/packages/test/fixture/v1.0.0/src/message", bytes);
    read_fixture("../message-codecs/project/schema.h", bytes, sizeof(bytes));
    file("generated/project/packages/test/fixture/v1.0.0/src/schema.h", bytes);
    native_fixture("encode", "@<\nflag\n//std/condition\ntext\n//text\nnumber\n//integer\n@>\nwire\n//bytes\n@C\n");
    native_fixture("decode", "@<\nwire\n//bytes\ntechnical failure\n//std/condition\n@>\nvalue\n/test/fixture/v1.0.0/src/message\nfailure\n//failure\n@C\n");
    native_fixture("inspect", "@<\nvalue\n/test/fixture/v1.0.0/src/message\n@C\n");
    native_fixture("role", "@>\nsending\n//std/condition\n@C\n");
    native_fixture("sender", "@<\nwire\n//bytes\n@>\nfailure\n//failure\n@C\n");
    native_fixture("receiver", "@>\nwire\n//bytes\nfailure\n//failure\n@C\n");
    native_fixture("check", "@<\nfailure\n//failure\n@C\n");
    native_fixture("checkfull", "@<\nfailure\n//failure\n@C\n");
    const char *codecs[] = {"encode", "decode"};
    for (size_t index = 0; index < 2; index++) {
        char path[128];
        int written = snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.h", codecs[index]);
        assert(written > 0 && (size_t)written < sizeof(path));
        file(path, "#include \"schema.h\"\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; };\n");
    }
    file("generated/project/packages/test/fixture/v1.0.0/src/sender.h", "#include \"native/channels/channel.h\"\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; AmaNativeChannelSender sender; };\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/receiver.h", "#include \"native/channels/channel.h\"\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; AmaNativeChannelReceiver receiver; };\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/sender.abi", "+ initialize\n- destroy\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/receiver.abi", "+ initialize\n- destroy\n");
    char source[4096], consumer[16384];
    read_fixture("project/main", source, sizeof(source));
    read_fixture("support/consumer.c", consumer, sizeof(consumer));
    host_launcher = consumer;
    native_entry_override = channel_port_entry;
    native_hook_override = channel_port_hook;
    native_fixture_completion = complete_channel_native;
    execute_project(source, "full\ndecoded\ndecoded\n", true);
    native_fixture_completion = NULL;
    native_hook_override = NULL;
    native_entry_override = NULL;
    host_launcher = NULL;
}

/** Run actual context, codec, endpoint lifecycle and bounded queue composition. */
void run(void) {
    /* This successful composition has no expected diagnostic substring. */
    (void)expect_diagnostic;
    it_sends_independent_typed_values_between_isolated_ama_contexts_and_drains_after_sender_disposal();
}
/** Return only after every generated ownership/accounting assertion completes. */
int main(void) { run(); return EXIT_SUCCESS; }
