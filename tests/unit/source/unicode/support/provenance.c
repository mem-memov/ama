/**
 * @file
 * @brief Check the pinned Unicode edition and full vendored source checksum before trusting regenerated tables.
 */
#include "source/identity/hash/sha256.h"

/** Hash original vendored bytes against the recorded edition checksum, independently of property parsing. */
static void it_verifies_the_exact_pinned_unicode_edition_and_source_checksum(void) {
    assert(strcmp(AMA_UNICODE_VERSION, "17.0.0") == 0);
    FILE *input = fopen("../../../../third_party/unicode/17.0.0/DerivedCoreProperties.txt", "rb");
    assert(input != NULL && fseek(input, 0, SEEK_END) == 0);
    long measured = ftell(input);
    assert(measured > 0 && (unsigned long)measured <= SIZE_MAX && fseek(input, 0, SEEK_SET) == 0);
    size_t length = (size_t)measured;
    unsigned char *bytes = malloc(length);
    assert(bytes != NULL && fread(bytes, 1, length, input) == length && !ferror(input) && fclose(input) == 0);
    unsigned char digest[32];
    identity_sha256(bytes, length, digest);
    free(bytes);
    static const unsigned char expected[] = {
        0x24,0xc7,0xfe,0xd1,0x19,0x5c,0x48,0x2f,0xaa,0xef,0xd5,0xc1,0xe7,0xeb,0x82,0x1c,
        0x5e,0xe1,0xfb,0x6d,0xe0,0x7e,0xcd,0xba,0xa6,0x4b,0x56,0xa9,0x9d,0xa2,0x2c,0x08
    };
    assert(memcmp(digest, expected, sizeof(expected)) == 0);
}
