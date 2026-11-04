#ifndef AMA_H
#define AMA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/* ============================================================================
 * Compile-time limits
 * ============================================================================
 *
 * All buffers and arrays are sized at compile time. Exceeding a limit is a
 * hard error: the compiler prints a message and calls exit(1).
 *
 * MAX_MODULES              — max nodes in the dependency graph, and max edges
 *                            per node.
 * MAX_MODULE_FILES         — max source files scanned by the driver.
 * MAX_SYMBOLS              — max entries in the global symbol table.
 * MAX_INTRINSIC_PARAMS     — max parameters per intrinsic module.
 * MAX_INTRINSIC_RETURNS    — max return slots per intrinsic module.
 * HEAP_WORDS_DEFAULT       — default size of the generated heap region
 *                            (in size_t words).
 *
 * MAX_IDENT_LEN     — identifiers (parameter, return, variable names).
 * MAX_PATH_LEN      — module paths ("math/add", "core/types/int").
 * MAX_PATH_LINK_LEN — alias targets (".sys.math.add") and absolute paths.
 * MAX_LEXEME_LEN    — any token's text: identifier, literal, or path.
 */

#define MAX_MODULES            512
#define MAX_MODULE_FILES       512
#define MAX_SYMBOLS            8192
#define MAX_INTRINSIC_PARAMS   8
#define MAX_INTRINSIC_RETURNS  8
#define HEAP_WORDS_DEFAULT     (1024 * 1024)

#define MAX_IDENT_LEN          256
#define MAX_PATH_LEN           512
#define MAX_PATH_LINK_LEN      1024
#define MAX_LEXEME_LEN         256

/* ============================================================================
 * Tokens
 * ============================================================================
 *
 * is_bol is 1 if the token is the first on its line (after indentation).
 * is_after_blank is 1 if at least one blank line occurred between the
 * previous significant token and this one. Both flags are used by the
 * parser to implement "greedy binding" for branch/loop conditions.
 */

typedef enum {
    TOK_IDENT, TOK_LITERAL, TOK_PATH,
    TOK_LESS, TOK_GREATER, TOK_AT, TOK_AMP,
    TOK_QUESTION, TOK_COLON, TOK_LBRACE, TOK_RBRACE, TOK_BANG,
    TOK_EOF, TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME_LEN];
    uint32_t line;
    uint32_t indent;
    int is_bol;
    int is_after_blank;
} Token;

typedef struct {
    long params_start, params_end;
    long returns_start, returns_end;
    long code_start, code_end;

    /* 1-based line numbers of the first line in each section.
     * Used by init_lexer so token.line refers to the real file line,
     * not to a line relative to the section start. */
    uint32_t params_line;
    uint32_t returns_line;
    uint32_t code_line;
} ModuleSections;

/* ============================================================================
 * Symbol table
 * ============================================================================
 *
 * Every declared name (parameter, return slot, local variable, or alias) is
 * registered with a unique offset in the global storage array. Aliases do
 * not consume offsets; they store a path_link instead.
 *
 * default_int holds the integer literal used as the declaration's default
 * value. Defaults that reference other variables or paths are not supported
 * in the current implementation; they would require an additional "kind"
 * field in this struct.
 *
 * Every function below calls exit(1) on error (duplicate symbol, overflow,
 * unknown name). The compiler does not attempt to recover.
 */

typedef enum { SYM_VAR, SYM_ALIAS, SYM_PARAM, SYM_RETURN } SymbolKind;

typedef struct {
    char module_path[MAX_PATH_LEN];
    char name[MAX_IDENT_LEN];
    SymbolKind kind;
    uint32_t offset;
    char path_link[MAX_PATH_LINK_LEN];
    int default_int;
} Symbol;

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    uint32_t count;
    uint32_t global_storage_pointer;
} SymbolTable;

extern SymbolTable G_Table;

Symbol* lookup_symbol(const char* module_path, const char* name);
Symbol* add_declarative_symbol(const char* module_path, const char* name,
                               SymbolKind kind, Token val_token);
Symbol* add_local_variable(const char* module_path, const char* name);
Symbol* get_nth_symbol(const char* module_path, int nth, SymbolKind kind);
int     module_exists(const char* module_path);
void    normalize_path(const char* in, char* out);

/* ============================================================================
 * Lexer
 * ============================================================================
 *
 * split_module_sections reads the raw file and returns byte offsets of the
 * three sections (params, returns, code), validated for exactly two blank
 * separators. init_lexer positions a lexer at a byte range. get_next_token
 * returns one token at a time, filtering comments and blank lines.
 */

