#ifndef AMA_LEXER_CHARACTERS_H
#define AMA_LEXER_CHARACTERS_H

static inline int is_digit_byte(int ch) {
    return ch >= '0' && ch <= '9';
}

static inline int is_ident_byte(int ch) {
    if (ch >= 'a' && ch <= 'z') return 1;
    if (ch >= 'A' && ch <= 'Z') return 1;
    if (ch >= '0' && ch <= '9') return 1;
    if (ch == '_') return 1;
    if (ch >= 0x80 && ch <= 0xFF) return 1;
    return 0;
}

static inline int is_ident_start_byte(int ch) {
    if (ch >= 'a' && ch <= 'z') return 1;
    if (ch >= 'A' && ch <= 'Z') return 1;
    if (ch == '_') return 1;
    if (ch >= 0x80 && ch <= 0xFF) return 1;
    return 0;
}

#endif
