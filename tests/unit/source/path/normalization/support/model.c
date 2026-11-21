/**
 * @file
 * @brief Compare forward reduction with independent reverse cancellation and prefix-balance models.
 * Domain: both namespace roots, ./ and ../ starts, and all 4-ary component words
 * of lengths 1 through 6 over a, b, dot and parent (21840 complete operands).
 */
/** Reverse traversal cancels names strictly to each parent's left; prefix balances independently locate underflow. */
static void it_matches_all_bounded_reverse_cancellation_models(void) {
    const char *tokens[] = {"a", "b", ".", ".."};
    for (size_t standard = 0; standard < 2; standard++) {
        for (size_t initial_parent = 0; initial_parent < 2; initial_parent++) {
            setup_paths();
            const ModulePathValue *base = canonical(standard ? "//a/b" : "/a/b");
            size_t possibilities = 4;
            for (size_t length = 1; length <= 6; length++) {
                for (size_t code = 0; code < possibilities; code++) {
                    size_t choices[6], offsets[6], number = code;
                    char input[64];
                    strcpy(input, initial_parent ? "../" : "./");
                    size_t position = strlen(input);
                    int balance = 2 - (int)initial_parent;
                    size_t underflow = SIZE_MAX;
                    for (size_t index = 0; index < length; index++) {
                        choices[index] = number % 4;
                        number /= 4;
                        if (index != 0) { input[position++] = '/'; }
                        offsets[index] = position;
                        size_t size = strlen(tokens[choices[index]]);
                        memcpy(input + position, tokens[choices[index]], size);
                        position += size;
                        if (choices[index] < 2) { balance++; }
                        if (choices[index] == 3) { balance--; }
                        if (balance < 0 && underflow == SIZE_MAX) { underflow = offsets[index]; }
                    }
                    input[position] = 0;
                    SourceSpan expression = text(input);
                    const ModulePathValue *value = base;
                    SourceIssue issue = {0};
                    ModulePathStatus status = module_path_normalize(&paths, expression, base, NULL, NULL, &value, &issue);
                    if (underflow != SIZE_MAX) {
                        assert(status == MODULE_PATH_UNDERFLOW && value == base);
                        assert(issue.span.source == expression.source && issue.span.start == underflow && issue.span.end == underflow + 2);
                        continue;
                    }
                    assert(status == MODULE_PATH_OK && value->root == base->root);
                    char reversed[8];
                    size_t origins[8], retained = 0, pending = 0;
                    for (size_t index = length; index != 0; index--) {
                        size_t token = choices[index - 1];
                        if (token == 3) { pending++; }
                        else if (token < 2) {
                            if (pending != 0) { pending--; }
                            else { reversed[retained] = tokens[token][0]; origins[retained++] = offsets[index - 1]; }
                        }
                    }
                    pending += initial_parent;
                    const char initial[] = {'b', 'a'};
                    for (size_t index = 0; index < 2; index++) {
                        if (pending != 0) { pending--; }
                        else { reversed[retained] = initial[index]; origins[retained++] = SIZE_MAX; }
                    }
                    assert(pending == 0 && value->count == retained);
                    char expected[32];
                    size_t written = standard ? 2 : 1;
                    memcpy(expected, standard ? "//" : "/", written);
                    for (size_t index = 0; index < retained; index++) {
                        if (index != 0) { expected[written++] = '/'; }
                        expected[written++] = reversed[retained - index - 1];
                        const ModulePathPart *part = &value->parts[index];
                        size_t origin = origins[retained - index - 1];
                        assert(part->length == 1 && part->bytes[0] == (unsigned char)expected[written - 1]);
                        if (origin == SIZE_MAX) { assert(part->origin.source == NULL); }
                        else { assert(part->origin.source == expression.source && part->origin.start == origin && part->origin.end == origin + 1); }
                    }
                    expected[written] = 0;
                    assert(value->length == written && strcmp(value->text, expected) == 0);
                }
                possibilities *= 4;
            }
            finish_paths();
        }
    }
}
