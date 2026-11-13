/**
 * @file
 * @brief Cover every short byte view, all nonminimal widths and the complete out-of-range four-byte payload domain.
 */

/** Check all 65536 two-byte views, including ASCII prefixes, exact failure offsets and unchanged success output. */
static void it_validates_every_two_byte_view_and_preserves_output_failure_contracts(void) {
    for (unsigned first = 0; first < 256; first++) {
        for (unsigned second = 0; second < 256; second++) {
            unsigned char bytes[] = {(unsigned char)first, (unsigned char)second};
            bool valid = (first < 128 && second < 128)
                || (first >= 0xc2 && first <= 0xdf && second >= 0x80 && second <= 0xbf);
            size_t error = 99;
            assert(utf8_validate(bytes, 2, &error) == valid);
            if (valid) { assert(error == 99); }
            else { assert(error == (first < 128 ? 1 : 0)); }
        }
    }
    size_t error = 99, width = 99;
    uint32_t scalar = UINT32_MAX;
    assert(utf8_validate(NULL, 0, &error) && error == 99);
    assert(!utf8_validate(NULL, 1, &error) && error == 0);
    assert(!utf8_decode(NULL, 0, &scalar, &width) && scalar == UINT32_MAX && width == 99);
    const uint32_t boundaries[] = {0x80,0x7ff,0x800,0xd7ff,0xe000,0xffff,0x10000,0x10ffff};
    for (size_t selected = 0; selected < sizeof(boundaries) / sizeof(*boundaries); selected++) {
        unsigned char bytes[4];
        size_t length = encode_scalar(boundaries[selected], bytes);
        for (size_t position = 1; position < length; position++) {
            unsigned char original = bytes[position];
            for (unsigned replacement = 0; replacement < 256; replacement++) {
                if (replacement >= 0x80 && replacement <= 0xbf) { continue; }
                bytes[position] = (unsigned char)replacement;
                scalar = UINT32_MAX;
                width = 99;
                assert(!utf8_decode(bytes, length, &scalar, &width) && scalar == UINT32_MAX && width == 99);
            }
            bytes[position] = original;
        }
    }
}

/** Encode a payload in an intentionally chosen width rather than its canonical shortest one. */
static void expect_rejected_width(uint32_t scalar, size_t width) {
    unsigned char bytes[4];
    uint32_t remaining = scalar;
    for (size_t index = width - 1; index != 0; index--) {
        bytes[index] = (unsigned char)(0x80 | (remaining & 0x3f));
        remaining >>= 6;
    }
    bytes[0] = (unsigned char)((width == 2 ? 0xc0 : width == 3 ? 0xe0 : 0xf0) | remaining);
    uint32_t decoded = UINT32_MAX;
    size_t consumed = 99;
    assert(!utf8_decode(bytes, width, &decoded, &consumed) && decoded == UINT32_MAX && consumed == 99);
}

/** Exhaust every overlong form and every 21-bit four-byte payload above the Unicode scalar ceiling. */
static void it_rejects_all_nonminimal_widths_and_out_of_range_four_byte_payloads(void) {
    const uint32_t ceilings[] = {0,0,0x80,0x800,0x10000};
    for (size_t width = 2; width <= 4; width++) {
        for (uint32_t scalar = 0; scalar < ceilings[width]; scalar++) { expect_rejected_width(scalar, width); }
    }
    for (uint32_t scalar = 0x110000; scalar <= 0x1fffff; scalar++) { expect_rejected_width(scalar, 4); }
}
