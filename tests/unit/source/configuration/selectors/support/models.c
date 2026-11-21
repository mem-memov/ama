/**
 * @file
 * @brief Enumerate all bounded star substitutions independently of production DP rows.
 * Contract: [selector model](../../../../../../docs/implementation/project-configuration.md#t263-selector-matching-contract).
 */

/** Enumerate each possible consumed ASCII substring at a star; no DP state is shared. */
static bool word_model(const char *pattern, const char *word) {
    if (*pattern == 0) { return *word == 0; }
    if (*pattern != '*') { return *word == *pattern && *word != 0 && word_model(pattern + 1, word + 1); }
    for (size_t end = 0;; end++) {
        if (word_model(pattern + 1, word + end)) { return true; }
        if (word[end] == 0) { return false; }
    }
}

/** Decode an integer into a fixed-width independent Cartesian-product fixture. */
static void spelling(char *output, size_t value, size_t length, const char *alphabet, size_t radix) {
    for (size_t index = 0; index < length; index++) { output[index] = alphabet[value % radix]; value /= radix; }
    output[length] = 0;
}

/** Exhaust all 120 nonempty patterns over a,b,* up to length four and 62 words up to five. */
static void it_matches_every_bounded_scalar_expansion_model(void) {
    begin();
    size_t patterns_count = 3;
    for (size_t width = 1; width <= 4; width++, patterns_count *= 3) {
        for (size_t pattern = 0; pattern < patterns_count; pattern++) {
            char suffix[5], expression[80];
            spelling(suffix, pattern, width, "ab*", 3);
            assert(snprintf(expression, sizeof(expression), "/a/pkg/v1.0.0/src/%s", suffix) > 0);
            const DependencySelector *selector = compile(expression);
            size_t words = 2;
            for (size_t length = 1; length <= 5; length++, words *= 2) {
                for (size_t word = 0; word < words; word++) {
                    char value[6], identity[80];
                    spelling(value, word, length, "ab", 2);
                    assert(snprintf(identity, sizeof(identity), "/a/pkg/v1.0.0/src/%s", value) > 0);
                    assert(matches(selector, identity) == word_model(suffix, value));
                }
            }
        }
    }
    finish();
}

/** Enumerate all nonempty whole-component partitions at an exact star token. */
static bool path_model(const char *const *patterns, size_t p, const char *const *words, size_t w) {
    if (p == 0) { return w == 0; }
    if (strcmp(patterns[0], "*") != 0) {
        return w != 0 && word_model(patterns[0], words[0]) && path_model(patterns + 1, p - 1, words + 1, w - 1);
    }
    for (size_t end = 1; end <= w; end++) {
        if (path_model(patterns + 1, p - 1, words + end, w - end)) { return true; }
    }
    return false;
}

/** Exhaust all 340 path patterns of 1..4 components and all 30 binary component paths. */
static void it_matches_every_bounded_component_partition_model(void) {
    begin();
    const char *tokens[] = {"a", "*", "a*", "**"};
    size_t possibilities = 4;
    for (size_t length = 1; length <= 4; length++, possibilities *= 4) {
        for (size_t pattern = 0; pattern < possibilities; pattern++) {
            const char *parts[4];
            char expression[80] = "/a/pkg/v1.0.0/src";
            size_t code = pattern;
            for (size_t index = 0; index < length; index++) {
                parts[index] = tokens[code % 4]; code /= 4;
                strcat(expression, "/"); strcat(expression, parts[index]);
            }
            const DependencySelector *selector = compile(expression);
            size_t words = 2;
            for (size_t count = 1; count <= 4; count++, words *= 2) {
                for (size_t word = 0; word < words; word++) {
                    const char *values[4];
                    char identity[80] = "/a/pkg/v1.0.0/src";
                    size_t encoded = word;
                    for (size_t index = 0; index < count; index++) {
                        values[index] = encoded % 2 == 0 ? "a" : "b"; encoded /= 2;
                        strcat(identity, "/"); strcat(identity, values[index]);
                    }
                    assert(matches(selector, identity) == path_model(parts, length, values, count));
                }
            }
        }
    }
    finish();
}