typedef struct {
    FILE* f;
    long end_pos;
    uint32_t current_line;
    int is_first_on_line;
    uint32_t indent_count;
    int blank_seen;
} Lexer;

int   split_module_sections(FILE* f, ModuleSections* sections);
void  init_lexer(Lexer* L, FILE* f, long start_pos, long end_pos,
                 uint32_t start_line);
Token get_next_token(Lexer* L);
void  parse_declaration_section(Lexer* L, const char* module_path,
                                SymbolKind target_kind);

/* ============================================================================
 * Token stream and parser
 * ============================================================================
 *
 * CodeParser holds the token array (produced by the driver from a module's
 * code section) and the module path for error messages. parse_statement
 * parses one statement at the given indent level.
 */

typedef struct {
    Token* data;
    int size;
    int pos;
} TokenStream;

typedef struct {
    TokenStream* stream;
    const char* module_path;
} CodeParser;

Token* ts_peek (TokenStream* t);
Token* ts_peek2(TokenStream* t);
Token  ts_next (TokenStream* t);

void parse_statement(CodeParser* P, int indent);

/* ============================================================================
 * Emitters
 * ============================================================================
 *
 * All emitters write C code into G_Out, which is opened by the driver and
 * closed after main() is emitted. Emitters assume the parser has already
 * validated the code: no arity checks, no type checks, no bounds checks.
 */

extern FILE* G_Out;

void emit_assign_const(uint32_t dst, int val);
void emit_assign_var(uint32_t dst, uint32_t src);
void emit_assign_load_ind(uint32_t dst, uint32_t ptr);
void emit_assign_addr(uint32_t dst, uint32_t addr);

void emit_store_indirect_const(uint32_t ptr, int val);
void emit_store_indirect_var(uint32_t ptr, uint32_t src);
void emit_store_indirect_load_ind(uint32_t ptr, uint32_t src_ptr);
void emit_store_indirect_addr(uint32_t ptr, uint32_t addr);

void emit_break(void);
void emit_return(void);

void emit_branch_start(uint32_t cond);
void emit_branch_else(void);
void emit_branch_end(void);

void emit_loop_start(uint32_t cond);
void emit_loop_end(void);

void emit_call_init(const char* callee_path);
void emit_call_param_val(uint32_t dst, uint32_t src);
void emit_call_param_const(uint32_t dst, int val);
void emit_call_param_addr(uint32_t dst, uint32_t addr);
void emit_call_result(uint32_t dst, uint32_t src);
void emit_call_exec(const char* callee_path);

/* ============================================================================
 * Dependency graph and topological sort
 * ============================================================================
 *
 * build_and_validate_project_graph constructs the graph from G_Table:
 * every module in G_Table becomes a node, every alias becomes an edge.
 * It then runs DFS post-order over all nodes, detecting cycles (recursion
 * is forbidden by the language spec).
 *
 * The caller must provide order_out with at least MAX_MODULES ints. The
 * function returns the number of nodes written (in topological order:
 * leaves first, entry last) and calls exit(1) on recursion or overflow.
 */

int build_and_validate_project_graph(int* order_out);

/* Returns the path of the graph node at the given index, or NULL if the
 * index is out of range. The returned pointer remains valid until the next
 * call to build_and_validate_project_graph (if ever). */
const char* G_Graph_path(int index);

/* ============================================================================
 * Backend
 * ============================================================================
 *
 * register_intrinsics populates G_Table with the parameters and returns of
 * every built-in module. It must be called before pass1_declarations.
 *
 * is_intrinsic_path returns nonzero for any module path starting with "sys/".
 *
 * backend_emit_prologue writes the storage array and g_heap_top.
 * backend_emit_intrinsics writes the bodies of all intrinsic modules.
 * backend_emit_main writes a main() that calls the entry module.
 */

void register_intrinsics(void);
int  is_intrinsic_path(const char* path);
void backend_emit_prologue(FILE* out, uint32_t static_end, uint32_t total_size);
void backend_emit_intrinsics(FILE* out);
void backend_emit_main(FILE* out, const char* entry_path);

/* ============================================================================
 * Driver
 * ============================================================================
 *
 * compile_project performs the full pipeline:
 *   scan → register_intrinsics → pass1 → topo-sort → prologue →
 *   intrinsics → module bodies → main()
 *
 * G_Files is populated by scan_dir. Each entry stores a source file's
 * relative path (used as the module path) and its absolute path.
 */

typedef struct {
    char rel_path[MAX_PATH_LEN];
    char abs_path[MAX_PATH_LINK_LEN];
} ModuleFile;

extern ModuleFile G_Files[];
extern int        G_FileCount;

void compile_project(const char* root, const char* entry, const char* out_path);

#endif /* AMA_H */