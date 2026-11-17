/**
 * @file
 * @brief Verify the complete Unicode scalar/XID domain, exact paths and offline regeneration.
 */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "source/unicode/unicode.h"
#include "source/path/exact.h"

/** Encode one known scalar for independent exhaustive decoder tests, without calling production encoding logic. */
static size_t encode_scalar(uint32_t scalar, unsigned char bytes[4]) {
    if (scalar < 0x80) {
        bytes[0] = (unsigned char)scalar;
        return 1;
    }
    size_t width = scalar < 0x800 ? 2 : scalar < 0x10000 ? 3 : 4;
    uint32_t remaining = scalar;
    for (size_t index = width - 1; index != 0; index--) {
        bytes[index] = (unsigned char)(0x80 | (remaining & 0x3f));
        remaining >>= 6;
    }
    bytes[0] = (unsigned char)((width == 2 ? 0xc0 : width == 3 ? 0xe0 : 0xf0) | remaining);
    return width;
}

#include "support/names.c"
#include "support/decoding.c"
#include "support/provenance.c"

/** Enumerate the complete scalar domain and truncate every multibyte encoding; surrogate code points must fail. */
static void it_decodes_every_unicode_scalar_and_rejects_surrogates(void) {
    for (uint32_t scalar = 0; scalar <= 0x10ffff; scalar++) {
        unsigned char bytes[4];
        size_t length = encode_scalar(scalar, bytes);
        uint32_t decoded = 0xffffffff;
        size_t width = 99;
        bool valid = utf8_decode(bytes, length, &decoded, &width);
        if (scalar >= 0xd800 && scalar <= 0xdfff) {
            assert(!valid && decoded == 0xffffffff && width == 99);
        } else {
            assert(valid && decoded == scalar && width == length);
            for (size_t truncated = 0; truncated < length; truncated++) {
                valid = utf8_decode(bytes, truncated, &decoded, &width);
                assert(!valid && decoded == scalar && width == length);
            }
        }
    }
}

/** Check malformed width/continuation/minimal-value partitions and unchanged decoder outputs on failure. */
static void it_rejects_overlong_out_of_range_and_malformed_sequences(void) {
    const unsigned char invalid[][4] = {
        {0x80, 0, 0, 0}, {0xc0, 0x80, 0, 0}, {0xc1, 0xbf, 0, 0},
        {0xe0, 0x9f, 0xbf, 0}, {0xf0, 0x8f, 0xbf, 0xbf},
        {0xf4, 0x90, 0x80, 0x80}, {0xf5, 0x80, 0x80, 0x80},
        {0xff, 0, 0, 0}, {0xc2, 'x', 0, 0}, {0xe2, 0x82, 'x', 0}
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        unsigned char prefixed[5] = {'a'};
        memcpy(prefixed + 1, invalid[index], 4);
        size_t error_offset = 99;
        bool valid = utf8_validate(prefixed, sizeof(prefixed), &error_offset);
        assert(!valid && error_offset == 1);
    }
}

/** Parse pinned property bytes independently into expected finite-domain bitmaps; own temporary line storage and input closure. */
static void read_expected_properties(unsigned char *start, unsigned char *continuation) {
    FILE *input = fopen("../../../../third_party/unicode/17.0.0/DerivedCoreProperties.txt", "rb");
    assert(input != NULL);
    char *line = NULL;
    size_t capacity = 0;
    while (getline(&line, &capacity, input) >= 0) {
        char *separator = strchr(line, ';');
        if (separator == NULL) {
            continue;
        }
        char *property = separator + 1;
        while (*property == ' ') {
            property++;
        }
        property[strcspn(property, " #\r\n\t")] = 0;
        unsigned char *table = strcmp(property, "XID_Start") == 0 ? start
            : strcmp(property, "XID_Continue") == 0 ? continuation : NULL;
        if (table == NULL) {
            continue;
        }
        char *end;
        unsigned long first = strtoul(line, &end, 16);
        unsigned long last = first;
        if (end[0] == '.' && end[1] == '.') {
            last = strtoul(end + 2, &end, 16);
        }
        assert(first <= last && last <= 0x10ffff);
        for (unsigned long scalar = first; scalar <= last; scalar++) {
            table[scalar] = 1;
        }
    }
    assert(!ferror(input));
    free(line);
    int closed = fclose(input);
    assert(closed == 0);
}

/** Compare every code point with the pinned independent property model, including nonmembers and out-of-range input. */
static void it_matches_pinned_properties_over_the_entire_scalar_domain(void) {
    unsigned char *expected = calloc(0x110000, 2);
    assert(expected != NULL);
    read_expected_properties(expected, expected + 0x110000);
    for (uint32_t scalar = 0; scalar <= 0x10ffff; scalar++) {
        assert(unicode_identifier_start(scalar) == (scalar == '_' || expected[scalar] != 0));
        assert(unicode_identifier_continue(scalar)
            == (scalar == '_' || expected[0x110000 + scalar] != 0));
    }
    assert(!unicode_identifier_start(0x110000));
    free(expected);
}

/** Check international XID words and exact byte distinctions without normalization or case folding. */
static void it_preserves_unicode_spelling_and_identifier_boundaries(void) {
    const char *valid[] = {"_", "中国", "магазин", "é", "e\xcc\x81", "item2"};
    for (size_t index = 0; index < sizeof(valid) / sizeof(valid[0]); index++) {
        size_t error;
        bool accepted = unicode_identifier((const unsigned char *)valid[index], strlen(valid[index]), &error);
        assert(accepted);
    }
    const char *invalid[] = {"", "2item", "\xcc\x81", "a.b", "a&", "😀"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        size_t error;
        bool accepted = unicode_identifier((const unsigned char *)invalid[index], strlen(invalid[index]), &error);
        assert(!accepted);
    }
    assert(strcmp("é", "e\xcc\x81") != 0);
    assert(strcmp("Item", "item") != 0);
}

