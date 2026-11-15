/** @file
 * @brief Compile cooperative channel actors through actual typed native wait visitors and real host outcomes.
 * Producer-model names supply exact C bindings; generated implementation is never inspected.
 */
#include "../execution/common.c"
#include "../message-codecs/support/entries.c"
#include "../channels/support/ports.c"
#include "support/entries.c"

/** Load only maintained fixture inputs into a bounded caller-owned buffer. */
static void read_fixture(const char *path, char *buffer, size_t capacity) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t length = fread(buffer, 1, capacity - 1, input);
    assert(!ferror(input) && length < capacity - 1);
    int closed = fclose(input);
    assert(closed == 0);
    buffer[length] = 0;
}

/** Resolve one exact checked nominal module from the producer, never a filename/hash approximation. */
static const ScaffoldType *fixture_type(const NativeScaffold *scaffold, const char *path) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, path) == 0) { return type; }
    }
    assert(false);
    return NULL;
}

/** Author a real companion against generated exact roles and the maintained operation/port protocol. */
static void write_wait_companion(const NativeScaffold *scaffold, const ScaffoldType *type, bool cancel, bool sending) {
    FILE *output = fopen(type->body_path, "wb");
    assert(output != NULL);
    bool written = scaffold_include(output, type->interface_name) && scaffold_include(output, type->begin_name);
    assert(written);
    int printed = fprintf(output, "#define TEST_WAIT_PARAMETERS %s\n#define TEST_WAIT_RESULTS %s\n",
        type->names[SCAFFOLD_PARAMETERS]->internal_name, type->names[SCAFFOLD_RESULTS]->internal_name);
    assert(printed > 0);
    if (cancel) {
        const ScaffoldType *wait = fixture_type(scaffold, ".awaitsender");
        printed = fprintf(output, "#define TEST_WAIT_READ %s\n#include \"%s\"\n#undef TEST_WAIT_READ\n",
            wait->names[SCAFFOLD_READ]->internal_name, TEST_CANCEL_NATIVE);
    } else {
        const ScaffoldType *port = fixture_type(scaffold, sending ? ".sender" : ".receiver");
        printed = fprintf(output, "#define TEST_PORT_READ %s\n#define TEST_WAIT_SENDING %u\n#include \"%s\"\n"
            "#undef TEST_PORT_READ\n#undef TEST_WAIT_SENDING\n", port->names[SCAFFOLD_READ]->internal_name,
            sending ? 1u : 0u, TEST_WAIT_NATIVE);
    }
    assert(printed > 0);
    printed = fputs("#undef TEST_WAIT_PARAMETERS\n#undef TEST_WAIT_RESULTS\n", output);
    assert(printed != EOF);
    written = scaffold_include(output, type->end_name);
    int closed = fclose(output);
    assert(written && closed == 0);
}

/** Supply complete native companions before compilation and count optimized direct allocations. */
static void complete_wait_native(NativeScaffold *scaffold) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        const char *path = type->plan->type->module->source->logical_path;
        if (strcmp(path, ".awaitsender") == 0 || strcmp(path, ".awaitreceiver") == 0 || strcmp(path, ".cancelwait") == 0) {
            write_wait_companion(scaffold, type, strcmp(path, ".cancelwait") == 0, strcmp(path, ".awaitsender") == 0);
        } else if (strcmp(path, ".sender") == 0 || strcmp(path, ".receiver") == 0) {
            FILE *output = fopen(type->body_path, "ab");
            assert(output != NULL);
            int written = fputs("\n#include \"native/channels/assembly.h\"\n", output);
            int closed = fclose(output);
            assert(written != EOF && closed == 0);
        }
    }
    size_t count = scaffold->options.flag_count;
    const char **flags = source_arena_allocate(&scaffold->arena, (count + 1) * sizeof(*flags));
    assert(flags != NULL);
    if (count != 0) { memcpy(flags, scaffold->options.flags, count * sizeof(*flags)); }
    flags[count] = "-Wl,--wrap=malloc,--wrap=calloc,--wrap=free";
    scaffold->options.flags = flags;
    scaffold->options.flag_count = count + 1;
}

