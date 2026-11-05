#ifndef AMA_LEXER_CHARACTERS_H
#define AMA_LEXER_CHARACTERS_H

static inline int is_decimal_digit(int character) {
    return character >= '0' && character <= '9';
}

static inline int is_identifier_character(int character) {
    if (character >= 'a' && character <= 'z') return 1;
    if (character >= 'A' && character <= 'Z') return 1;
    if (character >= '0' && character <= '9') return 1;
    if (character == '_') return 1;
    if (character >= 0x80 && character <= 0xFF) return 1;
    return 0;
}

static inline int is_identifier_start_character(int character) {
    if (character >= 'a' && character <= 'z') return 1;
    if (character >= 'A' && character <= 'Z') return 1;
    if (character == '_') return 1;
    if (character >= 0x80 && character <= 0xFF) return 1;
    return 0;
}

#endif
