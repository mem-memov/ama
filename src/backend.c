#include "ama.h"

/* Arithmetic, comparisons, logical operations, and numeric output.
 * Intrinsics use the same separate C variables and call protocol as modules. */

typedef void (*IntrinsicEmitter)(FILE* out,
                                  const uint32_t* p, int np,
                                  const uint32_t* r, int nr);

typedef struct {
    const char* name;
    const char* params[MAX_INTRINSIC_PARAMS];
    const char* returns[MAX_INTRINSIC_RETURNS];
    int n_params;
    int n_returns;
    IntrinsicEmitter emit;
} Intrinsic;

static void emit_sys_add(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_add(void) {\n");
    fprintf(out, "  v_%u = v_%u + v_%u;\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_sub(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_sub(void) {\n");
    fprintf(out, "  v_%u = v_%u - v_%u;\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_mul(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_mul(void) {\n");
    fprintf(out, "  v_%u = v_%u * v_%u;\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_div(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_div(void) {\n");
    fprintf(out, "  size_t divisor = v_%u;\n", p[1]);
    fprintf(out, "  if (divisor == 0) {\n");
    fprintf(out, "    v_%u = 0;\n", r[0]);
    fprintf(out, "    v_%u = 1;\n", r[1]);
    fprintf(out, "    return;\n");
    fprintf(out, "  }\n");
    fprintf(out, "  v_%u = v_%u / divisor;\n", r[0], p[0]);
    fprintf(out, "  v_%u = 0;\n", r[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_mod(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_mod(void) {\n");
    fprintf(out, "  size_t divisor = v_%u;\n", p[1]);
    fprintf(out, "  if (divisor == 0) {\n");
    fprintf(out, "    v_%u = 0;\n", r[0]);
    fprintf(out, "    v_%u = 1;\n", r[1]);
    fprintf(out, "    return;\n");
    fprintf(out, "  }\n");
    fprintf(out, "  v_%u = v_%u %% divisor;\n", r[0], p[0]);
    fprintf(out, "  v_%u = 0;\n", r[1]);
    fprintf(out, "}\n\n");
}

/* ============================================================================
 * Comparison emitters
 * ============================================================================
 *
 * All comparisons return 0 (false) or 1 (true). Both operands are size_t;
 * comparisons are unsigned. The "result" return slot is the boolean.
 *
 * The emit_sys_cmp helper is used for all six operators to avoid duplication;
 * each public emitter calls it with its own C operator.
 */

static void emit_sys_cmp(FILE* out, const char* c_op, const char* c_name,
                          const uint32_t* p, const uint32_t* r) {
    fprintf(out, "void mod_%s(void) {\n", c_name);
    fprintf(out, "  v_%u = (v_%u %s v_%u) ? 1 : 0;\n",
            r[0], p[0], c_op, p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_lt(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    emit_sys_cmp(out, "<",  "sys_math_lt", p, r);
}
static void emit_sys_le(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    emit_sys_cmp(out, "<=", "sys_math_le", p, r);
}
static void emit_sys_gt(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    emit_sys_cmp(out, ">",  "sys_math_gt", p, r);
}
static void emit_sys_ge(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    emit_sys_cmp(out, ">=", "sys_math_ge", p, r);
}
static void emit_sys_eq(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    emit_sys_cmp(out, "==", "sys_math_eq", p, r);
}
static void emit_sys_ne(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    emit_sys_cmp(out, "!=", "sys_math_ne", p, r);
}

static void emit_sys_and(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_and(void) {\n");
    fprintf(out, "  v_%u = (v_%u && v_%u) ? 1 : 0;\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_or(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_or(void) {\n");
    fprintf(out, "  v_%u = (v_%u || v_%u) ? 1 : 0;\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_not(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_not(void) {\n");
    fprintf(out, "  v_%u = v_%u ? 0 : 1;\n", r[0], p[0]);
    fprintf(out, "}\n\n");
}

/* ============================================================================
 * I/O emitter
 * ============================================================================
 *
 * print writes one value to stdout in decimal, followed by '\n'. The value is
 * size_t, printed with %zu (unsigned). There is no flush — stdout is line-
 * buffered when connected to a terminal, and the '\n' triggers a flush.
 *
 * The "error" return slot is always 0 (printing never fails in this version).
 * A future version may return non-zero for I/O errors.
 */

static void emit_sys_print(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_io_print(void) {\n");
    fprintf(out, "  printf(\"%%zu\\n\", v_%u);\n", p[0]);
    fprintf(out, "  v_%u = 0;\n", r[0]);
    fprintf(out, "}\n\n");
}

/* ============================================================================
 * Intrinsic table
 * ============================================================================
 *
 * Every entry lists:
 *   name      — module path used by the compiler ("sys/math/add")
 *   params    — parameter names, in declaration order (NULL after the last)
 *   returns   — return names, in declaration order (NULL after the last)
 *   n_params  — number of parameters (redundant with the NULL terminator,
 *                but used to guard against reading past the array)
 *   n_returns — number of returns
 *   emit      — function that emits the C body
 *
 * The table is static and const. Its size determines G_IntrinsicCount; there
 * is no separate MAX_INTRINSICS constant — the table itself is the limit.
 *
 * Ordering within the table is irrelevant; register_intrinsics iterates from
 * index 0 to G_IntrinsicCount - 1.
 */

static const Intrinsic G_Intrinsics[] = {
    /* arithmetic */
    {"sys/math/add", {"left","right",0}, {"sum",0},           2, 1, emit_sys_add},
    {"sys/math/sub", {"left","right",0}, {"diff",0},          2, 1, emit_sys_sub},
    {"sys/math/mul", {"left","right",0}, {"prod",0},          2, 1, emit_sys_mul},
    {"sys/math/div", {"left","right",0}, {"quot","error",0},  2, 2, emit_sys_div},
    {"sys/math/mod", {"left","right",0}, {"rem","error",0},   2, 2, emit_sys_mod},
    /* comparison */
    {"sys/math/lt",  {"a","b",0},        {"result",0},        2, 1, emit_sys_lt},
    {"sys/math/le",  {"a","b",0},        {"result",0},        2, 1, emit_sys_le},
    {"sys/math/gt",  {"a","b",0},        {"result",0},        2, 1, emit_sys_gt},
    {"sys/math/ge",  {"a","b",0},        {"result",0},        2, 1, emit_sys_ge},
    {"sys/math/eq",  {"a","b",0},        {"result",0},        2, 1, emit_sys_eq},
    {"sys/math/ne",  {"a","b",0},        {"result",0},        2, 1, emit_sys_ne},
    /* boolean */
    {"sys/math/and", {"a","b",0},        {"result",0},        2, 1, emit_sys_and},
    {"sys/math/or",  {"a","b",0},        {"result",0},        2, 1, emit_sys_or},
    {"sys/math/not", {"a",0},            {"result",0},        1, 1, emit_sys_not},
    /* I/O */
    {"sys/io/print", {"value",0},        {"error",0},         1, 1, emit_sys_print},
};

static const int G_IntrinsicCount =
    (int)(sizeof(G_Intrinsics) / sizeof(G_Intrinsics[0]));

static const Intrinsic* find_intrinsic(const char* path) {
    for (int i = 0; i < G_IntrinsicCount; i++)
        if (strcmp(G_Intrinsics[i].name, path) == 0)
            return &G_Intrinsics[i];
    return NULL;
}

int is_intrinsic_path(const char* path) {
    return find_intrinsic(path) != NULL;
}

void register_intrinsics(void) {
    Token dummy;
    memset(&dummy, 0, sizeof(dummy));
    dummy.type = TOK_LITERAL;
    strcpy(dummy.lexeme, "0");

    for (int i = 0; i < G_IntrinsicCount; i++) {
        const Intrinsic* in = &G_Intrinsics[i];

        for (int j = 0; j < in->n_params; j++) {
            if (in->params[j] == NULL) {
                fprintf(stderr,
                        "internal error: intrinsic '%s' declares n_params=%d "
                        "but params[%d] is NULL\n",
                        in->name, in->n_params, j);
                exit(1);
            }
            add_declarative_symbol(in->name, in->params[j], SYM_PARAM, dummy);
        }
        for (int j = 0; j < in->n_returns; j++) {
            if (in->returns[j] == NULL) {
                fprintf(stderr,
                        "internal error: intrinsic '%s' declares n_returns=%d "
                        "but returns[%d] is NULL\n",
                        in->name, in->n_returns, j);
                exit(1);
            }
            add_declarative_symbol(in->name, in->returns[j], SYM_RETURN, dummy);
        }
    }
}

void backend_emit_intrinsics(FILE* out) {
    for (int i = 0; i < G_IntrinsicCount; i++) {
        const Intrinsic* in = &G_Intrinsics[i];

        uint32_t p[MAX_INTRINSIC_PARAMS];
        uint32_t r[MAX_INTRINSIC_RETURNS];

        for (int j = 0; j < in->n_params; j++) {
            Symbol* s = lookup_symbol(in->name, in->params[j]);
            if (!s) {
                fprintf(stderr,
                        "internal error: intrinsic '%s' parameter '%s' "
                        "not registered\n",
                        in->name, in->params[j]);
                exit(1);
            }
            p[j] = s->id;
        }
        for (int j = 0; j < in->n_returns; j++) {
            Symbol* s = lookup_symbol(in->name, in->returns[j]);
            if (!s) {
                fprintf(stderr,
                        "internal error: intrinsic '%s' return '%s' "
                        "not registered\n",
                        in->name, in->returns[j]);
                exit(1);
            }
            r[j] = s->id;
        }

        in->emit(out, p, in->n_params, r, in->n_returns);
    }
}

/* ============================================================================
 * main() wrapper
 * ============================================================================
 *
 * Emits a main() that calls the entry module and returns 0. The entry module
 * is called by its C function name: "math/square" → "mod_math_square()".
 *
 * Validation:
 *   - The entry module must not take parameters. Entry modules are the
 *     program's argv[]-free entry point; passing values to them is not
 *     supported.
 *   - The entry module must exist (checked by the driver before calling
 *     this function; re-checked here for safety).
 */

void backend_emit_main(FILE* out, const char* entry_path) {
    /* Build the C function name. */
    char fn[MAX_PATH_LEN + 8];
    int len = snprintf(fn, sizeof(fn), "mod_");
    if (len < 0 || (size_t)len >= sizeof(fn)) {
        fprintf(stderr, "error: entry path too long\n");
        exit(1);
    }
    for (const char* p = entry_path; *p; p++) {
        if ((size_t)len + 1 >= sizeof(fn)) {
            fprintf(stderr, "error: entry path too long\n");
            exit(1);
        }
        fn[len++] = (*p == '/') ? '_' : *p;
    }
    fn[len] = '\0';

    /* Verify the entry module exists and takes no parameters. */
    int found = graph_has_node(entry_path);
    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, entry_path) != 0)
            continue;
        found = 1;
        if (s->kind == SYM_PARAM) {
            fprintf(stderr,
                    "error: entry module '%s' must take no parameters "
                    "(found parameter '%s')\n",
                    entry_path, s->name);
            exit(1);
        }
    }
    if (!found) {
        fprintf(stderr, "error: entry module '%s' not found\n", entry_path);
        exit(1);
    }

    fprintf(out, "int main(void) {\n");
    emit_call_init(entry_path);
    fprintf(out, "  %s();\n", fn);
    fprintf(out, "  return 0;\n");
    fprintf(out, "}\n");
}
/* Symbols have private compiler IDs, never addresses visible to programs. */
void backend_emit_prologue(FILE* out) {
    fprintf(out, "/* Generated by the Ama compiler. */\n");
    fprintf(out, "#include <stddef.h>\n#include <stdio.h>\n\n");
    for (uint32_t i = 0; i < G_Table.count; i++) {
        const Symbol* s = &G_Table.symbols[i];
        if (s->kind != SYM_ALIAS)
            fprintf(out, "static size_t v_%u;\n", s->id);
    }
    fputc('\n', out);
}
