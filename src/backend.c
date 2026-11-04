#include "ama.h"

/* ============================================================================
 * Intrinsic modules
 * ============================================================================
 *
 * Intrinsics are built-in operations implemented in C rather than in Ама.
 * They are the primitives that user code is built from: arithmetic, comparison,
 * boolean logic, I/O, and memory access. Every user program ultimately calls
 * one of these (there is no other way to add two numbers, print, or access
 * memory via a pointer).
 *
 * Why intrinsics instead of language atoms?
 *   - The language core is deliberately minimal: three atoms (^, <, =) and
 *     six structural markers (?, :, {, }, !, @/&). Everything else is a module.
 *   - Arithmetic and I/O cannot be expressed in Ама itself (there is no +, no
 *     print, no syscall interface). They must be provided by the implementation.
 *   - Keeping them as modules (rather than as language constructs) preserves
 *     the "everything is a module" property and allows them to be replaced or
 *     extended later.
 *
 * Contract:
 *   - Intrinsic names are reserved. A user module with the same path is a
 *     redefinition error (caught during pass 1).
 *   - All intrinsics follow the same calling convention as user modules:
 *     parameters are read from storage[], returns are written to storage[],
 *     the function returns void.
 *   - Intrinsics do not call other modules. They are leaves in the dependency
 *     graph (no outgoing edges).
 *
 * Coverage:
 *   - Arithmetic: add, sub, mul, div, mod.
 *   - Comparison: lt, le, gt, ge, eq, ne (return 0 or 1).
 *   - Boolean: and, or, not.
 *   - I/O: print (one value, decimal, to stdout, with a trailing newline).
 *   - Memory: alloc, free (no-op), load, store.
 *
 * Gaps in this version:
 *   - No floating-point operations.
 *   - No string operations (strings would be word sequences in storage[]).
 *   - No file I/O, no network.
 *   - free() is a no-op; the allocator is a simple bump allocator.
 *   - mem/load has no bounds check; mem/store checks against STORAGE_SIZE.
 *   - No atomic operations (the runtime is single-threaded by design).
 */

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

/* ============================================================================
 * Arithmetic emitters
 * ============================================================================
 *
 * add, sub, mul operate directly on storage slots. All values are size_t;
 * overflow wraps (unsigned arithmetic). No error is reported for overflow —
 * this matches C semantics for unsigned types and the language's "no hidden
 * checks" principle.
 *
 * div and mod report division-by-zero via a second return slot named "error":
 *   error = 0 → success
 *   error = 1 → divisor was zero (quotient/remainder set to 0)
 *
 * The caller is not required to declare or read "error"; if not declared, the
 * slot is still written but never observed. This matches the language's
 * explicit-error-checking model: nothing is enforced, everything is visible.
 */

