#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "source/unicode/unicode.h"
#include "source/path/exact.h"

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
                assert(!valid);
            }
        }
    }
}

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
    int linked = symlink("中国", "generated/alias");
    assert(linked == 0);
    found = path_exact_child("generated", "alias", "generated/alias", &metadata);
    assert(found == PATH_UNSUPPORTED);
}

void run(void) {
    it_decodes_every_unicode_scalar_and_rejects_surrogates();
    it_rejects_overlong_out_of_range_and_malformed_sequences();
    it_matches_pinned_properties_over_the_entire_scalar_domain();
    it_preserves_unicode_spelling_and_identifier_boundaries();
    it_matches_exact_filesystem_bytes_and_refuses_symlinks();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
