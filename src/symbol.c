#include "ama.h"

/* Names are module-scoped. Numeric symbols receive private IDs used only
 * to name separate C variables; aliases contain module paths. */

SymbolTable G_Table = {
    .count = 0
};

/* ============================================================================
 * Internal helpers
 * ============================================================================ */

/* Copy src into dst[0 .. dst_size - 1]. On overflow, print a diagnostic and
 * exit. The message includes a short prefix of src to identify the offending
 * input without dumping a huge string. */
static void copy_string_checked(char* dst, size_t dst_size,
                                 const char* src, const char* what) {
    size_t len = strlen(src);
    if (len >= dst_size) {
        fprintf(stderr,
                "error: %s too long (%zu bytes, max %zu): '%.40s%s'\n",
                what, len, dst_size - 1, src,
                (len > 40) ? "..." : "");
        exit(1);
    }
    memcpy(dst, src, len + 1);
}

/* Return a fresh slot, checking the table capacity. The caller is responsible
 * for filling in all fields of the returned Symbol. */
static Symbol* allocate_slot(void) {
    if (G_Table.count >= MAX_SYMBOLS) {
        fprintf(stderr,
                "error: symbol table is full (max %d entries); "
                "increase MAX_SYMBOLS in ama.h\n",
                MAX_SYMBOLS);
        exit(1);
    }
    Symbol* s = &G_Table.symbols[G_Table.count++];
    memset(s, 0, sizeof(*s));
    return s;
}

/* ============================================================================
 * lookup_symbol
 * ============================================================================
 *
 * Find a symbol by its fully qualified name. Returns NULL if not present.
 * Case-sensitive, byte-wise comparison.
 */

Symbol* lookup_symbol(const char* module_path, const char* name) {
    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, module_path) == 0 &&
            strcmp(s->name, name) == 0)
            return s;
    }
    return NULL;
}

Symbol* add_declarative_symbol(const char* module_path, const char* name,
                               SymbolKind kind, Token val_token) {
    if (lookup_symbol(module_path, name) != NULL) {
        fprintf(stderr,
                "error: [%s] '%s' is already declared "
                "(redefinition at line %u)\n",
                module_path, name, (unsigned)val_token.line);
        exit(1);
    }

    Symbol* s = allocate_slot();

    copy_string_checked(s->module_path, sizeof(s->module_path),
                        module_path, "module path");
    copy_string_checked(s->name, sizeof(s->name),
                        name, "identifier");
    s->kind = kind;
    s->path_link[0] = '\0';
    s->default_int = 0;

    switch (kind) {
    case SYM_ALIAS:
        if (val_token.type != TOK_PATH) {
            fprintf(stderr,
                    "internal error: alias '%s' in '%s' must have a path "
                    "value (got token type %d)\n",
                    name, module_path, (int)val_token.type);
            exit(1);
        }
        copy_string_checked(s->path_link, sizeof(s->path_link),
                            val_token.lexeme, "alias path");
        s->id = 0;
        break;

    case SYM_PARAM:
    case SYM_RETURN:
        if (val_token.type == TOK_LITERAL) {
            s->default_int = atoi(val_token.lexeme);
        } else if (val_token.type == TOK_IDENT) {
            fprintf(stderr,
                    "error: [%s] '%s' has a variable default ('%s'); "
                    "defaults that reference other variables are not "
                    "supported yet. Use a literal or assign in code.\n",
                    module_path, name, val_token.lexeme);
            exit(1);
        } else {
            fprintf(stderr,
                    "internal error: [%s] '%s' has unexpected default "
                    "token type %d\n",
                    module_path, name, (int)val_token.type);
            exit(1);
        }
        s->id = (uint32_t)(s - G_Table.symbols);
        break;

    case SYM_VAR:
    default:
        fprintf(stderr,
                "internal error: add_declarative_symbol called with kind %d "
                "(use add_local_variable for locals)\n",
                (int)kind);
        exit(1);
    }

    return s;
}

/* ============================================================================
 * add_local_variable
 * ============================================================================
 *
 * Register a local variable of the given module. Idempotent: if the name
 * already exists in that module (as any kind), the existing symbol is
 * returned unchanged. This is used by the parser's "implicit declaration"
 * strategy: when a name appears on the left side of an assignment and is
 * not yet known, it becomes a local variable.
 *
 * Important: this function does NOT check whether the existing symbol is
 * an alias. If the caller sees an alias and calls this, the alias is
 * returned as-is. That should not happen — the parser checks for
 * SYM_ALIAS before calling — but this function is deliberately permissive:
 * it is the caller's job to reject invalid usages.
 */

Symbol* add_local_variable(const char* module_path, const char* name) {
    Symbol* existing = lookup_symbol(module_path, name);
    if (existing)
        return existing;

    Symbol* s = allocate_slot();

    copy_string_checked(s->module_path, sizeof(s->module_path),
                        module_path, "module path");
    copy_string_checked(s->name, sizeof(s->name),
                        name, "identifier");
    s->kind = SYM_VAR;
    s->id = (uint32_t)(s - G_Table.symbols);
    s->path_link[0] = '\0';
    s->default_int = 0;

    return s;
}

Symbol* get_nth_symbol(const char* module_path, int nth, SymbolKind kind) {
    if (nth < 0)
        return NULL;

    int seen = 0;
    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, module_path) != 0)
            continue;
        if (s->kind != kind)
            continue;
        if (seen == nth)
            return s;
        seen++;
    }
    return NULL;
}

/* ============================================================================
 * module_exists
 * ============================================================================
 *
 * Return nonzero if the given module path has at least one symbol. A module
 * with no symbols at all (empty params, empty returns, no locals) is not
 * representable in the current symbol table and is treated as non-existent.
 * This is a known limitation: an entirely empty module cannot be detected
 * here, but such a module is also useless (it cannot be called).
 *
 * Used by the driver to validate the entry point before emission.
 */

int module_exists(const char* module_path) {
    for (uint32_t i = 0; i < G_Table.count; i++)
        if (strcmp(G_Table.symbols[i].module_path, module_path) == 0)
            return 1;
    return 0;
}

/* ============================================================================
 * normalize_path
 * ============================================================================
 *
 * Convert a dotted path (".sys.math.add") to slash form ("sys/math/add").
 *
 *   - Leading dot, if present, is dropped.
 *   - Every other dot is replaced with a slash.
 *   - Slashes already present in the input are preserved (idempotent for
 *     already-normalized paths).
 *
 * The caller must provide out with at least MAX_PATH_LEN bytes. The function
 * exits on overflow — a silent truncation would produce a wrong path that
 * happens to compile, which is much worse than a hard error.
 *
 * Empty input or input that reduces to empty output (e.g. just ".") is an
 * error; module paths are never empty.
 */

void normalize_path(const char* in, char* out) {
    int j = 0;
    int start = (in[0] == '.') ? 1 : 0;

    for (int i = start; in[i] != '\0'; i++) {
        if (j >= MAX_PATH_LEN - 1) {
            fprintf(stderr,
                    "error: normalized path exceeds %d bytes: '%.60s%s'\n",
                    MAX_PATH_LEN - 1, in, (strlen(in) > 60) ? "..." : "");
            exit(1);
        }
        out[j++] = (in[i] == '.') ? '/' : in[i];
    }
    out[j] = '\0';

    if (out[0] == '\0') {
        fprintf(stderr,
                "error: empty path after normalization (input: '%s')\n", in);
        exit(1);
    }
}
