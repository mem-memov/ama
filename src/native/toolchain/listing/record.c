/**
 * @file
 * @brief Read bounded POSIX nm-style token records without accepting control-byte truncation.
 *
 * [STACK-04](../../../../spec/implementation/compiler.md#stack-04)
 * The verified tool workflow supplies an unstripped non-LTO listing; malformed token/hex records fail closed.
 */
#include "internal.h"

#include <stdbool.h>

/** Recognize only nm token separators, ASCII space and TAB, independently of Ama source whitespace rules.
 * Component contract: [STACK-04](../../../../spec/implementation/compiler.md#stack-04). */
static bool is_space(unsigned char byte) {
    return byte == ' ' || byte == '\t';
}

/** Require every byte of an already nonempty token to be an ASCII hexadecimal digit.
 * Component contract: [STACK-04](../../../../spec/implementation/compiler.md#stack-04). */
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

/** Skip listing separators and borrow one nonempty bounded token; embedded control bytes fail rather than truncating the name.
 * Component contract: [STACK-04](../../../../spec/implementation/compiler.md#stack-04). */
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

/** Read at most four bounded tokens, preserving record completeness; excess/invalid tokens return zero.
 * Component contract: [STACK-04](../../../../spec/implementation/compiler.md#stack-04). */
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
