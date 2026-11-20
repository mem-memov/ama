/**
 * @file
 * @brief Verify exact version grammar, bounded byte views and host-range independence.
 *
 * Construct expected canonical versions independently and compare a complete
 * finite alphabet domain; intentional malformed examples exercise boundaries
 * that are outside that finite domain. No catalog or parser state is required.
 * @see [T231 contract](../../../../docs/implementation/versioned-source.md)
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "source/path/version.h"
#include "source/path/logical.h"
#include "source/path/syntax.h"
#include "source/unicode/unicode.h"

/** Exercise canonical decimal/variant examples with Unicode and isolated hyphens. */
static void it_accepts_exact_canonical_versions(void) {
    const char *cases[] = {
        "v0.0.0", "v2.4.14", "v12.34.56-speed", "v1.2.3-memory-saver",
        "v1.2.3-_", "v1.2.3-a-2", "v1.2.3-\xe9\x80\x9f\xe5\xba\xa6",
        "v1.2.3-a-\xcc\x81"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        bool valid = module_version_component((const unsigned char *)cases[index], strlen(cases[index]));
        assert(valid);
    }
}

/** Reject incomplete, padded, noncanonical and malformed variant spellings. */
static void it_rejects_noncanonical_or_incomplete_versions(void) {
    const char *cases[] = {
        "", "v", "v0", "v0.0", "v0.0.", "v.0.0", "v0..0", "v0.0.0.0",
        "V1.2.3", "v01.2.3", "v1.02.3", "v1.2.03", "v00.0.0", "v1.2.3-",
        "v1.2.3-2", "v1.2.3--a", "v1.2.3-a-", "v1.2.3-a--b", "v1.2.3-a b",
        " v1.2.3", "v1.2.3 ", "v1.2.3+build", "v1.2.3/a", "v1.2.3-a\t",
        "v1.2.3-\xff", "v1.2.3-\xe9\x80", "v1.2.3-\xc0\x80", "v1.2.3-\xcc\x81"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        bool valid = module_version_component((const unsigned char *)cases[index], strlen(cases[index]));
        assert(!valid);
    }
}

/** Respect exact lengths, including unterminated input and embedded zero bytes. */
static void it_reads_only_the_supplied_byte_view(void) {
    const unsigned char bytes[] = {'v', '0', '.', '0', '.', '0', 0, '-','a'};
    bool valid = module_version_component(bytes, 6);
    assert(valid);
    for (size_t length = 0; length < sizeof(bytes); length++) {
        valid = module_version_component(bytes, length);
        assert(valid == (length == 6));
    }
    valid = module_version_component(bytes, sizeof(bytes));
    assert(!valid);
    valid = module_version_component(NULL, 0);
    assert(!valid);
    valid = module_version_component(NULL, 100);
    assert(!valid);
    const unsigned char exact[] = {'v', '1', '.', '2', '.', '3'};
    valid = module_version_component(exact, sizeof(exact));
    assert(valid);
}

/** Canonical numeric spellings have no machine-integer width restriction. */
static void it_accepts_decimal_fields_larger_than_host_integers(void) {
    unsigned char bytes[12300];
    size_t cursor = 0;
    bytes[cursor++] = 'v';
    for (size_t field = 0; field < 3; field++) {
        memset(bytes + cursor, '9', 4096);
        cursor += 4096;
        if (field != 2) {
            bytes[cursor++] = '.';
        }
    }
    bool valid = module_version_component(bytes, cursor);
    assert(valid);
    bytes[1] = '0';
    valid = module_version_component(bytes, cursor);
    assert(!valid);
}

/** Construct the complete finite grammar language for length <= 7 over v01.-.
 * At these lengths no variant fits; a decimal field has at most two digits,
 * whose possible canonical spellings are exactly 0, 1, 10 and 11. Membership
 * below is a set comparison, not a second version-recognition algorithm.
 */
