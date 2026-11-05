#include "../common.c"
#include <sys/stat.h>

static void write_source(const char* path, const char* text) {
    FILE* file = fopen(path, "wb");
    assert(file != NULL);
    size_t written = fwrite(text, 1, strlen(text), file);
    assert(written == strlen(text));
    close_file(file);
}

static void assert_file_contains(const char* path, const char* expected) {
    FILE* file = fopen(path, "rb");
    assert(file != NULL);
    char text[4096];
    size_t length = fread(text, 1, sizeof(text) - 1, file);
    assert(!ferror(file));
    text[length] = '\0';
    close_file(file);
    assert(strstr(text, expected) != NULL);
}

static void it_generates_only_selected_stubs_and_never_overwrites_c_sources(void) {
    setup_test();
    char project[] = "generated/stubs-XXXXXX";
    char* directory = mkdtemp(project);
    assert(directory != NULL);
    char interface_path[1024], source_path[1024], draft_path[1024], ama_path[1024];
    char header_directory[1024], header_path[2048];
    snprintf(interface_path, sizeof(interface_path), "%s/main", project);
    snprintf(source_path, sizeof(source_path), "%s/main.c", project);
    snprintf(draft_path, sizeof(draft_path), "%s/draft", project);
    snprintf(ama_path, sizeof(ama_path), "%s/empty", project);
    snprintf(header_directory, sizeof(header_directory), "%s/headers/nested", project);
    snprintf(header_path, sizeof(header_path), "%s/ama_native_main.h", header_directory);
    write_source(interface_path, "@>\nanswer\n9\n@C\n");
    write_source(draft_path, "@<\nincomplete_parameter\n");
    write_source(ama_path, "@Ama\n");
    char* generate[] = {AMA_COMPILER, "--generate-c-stubs", project, header_directory, NULL};
    int created = mkdir(source_path, 0700);
    assert(created == 0);
    int status = run_process(generate, "generated/stubs.out", "generated/stubs.err");
    expect_status(status, 1, "generated/stubs.err");
    int removed = rmdir(source_path);
    assert(removed == 0);
    status = run_process(generate, "generated/stubs.out", "generated/stubs.err");
    expect_status(status, 0, "generated/stubs.err");
    assert_file_contains(source_path, "#error \"Implement this native module");
    assert_file_contains(header_path, "void ama_native_main(size_t* result_answer);");
    char unselected_source[1024];
    snprintf(unselected_source, sizeof(unselected_source), "%s/draft.c", project);
    assert(access(unselected_source, F_OK) != 0);
    snprintf(unselected_source, sizeof(unselected_source), "%s/empty.c", project);
    assert(access(unselected_source, F_OK) != 0);

    const char* implementation =
        "#include \"ama_native_main.h\"\n#include <assert.h>\n#include <stdio.h>\n\n"
        "void ama_native_main(size_t* result_answer) {\n"
        "    assert(result_answer && *result_answer == 9);\n"
        "    *result_answer = 42;\n    puts(\"stub complete\");\n}\n";
    write_source(source_path, implementation);
    status = run_process(generate, "generated/stubs.out", "generated/stubs.err");
    expect_status(status, 0, "generated/stubs.err");
    FILE* preserved = fopen(source_path, "rb");
    assert(preserved != NULL);
    assert_stream_equals(preserved, implementation);
    close_file(preserved);

    char* compile[] = {AMA_COMPILER, project, "main", "generated/program.c", NULL};
    status = run_process(compile, "generated/ama.out", "generated/ama.err");
    expect_status(status, 0, "generated/ama.err");
    const char* sources[] = {source_path};
    status = compile_generated_program(sources, 1);
    expect_status(status, 0, "generated/cc.err");
    char* execute[] = {"./generated/program", NULL};
    status = run_process(execute, "generated/stdout", "generated/stderr");
    expect_status(status, 0, "generated/stderr");
    assert_files_equal("generated/stdout", "expected.txt");
    destroy_test();
}

void run(void) {
    it_generates_only_selected_stubs_and_never_overwrites_c_sources();
}

int main(void) {
    run();
    printf("38-native-stubs: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
