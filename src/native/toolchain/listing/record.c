#include "internal.h"

#include <stdbool.h>

static bool is_space(unsigned char byte) {
    return byte == ' ' || byte == '\t';
}

static bool is_hex(AmaByteView word) {
    for (size_t index = 0; index < word.length; index++) {
        unsigned char byte = word.data[index];
        if (!((byte >= '0' && byte <= '9') || (byte >= 'a' && byte <= 'f') ||
            (byte >= 'A' && byte <= 'F'))) {
            return false;
        }
    }
    return true;
}

static bool next_word(AmaByteView line, size_t *position, AmaByteView *word) {
    while (*position < line.length && is_space(line.data[*position])) {
        (*position)++;
    }
    size_t start = *position;
    while (*position < line.length && !is_space(line.data[*position])) {
        unsigned char byte = line.data[(*position)++];
        if (byte < 32 || byte == 127) {
            return false;
        }
    }
    *word = (AmaByteView) {line.data + start, *position - start};
    return word->length != 0;
}

static size_t read_words(AmaByteView line, AmaByteView words[4]) {
    size_t position = 0;
    size_t count = 0;
    while (position < line.length) {
        while (position < line.length && is_space(line.data[position])) {
            position++;
        }
        if (position == line.length) {
            break;
        }
        if (count == 4 || !next_word(line, &position, &words[count])) {
            return 0;
        }
        count++;
    }
    return count;
}

bool ama_listing_record(AmaByteView line, AmaByteView *symbol) {
    AmaByteView words[4];
    size_t count = read_words(line, words);
    if (count < 2 || words[1].length != 1) {
        return false;
    }
    unsigned char type = words[1].data[0];
    if (!((type >= 'a' && type <= 'z') || (type >= 'A' && type <= 'Z') || type == '?')) {
        return false;
    }
    for (size_t index = 2; index < count; index++) {
        if (!is_hex(words[index])) {
            return false;
        }
    }
    *symbol = words[0];
    return true;
}
