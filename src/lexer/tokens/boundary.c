/**
 * @file
 * @brief Enforce the compact-access and single-space operator join relation.
 * [SYN-06](../../../spec/language/source-and-names.md#syn-06) gives punctuation
 * its fixed lexical joins; complete statement structure remains a parser decision.
 */
#include "internal.h"

/** These tokens open a compact access whose next name directly adjoins the opener. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06). */
static bool opens(LexKind kind) {
    return kind == LEX_DOT || kind == LEX_STANDARD_ROOT || kind == LEX_INDEX_OPEN || kind == LEX_REFERENCE_INDEX;
}
/** A storage/index suffix requires an existing complete base without any intervening separator. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06). */
static bool suffix(LexKind kind) {
    return kind == LEX_LIST || kind == LEX_REFERENCE_LIST || kind == LEX_REFERENCE_INDEX || kind == LEX_INDEX_OPEN;
}
bool lex_boundary(LexCursor *cursor, const LexToken *token, bool *reference_prefix) {
    bool separated = cursor->has_previous;
    LexKind previous = cursor->previous.kind;
    *reference_prefix = false;
    if (cursor->has_previous && previous == LEX_OUTPUT && token->kind == LEX_OUTPUT) {
        return lex_failure(cursor, token->text.start, "The retained-result marker is one compound token", "adjoining >>");
    }
    if (cursor->has_previous && (opens(previous) || (previous == LEX_REFERENCE && cursor->reference_prefix))) {
        if (token->kind != LEX_NAME) {
            return lex_failure(cursor, token->text.start, "A compact access opener requires its name", "an adjoining complete Unicode name");
        }
        separated = false;
    } else if (suffix(token->kind)) {
        if (!cursor->has_previous || (previous != LEX_NAME && previous != LEX_INDEX_CLOSE)) {
            return lex_failure(cursor, token->text.start, "A storage/index suffix requires its base", "an adjoining complete access name");
        }
        separated = false;
    } else if (token->kind == LEX_INDEX_CLOSE) {
        separated = false;
    } else if (token->kind == LEX_DOT && cursor->has_previous && previous == LEX_NAME) {
        separated = false;
    } else if (token->kind == LEX_STANDARD_ROOT && cursor->has_previous && previous == LEX_NAME) {
        return lex_failure(cursor, token->text.start, "Double dot is only a leading module-root marker", "a single adjoining component dot");
    } else if (token->kind == LEX_REFERENCE) {
        *reference_prefix = !cursor->has_previous || (previous != LEX_NAME && previous != LEX_INDEX_CLOSE);
        if (!*reference_prefix) { separated = false; }
    }
    if (token->separator != separated) {
        return lex_failure(cursor, token->text.start, "Incorrect code-token separator",
            separated ? "exactly one ASCII space" : "adjoining compact access tokens");
    }
    return true;
}