static void emit_sys_add(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_add(void) {\n");
    fprintf(out, "  storage[%u] = storage[%u] + storage[%u];\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_sub(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_sub(void) {\n");
    fprintf(out, "  storage[%u] = storage[%u] - storage[%u];\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_mul(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_mul(void) {\n");
    fprintf(out, "  storage[%u] = storage[%u] * storage[%u];\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_div(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_div(void) {\n");
    fprintf(out, "  size_t divisor = storage[%u];\n", p[1]);
    fprintf(out, "  if (divisor == 0) {\n");
    fprintf(out, "    storage[%u] = 0;\n", r[0]);
    fprintf(out, "    storage[%u] = 1;\n", r[1]);
    fprintf(out, "    return;\n");
    fprintf(out, "  }\n");
    fprintf(out, "  storage[%u] = storage[%u] / divisor;\n", r[0], p[0]);
    fprintf(out, "  storage[%u] = 0;\n", r[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_mod(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_mod(void) {\n");
    fprintf(out, "  size_t divisor = storage[%u];\n", p[1]);
    fprintf(out, "  if (divisor == 0) {\n");
    fprintf(out, "    storage[%u] = 0;\n", r[0]);
    fprintf(out, "    storage[%u] = 1;\n", r[1]);
    fprintf(out, "    return;\n");
    fprintf(out, "  }\n");
    fprintf(out, "  storage[%u] = storage[%u] %% divisor;\n", r[0], p[0]);
    fprintf(out, "  storage[%u] = 0;\n", r[1]);
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
    fprintf(out, "  storage[%u] = (storage[%u] %s storage[%u]) ? 1 : 0;\n",
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

/* ============================================================================
 * Boolean emitters
 * ============================================================================
 *
 * C's && and || treat any non-zero value as true and return 0 or 1. We use
 * them directly; operands are size_t, coerced by C. No short-circuit
 * differences matter here because the operands are already-evaluated values
 * stored in storage slots (no side effects).
 */

static void emit_sys_and(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_and(void) {\n");
    fprintf(out, "  storage[%u] = (storage[%u] && storage[%u]) ? 1 : 0;\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_or(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_or(void) {\n");
    fprintf(out, "  storage[%u] = (storage[%u] || storage[%u]) ? 1 : 0;\n",
            r[0], p[0], p[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_not(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_math_not(void) {\n");
    fprintf(out, "  storage[%u] = storage[%u] ? 0 : 1;\n", r[0], p[0]);
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
    fprintf(out, "  printf(\"%%zu\\n\", storage[%u]);\n", p[0]);
    fprintf(out, "  storage[%u] = 0;\n", r[0]);
    fprintf(out, "}\n\n");
}

/* ============================================================================
 * Memory emitters
 * ============================================================================
 *
 * The storage array is split into two regions:
 *   [0 .. static_end)      static (parameters, returns, locals)
 *   [static_end .. N)      heap (allocated by sys/mem/alloc)
 *
 * Allocator: bump allocator. Each alloc advances g_heap_top by the requested
 * size; free is a no-op. This is intentional for the prototype:
 *   - No fragmentation.
 *   - No free-list bookkeeping.
 *   - Fast (one addition per allocation).
 * The trade-off: memory is never reclaimed, so a long-running program that
 * allocates repeatedly will exhaust the heap. A future version can replace
 * this with a free-list allocator without changing the interface.
 *
 * Bounds checks:
 *   - alloc: checks that the request fits in the heap (with overflow-safe
 *     arithmetic: sz > STORAGE_SIZE - g_heap_top).
 *   - store: checks that addr < STORAGE_SIZE before writing.
 *   - load:  does NOT check; a bad address would read past the array. The
 *            language has no exceptions; load returns whatever is at that
 *            offset (possibly out of bounds). Callers should validate
 *            addresses themselves or use mem/store to probe first.
 *
 * nil (address 0):
 *   - storage[0] is a valid cell (it always holds whatever was last written
 *     there, typically 0). Dereferencing nil is not a crash; it reads or
 *     writes storage[0]. This matches the "no hidden runtime checks" model.
 */

static void emit_sys_alloc(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_mem_alloc(void) {\n");
    fprintf(out, "  size_t sz = storage[%u];\n", p[0]);
    fprintf(out, "  if (sz > STORAGE_SIZE - g_heap_top) {\n");
    fprintf(out, "    storage[%u] = 0;\n", r[0]);
    fprintf(out, "    storage[%u] = 1;\n", r[1]);
    fprintf(out, "    return;\n");
    fprintf(out, "  }\n");
    fprintf(out, "  storage[%u] = g_heap_top;\n", r[0]);
    fprintf(out, "  g_heap_top += sz;\n");
    fprintf(out, "  storage[%u] = 0;\n", r[1]);
    fprintf(out, "}\n\n");
}

static void emit_sys_free(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)p; (void)nr;
    fprintf(out, "void mod_sys_mem_free(void) {\n");
    fprintf(out, "  /* Bump allocator: freeing is not implemented.\n");
    fprintf(out, "   * Memory is reclaimed only when the program exits. */\n");
    fprintf(out, "  storage[%u] = 0;\n", r[0]);
    fprintf(out, "}\n\n");
}

static void emit_sys_load(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_mem_load(void) {\n");
    fprintf(out, "  size_t addr = storage[%u];\n", p[0]);
    fprintf(out, "  storage[%u] = (addr < STORAGE_SIZE) ? storage[addr] : 0;\n", r[0]);
    fprintf(out, "}\n\n");
}

static void emit_sys_store(FILE* out, const uint32_t* p, int np, const uint32_t* r, int nr) {
    (void)np; (void)nr;
    fprintf(out, "void mod_sys_mem_store(void) {\n");
    fprintf(out, "  size_t addr = storage[%u];\n", p[0]);
    fprintf(out, "  if (addr >= STORAGE_SIZE) {\n");
    fprintf(out, "    storage[%u] = 1;\n", r[0]);
    fprintf(out, "    return;\n");
    fprintf(out, "  }\n");
    fprintf(out, "  storage[addr] = storage[%u];\n", p[1]);
    fprintf(out, "  storage[%u] = 0;\n", r[0]);
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
    /* memory */
    {"sys/mem/alloc", {"size",0},        {"ptr","error",0},   1, 2, emit_sys_alloc},
    {"sys/mem/free",  {"ptr",0},         {"error",0},         1, 1, emit_sys_free},
    {"sys/mem/load",  {"addr",0},        {"value",0},         1, 1, emit_sys_load},
    {"sys/mem/store", {"addr","value",0},{"error",0},         2, 1, emit_sys_store},
};

static const int G_IntrinsicCount =
    (int)(sizeof(G_Intrinsics) / sizeof(G_Intrinsics[0]));

/* ============================================================================
 * Intrinsic lookup
 * ============================================================================
 *
 * find_intrinsic returns a pointer into the table (never freed) or NULL.
 * is_intrinsic_path is the public predicate; it is used by the driver to
 * decide whether a module body should be emitted from a file (user module)
 * or from an intrinsic emitter (built-in).
 *
 * Important: this check is against the ACTUAL table, not against the "sys/"
 * prefix. A user module named "sys/custom/thing" is NOT treated as intrinsic
 * (unless it happens to be in the table) and will be compiled from its file
 * normally. This allows user code to use the "sys/" prefix for its own
 * purposes if it wants, while reserving the specific intrinsic paths.
 */

static const Intrinsic* find_intrinsic(const char* path) {
    for (int i = 0; i < G_IntrinsicCount; i++)
        if (strcmp(G_Intrinsics[i].name, path) == 0)
            return &G_Intrinsics[i];
    return NULL;
}

int is_intrinsic_path(const char* path) {
    return find_intrinsic(path) != NULL;
}

/* ============================================================================
 * Intrinsic registration
 * ============================================================================
 *
 * register_intrinsics declares every intrinsic's parameters and returns in
 * G_Table, giving them offsets in storage[] just like user variables. It
 * MUST be called before pass1_declarations, so that:
 *
 *   - User modules can reference intrinsics by alias without error.
 *   - Duplicate declarations (user writes a module named "sys/math/add")
 *     are caught as redefinitions.
 *
 * All intrinsic parameters and returns are given default 0, since the emitter
 * always writes to them before reading.
 */

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

/* ============================================================================
 * Intrinsic emission
 * ============================================================================
 *
 * For each intrinsic, resolve its parameter and return names to storage
 * offsets via lookup_symbol, then call its emitter. Emitters write complete
 * C function bodies (including the signature).
 *
 * Emission happens AFTER the prologue (so storage[] exists) and BEFORE any
 * user module bodies (so user modules can call intrinsics without forward
 * declarations).
 */

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
            p[j] = s->offset;
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
            r[j] = s->offset;
        }

        in->emit(out, p, in->n_params, r, in->n_returns);
    }
}

/* ============================================================================
 * Prologue
 * ============================================================================
 *
 * Emits the storage array, the heap pointer, and the required #includes.
 *
 * Parameters:
 *   static_end  — first unused offset in the static region (== number of
 *                 variables declared by all modules, including intrinsics)
 *   total_size  — total size of storage[] (static_end + heap words)
 *
 * The heap occupies [static_end, total_size). g_heap_top starts at static_end.
 *
 * Validation:
 *   - total_size must be > static_end (the heap must be at least 1 word).
 *   - Both must fit in 32 bits (the emitter uses %u).
 */

void backend_emit_prologue(FILE* out, uint32_t static_end, uint32_t total_size) {
    if (total_size <= static_end) {
        fprintf(stderr,
                "error: prologue: total_size (%u) must be greater than "
                "static_end (%u)\n",
                total_size, static_end);
        exit(1);
    }

    fprintf(out, "/* Generated by the Ама compiler. Do not edit. */\n");
    fprintf(out, "#include <stdint.h>\n");
    fprintf(out, "#include <stdio.h>\n\n");
    fprintf(out, "#define STORAGE_SIZE %u\n", total_size);
    fprintf(out, "static size_t storage[STORAGE_SIZE];\n");
    fprintf(out, "static size_t g_heap_top = %u;\n\n", static_end);
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
    int found = 0;
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