static bool expected_finite_member(const unsigned char *bytes, size_t length) {
    const char *numbers[] = {"0", "1", "10", "11"};
    for (size_t major = 0; major < 4; major++) {
        for (size_t minor = 0; minor < 4; minor++) {
            for (size_t patch = 0; patch < 4; patch++) {
                char expected[12];
                int count = snprintf(expected, sizeof(expected), "v%s.%s.%s",
                    numbers[major], numbers[minor], numbers[patch]);
                assert(count > 0 && (size_t)count < sizeof(expected));
                if ((size_t)count == length && memcmp(expected, bytes, length) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

/** Exhaust every string of lengths 0..7 over the declared five-character domain. */
static void it_matches_the_complete_bounded_grammar_language(void) {
    const unsigned char alphabet[] = {'v', '0', '1', '.', '-'};
    size_t possibilities = 1;
    for (size_t length = 0; length <= 7; length++) {
        for (size_t ordinal = 0; ordinal < possibilities; ordinal++) {
            unsigned char bytes[7];
            size_t remainder = ordinal;
            for (size_t index = 0; index < length; index++) {
                bytes[index] = alphabet[remainder % sizeof(alphabet)];
                remainder /= sizeof(alphabet);
            }
            bool expected = expected_finite_member(bytes, length);
            bool actual = module_version_component(bytes, length);
            assert(actual == expected);
        }
        possibilities *= sizeof(alphabet);
    }
}

/** Admit complete endpoints with hierarchical packages and exact root/area identity. */
static void it_accepts_canonical_ordinary_and_standard_identities(void) {
    const char *cases[] = {
        "/acme/shop/v2.4.14/src/item", "/acme/a/b/v0.0.0/tests/item/child",
        "/vendor name/package name/v1.2.3-speed/src/item name/child-name",
        "/供应/容器/v1.0.0-速度/src/元素",
        "//integer", "//item state/validate", "//src/tests"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        bool valid = module_path_canonical((const unsigned char *)cases[index], strlen(cases[index]));
        assert(valid);
    }
}

/** Reject every missing structural region, noncanonical component and old root form. */
static void it_rejects_incomplete_or_unnormalized_identities(void) {
    const char *cases[] = {
        "", "/", "//", "///integer", ".main", "..integer", "./item", "../item",
        "~items/item", "/acme", "/acme/shop", "/acme/v1.2.3/src/item",
        "/v1.2.3/shop/v1.2.3/src/item", "/acme/shop/v1.2.3",
        "/acme/shop/v1.2.3/src", "/acme/shop/v1.2.3/src/",
        "/acme/shop/v1.2.3/assets/item", "/acme/shop/v01.2.3/src/item",
        "/acme/shop/v1.2.3/src//item", "/acme/shop/v1.2.3/src/./item",
        "/acme/shop/v1.2.3/src/../item", "/acme/shop/v1.2.3/src/item+",
        "/acme/shop/v1.2.3/src/item.c", "/acme/shop/v1.2.3/src/ item",
        "/acme/shop/v1.2.3/src/item ", "/acme/shop/v1.2.3/src/item  name",
        "//item/", "//item//child", "//../item", "//item/./child", "//v1.2.3",
        "//\xff", "/acme/shop/v1.2.3/src/\xff"
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        bool valid = module_path_canonical((const unsigned char *)cases[index], strlen(cases[index]));
        assert(!valid);
    }
}

/** The logical endpoint recognizer also accepts bounded unterminated source views. */
static void it_bounds_canonical_identity_reads_and_preserves_input(void) {
    const unsigned char bytes[] = {'/', '/', 'a', 0, '/', 'b'};
    const unsigned char expected[] = {'/', '/', 'a', 0, '/', 'b'};
    for (size_t length = 0; length <= sizeof(bytes); length++) {
        bool valid = module_path_canonical(bytes, length);
        assert(valid == (length == 3));
    }
    bool valid = module_path_canonical(NULL, 0);
    assert(!valid);
    valid = module_path_canonical(NULL, 100);
    assert(!valid);
    assert(memcmp(bytes, expected, sizeof(bytes)) == 0);
}

/** Enumerate independently constructed package/area/module shape partitions.
 * The three shape counts and fixed valid/invalid choices determine membership;
 * the oracle does not split or recognize the constructed path again.
 */
static void it_checks_every_bounded_package_shape_partition(void) {
    const char *vendors[] = {"acme", "vendor name", "v1.2.3", ""};
    const char *versions[] = {"v0.0.0", "v1.2.3-speed", "v01.0.0"};
    const char *areas[] = {"src", "tests", "assets"};
    for (size_t vendor = 0; vendor < 4; vendor++) {
        for (size_t packages = 0; packages <= 3; packages++) {
            for (size_t version = 0; version < 3; version++) {
                for (size_t area = 0; area < 3; area++) {
                    for (size_t modules = 0; modules <= 3; modules++) {
                        char path[200];
                        int written = snprintf(path, sizeof(path), "/%s", vendors[vendor]);
                        assert(written > 0 && (size_t)written < sizeof(path));
                        size_t length = (size_t)written;
                        for (size_t index = 0; index < packages; index++) {
                            memcpy(path + length, "/package", 8);
                            length += 8;
                        }
                        written = snprintf(path + length, sizeof(path) - length,
                            "/%s/%s", versions[version], areas[area]);
                        assert(written > 0 && (size_t)written < sizeof(path) - length);
                        length += (size_t)written;
                        for (size_t index = 0; index < modules; index++) {
                            memcpy(path + length, "/module", 7);
                            length += 7;
                        }
                        bool expected = vendor < 2 && packages != 0 && version < 2
                            && area < 2 && modules != 0;
                        bool valid = module_path_canonical((const unsigned char *)path, length);
                        assert(valid == expected);
                    }
                }
            }
        }
    }
}

/** Valid namespace depth is bounded by the actual byte view, not a guessed cap. */
static void it_accepts_deep_finite_namespaces_without_a_semantic_depth_cap(void) {
    unsigned char standard[2050];
    standard[0] = '/';
    size_t length = 1;
    for (size_t count = 0; count < 1024; count++) {
        standard[length++] = '/';
        standard[length++] = 'a';
        bool valid = module_path_canonical(standard, length);
        assert(valid);
    }
    unsigned char ordinary[2100];
    const char *prefix = "/vendor";
    size_t prefix_length = strlen(prefix);
    memcpy(ordinary, prefix, prefix_length);
    for (size_t count = 0; count < 1024; count++) {
        ordinary[prefix_length++] = '/';
        ordinary[prefix_length++] = 'a';
    }
    const char *suffix = "/v0.0.0/src/item";
    size_t suffix_length = strlen(suffix);
    memcpy(ordinary + prefix_length, suffix, suffix_length);
    bool valid = module_path_canonical(ordinary, prefix_length + suffix_length);
    assert(valid);
}

/** Raw source forms include noncanonical roots/components but never optional
 * separators, empty components, old module dots or slashes in static names.
 */
static void it_separates_raw_path_syntax_from_canonical_identity(void) {
    const char *valid[] = {"/a", "//a/b", "./a", "../a", "../../a", "~a",
        "~vendor tools/item name", "/a/../b/./c", "//./integer", "/a/..",
        "/acme/p/v1.2.3-speed/src/a", "/供应/包/v1.0.0-速度/src/项", "~a/v2.4.14"};
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        size_t length = strlen(valid[index]);
        assert(module_path_syntax((const unsigned char *)valid[index], length));
        assert(module_path_prefix((const unsigned char *)valid[index], length) == length);
    }
    const char *invalid[] = {"", "/", "//", "///a", "./", "../", "~", "~/a",
        "~ a", "~a/", "/a/", "/a//b", "/a.b", ".T", "...T", "/a/.../b",
        "/a/v01.2.3", "/a/v1.2.03", "/a/v1.2.3-", "/a/v1.2.3-a b",
        "/ a", "/a /b", "/a/ b", "/a  b", "/a/2b", "/a/\xff", "/a\t"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        assert(!module_path_syntax((const unsigned char *)invalid[index], strlen(invalid[index])));
    }
    assert(!module_path_syntax(NULL, 0));
    assert(!module_path_syntax(NULL, 99));
}

/** Token prefixes stop before structural operators, preserving exact bounded
 * input bytes; complete-span predicates reject those same remaining suffixes.
 */
static void it_bounds_shared_name_version_and_path_prefixes(void) {
    const char *cases[] = {"//item[]", "~name name/a !", "./a < value", "../a&", "/a\0b"};
    const size_t available[] = {8, 14, 11, 5, 4};
    const size_t expected[] = {6, 12, 3, 4, 2};
    for (size_t index = 0; index < 5; index++) {
        assert(module_path_prefix((const unsigned char *)cases[index], available[index]) == expected[index]);
        assert(!module_path_syntax((const unsigned char *)cases[index], available[index]));
    }
    const unsigned char exact[] = {'/', '/', 'a'};
    assert(module_path_syntax(exact, sizeof(exact)));
    assert(module_path_prefix(exact, 2) == 0);
    assert(unicode_name_prefix(NULL, 100) == 0);
    const char *names[] = {"a b!", "a  b", "a ", "a 2", "\xff", "a-", "a-b c_2[]"};
    const size_t widths[] = {3, 1, 1, 1, 0, 1, 7};
    for (size_t index = 0; index < 7; index++) {
        assert(unicode_name_prefix((const unsigned char *)names[index], strlen(names[index])) == widths[index]);
    }
    assert(module_version_prefix((const unsigned char *)"v1.2.3/a", 8) == 6);
    assert(module_version_prefix((const unsigned char *)"v1.2.3-a!", 9) == 8);
    assert(module_version_prefix((const unsigned char *)"v1.2.3-", 7) == 0);
}

/** Map a finite test spelling to its disjoint length block and base-four word.
 * The alphabet is . / a ~; membership generation below uses grammar expansion,
 * not the production recognizer's byte-scanning algorithm.
 */
static size_t finite_path_index(const char *text, size_t length) {
    size_t base = 0, power = 1, value = 0;
    for (size_t index = 0; index < length; index++) {
        base += power;
        power *= 4;
        const char *letter = strchr("./a~", text[index]);
        assert(letter != NULL);
        value = value * 4 + (size_t)(letter - "./a~");
    }
    return base + value;
}

/** Expand every component choice whose resulting path fits seven bytes.
 * Names in this alphabet are exactly a^n, and the other components are . and ...
 * Each recursion adds at least two bytes; the finite test bound proves termination.
 */
static void generate_finite_paths(bool *members, char *text, size_t length) {
    const char *components[] = {".", "..", "a", "aa", "aaa", "aaaa", "aaaaa", "aaaaaa"};
    for (size_t index = 0; index < sizeof(components) / sizeof(*components); index++) {
        size_t width = strlen(components[index]);
        if (width > 7 - length) { continue; }
        memcpy(text + length, components[index], width);
        size_t end = length + width;
        members[finite_path_index(text, end)] = true;
        if (end + 1 < 7) {
            text[end] = '/';
            generate_finite_paths(members, text, end + 1);
        }
    }
}

/** Exhaust all 21,845 strings of lengths 0..7 over ./a~ against an independently
 * expanded finite grammar set, including every root and malformed join.
 */
static void it_matches_the_complete_bounded_raw_path_language(void) {
    bool members[21845] = {false};
    char text[7];
    const char *roots[] = {"/", "//", "./", "../"};
    for (size_t root = 0; root < 4; root++) {
        size_t length = strlen(roots[root]);
        memcpy(text, roots[root], length);
        generate_finite_paths(members, text, length);
    }
    text[0] = '~';
    for (size_t width = 1; width <= 6; width++) {
        memset(text + 1, 'a', width);
        size_t end = 1 + width;
        members[finite_path_index(text, end)] = true;
        if (end + 1 < 7) {
            text[end] = '/';
            generate_finite_paths(members, text, end + 1);
        }
    }
    size_t possibilities = 1;
    for (size_t length = 0; length <= 7; length++) {
        for (size_t ordinal = 0; ordinal < possibilities; ordinal++) {
            size_t remainder = ordinal;
            for (size_t index = 0; index < length; index++) {
                text[index] = "./a~"[remainder % 4];
                remainder /= 4;
            }
            bool expected = members[finite_path_index(text, length)];
            assert(module_path_syntax((const unsigned char *)text, length) == expected);
        }
        possibilities *= 4;
    }
}

/** Register every independently specified version and logical endpoint case explicitly. */
void run(void) {
    it_separates_raw_path_syntax_from_canonical_identity();
    it_bounds_shared_name_version_and_path_prefixes();
    it_matches_the_complete_bounded_raw_path_language();
    it_accepts_exact_canonical_versions();
    it_rejects_noncanonical_or_incomplete_versions();
    it_reads_only_the_supplied_byte_view();
    it_accepts_decimal_fields_larger_than_host_integers();
    it_matches_the_complete_bounded_grammar_language();
    it_accepts_canonical_ordinary_and_standard_identities();
    it_rejects_incomplete_or_unnormalized_identities();
    it_bounds_canonical_identity_reads_and_preserves_input();
    it_checks_every_bounded_package_shape_partition();
    it_accepts_deep_finite_namespaces_without_a_semantic_depth_cap();
}

/** Execute this independent assertion leaf; parent Make owns build/run/cleanup. */
int main(void) {
    run();
    return 0;
}