/** Backpressure yields to ready work; native readiness/cancellation drives only the appropriate actor. */
static void it_runs_cooperative_channel_waits_cancellation_and_isolated_actor_failure(void) {
    directory("generated/project");
    char bytes[16384];
    const char *data[] = {"message", "schema.h"};
    for (size_t index = 0; index < 2; index++) {
        char source[128], destination[128];
        int length = snprintf(source, sizeof(source), "../message-codecs/project/%s", data[index]);
        assert(length > 0 && (size_t)length < sizeof(source));
        length = snprintf(destination, sizeof(destination), "generated/project/%s", data[index]);
        assert(length > 0 && (size_t)length < sizeof(destination));
        read_fixture(source, bytes, sizeof(bytes));
        file(destination, bytes);
    }
    const char *tasks[] = {"producer", "consumer"};
    for (size_t index = 0; index < 2; index++) {
        char source[128], destination[128];
        int length = snprintf(source, sizeof(source), "project/%s", tasks[index]);
        assert(length > 0 && (size_t)length < sizeof(source));
        length = snprintf(destination, sizeof(destination), "generated/project/%s", tasks[index]);
        assert(length > 0 && (size_t)length < sizeof(destination));
        read_fixture(source, bytes, sizeof(bytes));
        file(destination, bytes);
    }
    native_fixture("encode", "@<\nflag\n..std.condition\ntext\n..text\nnumber\n..integer\n@>\nwire\n..bytes\n@C\n");
    native_fixture("decode", "@<\nwire\n..bytes\ntechnical failure\n..std.condition\n@>\nvalue\n.message\nfailure\n..failure\n@C\n");
    native_fixture("inspect", "@<\nvalue\n.message\n@C\n");
    native_fixture("role", "@>\nsending\n..std.condition\n@C\n");
    native_fixture("sender", "@<\nwire\n..bytes\n@>\nfailure\n..failure\n@C\n");
    native_fixture("receiver", "@>\nwire\n..bytes\nfailure\n..failure\n@C\n");
    native_fixture("check", "@<\nfailure\n..failure\n@C\n");
    native_fixture("checkfull", "@<\nfailure\n..failure\n@C\n");
    native_fixture("ready", "@C\n");
    native_fixture("maybefail", "@C\n");
    native_fixture("requestcancel", "@>\nrequested\n..std.condition\n@C\n");
    native_fixture("report", "@<\noutcome\n..task.outcome\nfailure\n..failure\n@C\n");
    native_fixture("awaitsender", "@<\noperation&\n..task.operation\nsender&\n.sender\nwire\n..bytes\n@C\n");
    native_fixture("awaitreceiver", "@<\noperation&\n..task.operation\nreceiver&\n.receiver\n@C\n");
    native_fixture("cancelwait", "@<\ntarget&\n.awaitsender\n@C\n");
    file("generated/project/encode.h", "#include \"schema.h\"\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; };\n");
    file("generated/project/decode.h", "#include \"schema.h\"\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; };\n");
    file("generated/project/sender.h", "#include \"native/channels/channel.h\"\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; AmaNativeChannelSender sender; };\n");
    file("generated/project/receiver.h", "#include \"native/channels/channel.h\"\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; AmaNativeChannelReceiver receiver; };\n");
    read_fixture("project/wait.h", bytes, sizeof(bytes));
    file("generated/project/awaitsender.h", bytes);
    file("generated/project/awaitreceiver.h", bytes);
    const char *resources[] = {"sender", "receiver", "awaitsender", "awaitreceiver", "cancelwait"};
    for (size_t index = 0; index < sizeof(resources) / sizeof(*resources); index++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "generated/project/%s.abi", resources[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, "+ initialize\n- destroy\n");
    }
    char source[16384], consumer[16384];
    read_fixture("project/main", source, sizeof(source));
    read_fixture("support/consumer.c", consumer, sizeof(consumer));
    host_launcher = consumer;
    native_entry_override = wait_actor_entry;
    native_hook_override = channel_port_hook;
    native_fixture_completion = complete_wait_native;
    execute_project(source,
        "full\nready\ndecoded\nproducer complete\ndecoded\n"
        "full\nready\ndecoded\n"
        "full\nready\nproducer failed\n"
        "full\nready\ncancelled\ndecoded\n"
        "full\nready\ndecoded\nproducer complete\ndecoded\n", true);
    expect_diagnostic("Channel actor failed");
    native_fixture_completion = NULL;
    native_hook_override = NULL;
    native_entry_override = NULL;
    host_launcher = NULL;
}

/** Run actual task/library/codec/native cancellation and context failure composition. */
void run(void) { it_runs_cooperative_channel_waits_cancellation_and_isolated_actor_failure(); }
/** Return only after generated release ownership checks and independent expected output pass. */
int main(void) { run(); return EXIT_SUCCESS; }
