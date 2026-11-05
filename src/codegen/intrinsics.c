#include "codegen/intrinsics.h"
#include "codegen/emitter.h"
#include "symbols/table.h"
#include <stdlib.h>
#include <string.h>

typedef enum { INTR_BINARY, INTR_DIVISION, INTR_NOT, INTR_PRINT } IntrinsicKind;

typedef struct {
    const char* name;
    const char* params[MAX_INTRINSIC_PARAMS];
    const char* returns[MAX_INTRINSIC_RETURNS];
    int n_params;
    int n_returns;
    IntrinsicKind kind;
    const char* op;
} Intrinsic;

/* Signatures and C operators stay together; no per-operation function wrappers. */
static const Intrinsic G_Intrinsics[] = {
    {"sys/math/add", {"left","right"}, {"sum"},          2, 1, INTR_BINARY, "+"},
    {"sys/math/sub", {"left","right"}, {"diff"},         2, 1, INTR_BINARY, "-"},
    {"sys/math/mul", {"left","right"}, {"prod"},         2, 1, INTR_BINARY, "*"},
    {"sys/math/div", {"left","right"}, {"quot","error"}, 2, 2, INTR_DIVISION, "/"},
    {"sys/math/mod", {"left","right"}, {"rem","error"},  2, 2, INTR_DIVISION, "%"},
    {"sys/math/lt",  {"a","b"},        {"result"},       2, 1, INTR_BINARY, "<"},
    {"sys/math/le",  {"a","b"},        {"result"},       2, 1, INTR_BINARY, "<="},
    {"sys/math/gt",  {"a","b"},        {"result"},       2, 1, INTR_BINARY, ">"},
    {"sys/math/ge",  {"a","b"},        {"result"},       2, 1, INTR_BINARY, ">="},
    {"sys/math/eq",  {"a","b"},        {"result"},       2, 1, INTR_BINARY, "=="},
    {"sys/math/ne",  {"a","b"},        {"result"},       2, 1, INTR_BINARY, "!="},
    {"sys/math/and", {"a","b"},        {"result"},       2, 1, INTR_BINARY, "&&"},
    {"sys/math/or",  {"a","b"},        {"result"},       2, 1, INTR_BINARY, "||"},
    {"sys/math/not", {"a"},            {"result"},       1, 1, INTR_NOT, NULL},
    {"sys/io/print", {"value"},        {"error"},        1, 1, INTR_PRINT, NULL},
};

static const int G_IntrinsicCount =
    (int)(sizeof(G_Intrinsics) / sizeof(G_Intrinsics[0]));

int is_intrinsic_path(const char* path) {
    for (int i = 0; i < G_IntrinsicCount; i++)
        if (strcmp(G_Intrinsics[i].name, path) == 0)
            return 1;
    return 0;
}

static void register_names(const Intrinsic* in, const char* const* names,
                            int count, SymbolKind kind) {
    Token zero = {0};
    zero.type = TOK_LITERAL;
    strcpy(zero.lexeme, "0");
    for (int i = 0; i < count; i++) {
        if (!names[i]) {
            fprintf(stderr, "internal error: intrinsic '%s' has an incomplete signature\n",
                    in->name);
            exit(1);
        }
        add_declarative_symbol(in->name, names[i], kind, zero);
    }
}

void register_intrinsics(void) {
    for (int i = 0; i < G_IntrinsicCount; i++) {
        const Intrinsic* in = &G_Intrinsics[i];
        register_names(in, in->params, in->n_params, SYM_PARAM);
        register_names(in, in->returns, in->n_returns, SYM_RETURN);
    }
}

static void resolve_ids(const Intrinsic* in, const char* const* names,
                         int count, uint32_t* ids) {
    for (int i = 0; i < count; i++) {
        Symbol* symbol = lookup_symbol(in->name, names[i]);
        if (!symbol) {
            fprintf(stderr, "internal error: intrinsic '%s' name '%s' not registered\n",
                    in->name, names[i]);
            exit(1);
        }
        ids[i] = symbol->id;
    }
}

/* Division and modulo share the explicit error-result convention. */
static void emit_division(FILE* out, const char* op,
                           const uint32_t* p, const uint32_t* r) {
    fprintf(out, "  size_t divisor = v_%u;\n", p[1]);
    fprintf(out, "  if (divisor == 0) {\n");
    fprintf(out, "    v_%u = 0;\n", r[0]);
    fprintf(out, "    v_%u = 1;\n", r[1]);
    fprintf(out, "    return;\n  }\n");
    fprintf(out, "  v_%u = v_%u %s divisor;\n", r[0], p[0], op);
    fprintf(out, "  v_%u = 0;\n", r[1]);
}

static void emit_intrinsic_body(FILE* out, const Intrinsic* in,
                                 const uint32_t* p, const uint32_t* r) {
    switch (in->kind) {
    case INTR_BINARY:
        /* C comparisons and logical operators already return exactly 0 or 1. */
        fprintf(out, "  v_%u = (v_%u %s v_%u);\n", r[0], p[0], in->op, p[1]);
        break;
    case INTR_DIVISION:
        emit_division(out, in->op, p, r);
        break;
    case INTR_NOT:
        fprintf(out, "  v_%u = !v_%u;\n", r[0], p[0]);
        break;
    case INTR_PRINT:
        fprintf(out, "  printf(\"%%zu\\n\", v_%u);\n", p[0]);
        fprintf(out, "  v_%u = 0;\n", r[0]);
        break;
    }
}

void backend_emit_intrinsics(FILE* out) {
    for (int i = 0; i < G_IntrinsicCount; i++) {
        const Intrinsic* in = &G_Intrinsics[i];
        uint32_t p[MAX_INTRINSIC_PARAMS];
        uint32_t r[MAX_INTRINSIC_RETURNS];
        resolve_ids(in, in->params, in->n_params, p);
        resolve_ids(in, in->returns, in->n_returns, r);
        fputs("void ", out);
        emit_module_name(out, in->name);
        fputs("(void) {\n", out);
        emit_intrinsic_body(out, in, p, r);
        fputs("}\n\n", out);
    }
}
