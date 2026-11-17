/**
 * @file
 * @brief Check Ama hyphen words against a finite grammar model and pinned Unicode data.
 */

/** Recognize the independent three-state grammar over A, underscore, zero,
 * hyphen and space. Spaces optionally restart a word; a pending hyphen cannot
 * finish a word. This model does not call any production recognition function.
 */
static bool hyphen_model(const unsigned char *bytes, size_t length, bool spaces) {
    enum { WORD_START, ORDINARY_CHARACTER, PENDING_HYPHEN } state = WORD_START;
    for (size_t index = 0; index < length; index++) {
        unsigned char byte = bytes[index];
        if (byte == ' ') {
            if (!spaces || state != ORDINARY_CHARACTER) { return false; }
            state = WORD_START;
        } else if (byte == '-') {
            if (state != ORDINARY_CHARACTER) { return false; }
            state = PENDING_HYPHEN;
        } else if (byte == 'A' || byte == '_' || (byte == '0' && state != WORD_START)) {
            state = ORDINARY_CHARACTER;
        } else {
            return false;
        }
    }
    return state == ORDINARY_CHARACTER;
}

/** Exhaust every string of lengths zero through seven over five role classes;
 * independently enumerate its valid word prefixes and its full-name language.
 */
static void it_matches_hyphen_words_and_names_over_a_finite_alphabet(void) {
    const unsigned char alphabet[] = {'A', '_', '0', '-', ' '};
    size_t combinations = 1;
    size_t checked = 0;
    for (size_t length = 0; length <= 7; length++) {
        for (size_t sample = 0; sample < combinations; sample++) {
            unsigned char bytes[8];
            size_t digits = sample;
            for (size_t index = 0; index < length; index++) {
                bytes[index] = alphabet[digits % 5];
                digits /= 5;
            }
            bytes[length] = 0xff;
            size_t prefix = 0;
            for (size_t end = 1; end <= length; end++) {
                if (hyphen_model(bytes, end, false)) { prefix = end; }
            }
            assert(unicode_word_prefix(bytes, length) == prefix);
            size_t error = 99;
            bool valid = unicode_identifier(bytes, length, &error);
            assert(valid == hyphen_model(bytes, length, false));
            assert(valid ? error == 99 : error == prefix);
            error = 99;
            valid = unicode_name(bytes, length, &error);
            assert(valid == hyphen_model(bytes, length, true));
            if (valid) { assert(error == 99); }
            checked++;
        }
        combinations *= 5;
    }
    assert(checked == 97656);
}

/** Compare every possible post-hyphen scalar with independently read pinned
 * XID_Continue data; reject every incomplete UTF-8 suffix before publishing it.
 */
static void it_checks_every_post_hyphen_scalar_and_truncated_encoding(void) {
    unsigned char *properties = calloc(0x110000, 2);
    assert(properties != NULL);
    read_expected_properties(properties, properties + 0x110000);
    for (uint32_t scalar = 0; scalar <= 0x10ffff; scalar++) {
        unsigned char bytes[6] = {'A', '-'};
        size_t width = encode_scalar(scalar, bytes + 2);
        bool expected = scalar == '_' || properties[0x110000 + scalar] != 0;
        size_t prefix = unicode_word_prefix(bytes, width + 2);
        assert(prefix == (expected ? width + 2 : 1));
        size_t error = 99;
        bool valid = unicode_identifier(bytes, width + 2, &error);
        assert(valid == expected);
        assert(valid ? error == 99 : error == 1);
        for (size_t partial = 0; partial < width; partial++) {
            assert(unicode_word_prefix(bytes, partial + 2) == 1);
        }
    }
    free(properties);
}

/** Exercise original UTF-8/name bytes and invalid joins outside the finite ASCII
 * model, including allowed combining continuations and rejected dash lookalikes.
 */
static void it_preserves_hyphen_word_boundaries_and_exact_unicode_bytes(void) {
    const char *valid[] = {"item-2", "red-blue-green", "_item-_value", "item_-value",
        "ключ-2", "A-\xcc\x81", "item-2 next_3", "a-b", "a_b", "a b", "ab"};
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        size_t error = 99;
        assert(unicode_name((const unsigned char *)valid[index], strlen(valid[index]), &error));
        assert(error == 99);
    }
    const char *invalid[] = {"-item", "item-", "item--name", "item- name",
        "item -name", "item 2", "item\xe2\x80\x93name", "item\xe2\x88\x92name",
        "A-\xc0\x80", "A-\xf0\x90", " item-2", "item-2 ", "item-2  next"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        size_t error = 99;
        assert(!unicode_name((const unsigned char *)invalid[index], strlen(invalid[index]), &error));
        assert(error < strlen(invalid[index]));
    }
    assert(unicode_word_prefix(NULL, 0) == 0);
    assert(unicode_word_prefix(NULL, 7) == 0);
    assert(unicode_word_prefix((const unsigned char *)"A-B", 2) == 1);
    assert(!unicode_identifier_continue('-'));
}