/** Use real filesystem entries to distinguish exact spelling, absence and unsupported symlinks. */
static void it_matches_exact_filesystem_bytes_and_refuses_symlinks(void) {
    FILE *file = fopen("generated/中国", "wb");
    assert(file != NULL);
    int closed = fclose(file);
    assert(closed == 0);
    struct stat metadata;
    PathProbe found = path_exact_child("generated", "中国", "generated/中国", &metadata);
    assert(found == PATH_FOUND && S_ISREG(metadata.st_mode));
    found = path_exact_child("generated", "missing", "generated/missing", &metadata);
    assert(found == PATH_MISSING);
    int removed = unlink("generated/alias");
    assert(removed == 0 || errno == ENOENT);
    int linked = symlink("中国", "generated/alias");
    assert(linked == 0);
    found = path_exact_child("generated", "alias", "generated/alias", &metadata);
    assert(found == PATH_UNSUPPORTED);
}

/** Execute a real compiler/generator with an explicitly owned stdout descriptor.
 * Child borrows argv; parent waits through EINTR and requires normal termination.
 */
static int execute(char *const arguments[], const char *output, bool readonly) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        int descriptor = open(output, readonly ? O_RDONLY : O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (descriptor < 0 || dup2(descriptor, STDOUT_FILENO) < 0 || close(descriptor) != 0) { _exit(126); }
        execvp(arguments[0], arguments);
        _exit(127);
    }
    int status;
    pid_t result;
    do { result = waitpid(child, &status, 0); } while (result < 0 && errno == EINTR);
    assert(result == child && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/** Compare all emitted bytes without interpreting or sampling generated ranges. */
static void compare_files(const char *left, const char *right) {
    FILE *first = fopen(left, "rb"), *second = fopen(right, "rb");
    assert(first != NULL && second != NULL);
    int byte;
    do { byte = fgetc(first); assert(byte == fgetc(second)); } while (byte != EOF);
    assert(!ferror(first) && !ferror(second));
    int closed_first = fclose(first), closed_second = fclose(second);
    assert(closed_first == 0 && closed_second == 0);
}

/** Regeneration is byte-exact and its edition/range/output errors fail explicitly. */
static void it_regenerates_pinned_tables_and_rejects_invalid_input_and_output(void) {
    char *compile[] = {"cc", "-std=c11", "-D_POSIX_C_SOURCE=200809L", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "../../../../tools/unicode/generate.c", "-o", "generated/unicode-generator", NULL};
    assert(execute(compile, "generated/compile-output", false) == 0);
    char *generate[] = {"generated/unicode-generator", "../../../../third_party/unicode/17.0.0/DerivedCoreProperties.txt", NULL};
    assert(execute(generate, "generated/regenerated.inc", false) == 0);
    compare_files("generated/regenerated.inc", "../../../../src/source/unicode/xid_tables.inc");
    const char *invalid[] = {"# DerivedCoreProperties-16.0.0.txt\n", "# DerivedCoreProperties-17.0.0.txt\n0042..0041 ; XID_Start\n",
        "# DerivedCoreProperties-17.0.0.txt\n110000 ; XID_Continue\n"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        FILE *input = fopen("generated/invalid-properties", "wb");
        assert(input != NULL && fputs(invalid[index], input) != EOF);
        int closed = fclose(input);
        assert(closed == 0);
        char *reject[] = {"generated/unicode-generator", "generated/invalid-properties", NULL};
        assert(execute(reject, "generated/rejected-output", false) != 0);
    }
    FILE *small = fopen("generated/small-properties", "wb");
    assert(small != NULL && fputs("# DerivedCoreProperties-17.0.0.txt\n0041 ; XID_Start\n", small) != EOF);
    int closed = fclose(small);
    assert(closed == 0);
    char *write_failure[] = {"generated/unicode-generator", "generated/small-properties", NULL};
    assert(execute(write_failure, "generated/small-properties", true) != 0);
}

#include "support/hyphens.c"

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
void run(void) {
    it_matches_hyphen_words_and_names_over_a_finite_alphabet();
    it_checks_every_post_hyphen_scalar_and_truncated_encoding();
    it_preserves_hyphen_word_boundaries_and_exact_unicode_bytes();
    it_verifies_the_exact_pinned_unicode_edition_and_source_checksum();
    it_validates_complete_international_names_without_trimming_or_normalizing();
    it_applies_the_pinned_start_rule_to_every_word_over_the_complete_scalar_domain();
    it_validates_every_two_byte_view_and_preserves_output_failure_contracts();
    it_rejects_all_nonminimal_widths_and_out_of_range_four_byte_payloads();
    it_decodes_every_unicode_scalar_and_rejects_surrogates();
    it_rejects_overlong_out_of_range_and_malformed_sequences();
    it_matches_pinned_properties_over_the_entire_scalar_domain();
    it_preserves_unicode_spelling_and_identifier_boundaries();
    it_matches_exact_filesystem_bytes_and_refuses_symlinks();
    it_regenerates_pinned_tables_and_rejects_invalid_input_and_output();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
