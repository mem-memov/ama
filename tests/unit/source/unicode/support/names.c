/**
 * @file
 * @brief Verify complete multiword Unicode names, exact spelling and first invalid original-byte positions.
 */

/** Multiword validation preserves NFC/NFD/case distinctions and never broadens the single-word API. */
static void it_validates_complete_international_names_without_trimming_or_normalizing(void) {
    const char *valid[] = {"_", "item2", "item step2", "магазин товар данные", "中国 变量", "é nom", "e\xcc\x81 nom", "Item value"};
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        size_t error = 99;
        assert(unicode_name((const unsigned char *)valid[index], strlen(valid[index]), &error) && error == 99);
    }
    assert(strcmp(valid[5], valid[6]) != 0 && strcmp("Item value", "item value") != 0);
    size_t error = 99;
    assert(!unicode_identifier((const unsigned char *)"item step2", 10, &error) && error == 4);
    const char *invalid[] = {"", " item", "item ", "item  value", "item 2", "item \xcc\x81", "item\tvalue", "item\xc2\xa0value", "item\nvalue", "item .value", "item 😀"};
    const size_t offsets[] = {0,0,4,5,5,5,4,4,4,5,5};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        error = 99;
        assert(!unicode_name((const unsigned char *)invalid[index], strlen(invalid[index]), &error));
        assert(error == offsets[index]);
    }
    assert(!unicode_name(NULL, 0, &error) && error == 0);
    assert(!unicode_name(NULL, 7, &error) && error == 0);
    const unsigned char malformed[] = {'o','k',' ',0xe4,0xb8};
    assert(!unicode_name(malformed, sizeof(malformed), &error) && error == 3);
    unsigned char bounded[] = {'a',' ','b',0xff};
    assert(unicode_name(bounded, 3, &error));
    assert(!unicode_name(bounded, 4, &error) && error == 3 && bounded[3] == 0xff);
}

/** Compose every permitted scalar as a second word: its start property must apply again after space. */
static void it_applies_the_pinned_start_rule_to_every_word_over_the_complete_scalar_domain(void) {
    unsigned char bytes[6] = {'a',' '};
    for (uint32_t scalar = 0; scalar <= 0x10ffff; scalar++) {
        if (scalar >= 0xd800 && scalar <= 0xdfff) { continue; }
        size_t length = encode_scalar(scalar, bytes + 2);
        size_t error = 99;
        bool valid = unicode_name(bytes, length + 2, &error);
        assert(valid == unicode_identifier_start(scalar));
        if (valid) { assert(error == 99); }
        else { assert(error == 2); }
    }
}
