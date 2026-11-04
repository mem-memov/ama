#include "ama.h"

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

/* ============================================================================
 * Module file registry
 * ============================================================================
 *
 * The driver scans the project tree and stores every regular, non-hidden file
 * as a (relative, absolute) pair. The relative path is the module path used
 * throughout the compiler ("math/add", "main"). The absolute path is used only
 * for opening the file.
 *
 * Files are processed in two passes:
 *   Pass 1 — parse params/returns of every module. After this pass, G_Table
 *            contains all declarations of the project (plus intrinsics), and
 *            the dependency graph can be built.
 *   Pass 2 — parse the code section of every module in topological order.
 *
 * Splitting into two passes is essential: when parsing "main" we must already
 * know the signatures of "math/abs" and "sys/io/print"; otherwise the parser
 * cannot distinguish a variable from an alias, or resolve positional
 * parameters. Topological order in pass 2 ensures that in the generated C,
 * every callee is defined before its caller (no forward declarations needed).
 */

ModuleFile G_Files[MAX_MODULE_FILES];
int        G_FileCount = 0;

/* ============================================================================
 * Directory scanning
 * ============================================================================
 *
 * Recursive descent. Skip hidden entries (".", "..", ".git", ".DS_Store", ...).
 * Recurse into directories, register regular files, ignore everything else
 * (sockets, pipes, device nodes — none of which belong in a source tree).
 *
 * Every path operation is checked for truncation: a silently truncated path
 * would point at the wrong file (or a nonexistent one), producing a confusing
 * downstream error. It is much better to fail here with a clear message.
 *
 * Known limitations:
 *   - Symlinks are followed (via stat, not lstat). A symlink loop could cause
 *     infinite recursion; the user's responsibility in this version.
 *   - If two paths resolve to the same physical file (symlinks), both are
 *     registered; the second wins in find_module_file.
 */

static void scan_dir(const char* abs_root, const char* rel_prefix) {
    char dir_path[MAX_PATH_LINK_LEN];
    int n;

    if (rel_prefix[0])
        n = snprintf(dir_path, sizeof(dir_path), "%s/%s", abs_root, rel_prefix);
    else
        n = snprintf(dir_path, sizeof(dir_path), "%s", abs_root);

    if (n < 0 || (size_t)n >= sizeof(dir_path)) {
        fprintf(stderr, "error: path too long: '%s/%s'\n",
                abs_root, rel_prefix);
        exit(1);
    }

    DIR* d = opendir(dir_path);
    if (!d) {
        fprintf(stderr, "error: cannot open directory '%s': %s\n",
                dir_path, strerror(errno));
        exit(1);
    }

    struct dirent* ent;
    while ((ent = readdir(d)) != NULL) {
        /* Skip ".", "..", and any dotfile (".git", ".DS_Store", ...). */
        if (ent->d_name[0] == '.')
            continue;

        char child_abs[MAX_PATH_LINK_LEN];
        char child_rel[MAX_PATH_LEN];

        n = snprintf(child_abs, sizeof(child_abs), "%s/%s",
                     dir_path, ent->d_name);
        if (n < 0 || (size_t)n >= sizeof(child_abs)) {
            fprintf(stderr, "error: path too long: '%s/%s'\n",
                    dir_path, ent->d_name);
            closedir(d);
            exit(1);
        }

        if (rel_prefix[0])
            n = snprintf(child_rel, sizeof(child_rel), "%s/%s",
                         rel_prefix, ent->d_name);
        else
            n = snprintf(child_rel, sizeof(child_rel), "%s", ent->d_name);
        if (n < 0 || (size_t)n >= sizeof(child_rel)) {
            fprintf(stderr, "error: relative path too long: '%s/%s'\n",
                    rel_prefix, ent->d_name);
            closedir(d);
            exit(1);
        }

        struct stat st;
        if (stat(child_abs, &st) != 0) {
            fprintf(stderr, "error: stat '%s': %s\n",
                    child_abs, strerror(errno));
            closedir(d);
            exit(1);
        }

        if (S_ISDIR(st.st_mode)) {
            scan_dir(abs_root, child_rel);
        } else if (S_ISREG(st.st_mode)) {
            if (G_FileCount >= MAX_MODULE_FILES) {
                fprintf(stderr,
                        "error: too many module files under '%s' (max %d)\n",
                        abs_root, MAX_MODULE_FILES);
                closedir(d);
                exit(1);
            }
            /* Copy with explicit bounds; the buffers are sized to match. */
            memcpy(G_Files[G_FileCount].rel_path, child_rel,
                   strlen(child_rel) + 1);
            memcpy(G_Files[G_FileCount].abs_path, child_abs,
                   strlen(child_abs) + 1);
            G_FileCount++;
        }
        /* else: not a regular file; ignore. */
    }
    closedir(d);
}

static ModuleFile* find_module_file(const char* rel_path) {
    for (int i = 0; i < G_FileCount; i++)
        if (strcmp(G_Files[i].rel_path, rel_path) == 0)
            return &G_Files[i];
    return NULL;
}

/* ============================================================================
 * Pass 1: declarations
 * ============================================================================
 *
 * For each file, split into sections and parse params/returns. Only
 * declarations are processed; code bodies are ignored (that is pass 2).
 *
 * The lexer reads directly from the FILE; it must stay open until both
 * sections are parsed. We close it once, after both parse_declaration_section
 * calls.
 *
 * Line numbers: init_lexer receives the section's real start line (from
 * ModuleSections) so that token.line in error messages refers to the actual
 * file line, not to a line relative to the section start. params_line is
 * always 1; returns_line and code_line come from split_module_sections.
 *
 * Note: a duplicate symbol aborts the build inside add_declarative_symbol.
 * By that time, some symbols from this file and earlier files are already in
 * G_Table; that is fine, we exit immediately.
 */

static void pass1_declarations(void) {
    for (int i = 0; i < G_FileCount; i++) {
        FILE* f = fopen(G_Files[i].abs_path, "rb");
        if (!f) {
            fprintf(stderr, "error: cannot open '%s': %s\n",
                    G_Files[i].abs_path, strerror(errno));
            exit(1);
        }

        ModuleSections sec;
        if (!split_module_sections(f, &sec)) {
            /* split_module_sections already printed a detailed diagnostic. */
            fclose(f);
            exit(1);
        }

        Lexer L;

        init_lexer(&L, f, sec.params_start, sec.params_end, sec.params_line);
        parse_declaration_section(&L, G_Files[i].rel_path, SYM_PARAM);

        init_lexer(&L, f, sec.returns_start, sec.returns_end, sec.returns_line);
        parse_declaration_section(&L, G_Files[i].rel_path, SYM_RETURN);

        fclose(f);
    }
}

/* ============================================================================
 * Pass 2: code bodies
 * ============================================================================
 *
 * Read all tokens of the code section into a dynamic array, close the file,
 * then parse. Closing before parsing keeps the FILE resource scoped; if the
 * parser exits on error, we are not leaving an open file.
 *
 * realloc is used with a temporary variable: on failure, the original pointer
 * is still valid and must be freed. The current code frees it and exits; that
 * is the only sane behavior when out of memory.
 *
 * Output order: signature, body, closing brace, blank line. The signature is
 * emitted before parsing so that any emit_* call inside parse_statement lands
 * inside the function. The closing brace is emitted after parsing completes
 * (or after the loop, if parse_statement reaches EOF).
 *
 * Line numbers: init_lexer receives sec.code_line so that token.line in
 * error messages refers to the actual file line.
 */

static void pass2_module_code(const char* rel_path, const char* abs_path) {
    FILE* f = fopen(abs_path, "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open '%s': %s\n",
                abs_path, strerror(errno));
        exit(1);
    }

    ModuleSections sec;
    if (!split_module_sections(f, &sec)) {
        fclose(f);
        exit(1);
    }

    Lexer L;
    init_lexer(&L, f, sec.code_start, sec.code_end, sec.code_line);

    int capacity = 1024;
    int size = 0;
    Token* tokens = malloc(sizeof(Token) * (size_t)capacity);
    if (!tokens) {
        fprintf(stderr, "error: out of memory (token buffer)\n");
        fclose(f);
        exit(1);
    }

    for (;;) {
        Token t = get_next_token(&L);
        if (size == capacity) {
            capacity *= 2;
            Token* tmp = realloc(tokens, sizeof(Token) * (size_t)capacity);
            if (!tmp) {
                free(tokens);
                fclose(f);
                fprintf(stderr, "error: out of memory (token buffer grow)\n");
                exit(1);
            }
            tokens = tmp;
        }
        tokens[size++] = t;
        if (t.type == TOK_EOF || t.type == TOK_ERROR)
            break;
    }
    fclose(f);

    fprintf(G_Out, "void mod_");
    for (const char* p = rel_path; *p; p++)
        fputc(*p == '/' ? '_' : *p, G_Out);
    fprintf(G_Out, "(void) {\n");

    TokenStream ts = { tokens, size, 0 };
    CodeParser P = { &ts, rel_path };
    while (ts_peek(&ts)->type != TOK_EOF && ts_peek(&ts)->type != TOK_ERROR)
        parse_statement(&P, 0);

    fprintf(G_Out, "}\n\n");
    free(tokens);
}

/* ============================================================================
 * Entry point validation
 * ============================================================================
 *
 * Two checks:
 *   1. A source file must exist for the entry path. This is stronger than
 *      "some symbol exists for it" — a module with only locals (no params,
 *      no returns, no aliases) has no symbols at pass 1 time, but is still a
 *      valid entry point.
 *   2. The entry must have no parameters. Entry points are called from main()
 *      without arguments; there is nowhere to pass values.
 *
 * The same check is repeated inside backend_emit_main as defense in depth,
 * but doing it here fails faster and before out.c is opened.
 */

static void validate_entry(const char* entry) {
    if (!find_module_file(entry)) {
        fprintf(stderr,
                "error: entry '%s' has no source file under the project "
                "root\n", entry);
        exit(1);
    }

    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, entry) == 0 && s->kind == SYM_PARAM) {
            fprintf(stderr,
                    "error: entry module '%s' must take no parameters "
                    "(found parameter '%s')\n",
                    entry, s->name);
            exit(1);
        }
    }
}

/* ============================================================================
 * Full pipeline
 * ============================================================================
 *
 * Eleven ordered steps, each with its own failure mode. Ordering matters:
 *
 *   1. Scan                 — populate G_Files.
 *   2. Register files       — ensure every file is a graph node (see note).
 *   3. Register intrinsics  — G_Table now has sys/… entries; must run before
 *                             pass 1 so user redeclarations are caught.
 *   4. Pass 1               — fill G_Table with all declarations.
 *   5. Validate entry       — file exists, no parameters.
 *   6. Topo sort            — build graph from aliases; detect recursion.
 *   7. Open out.c           — only after all passes that can fail are done.
 *   8. Prologue             — storage[] and g_heap_top.
 *   9. Intrinsic bodies     — before any user module so calls resolve.
 *  10. User module bodies   — in topo order; callees precede callers.
 *  11. main()               — calls the entry module; closes out.c.
 *
 * Step 2 (register files) deserves attention: the graph builder creates
 * nodes only from G_Table, but a module with no params, no returns, and no
 * aliases (e.g. a `main` that only uses locals) has no symbols at pass 1
 * time and would be invisible. Registering each file as a node before the
 * graph is built ensures such modules appear in the topo order and get
 * emitted. Without this, such an entry point would compile to a program
 * without a `mod_main` function — a link error at the very end.
 *
 * Error handling: every failure exits immediately with exit(1). Partial
 * out.c is left on disk but should not be used; the user is expected to
 * re-run make after fixing the error.
 */

void compile_project(const char* root, const char* entry, const char* out_path) {
    /* ---- 1. Scan the project tree ------------------------------------ */
    scan_dir(root, "");
    if (G_FileCount == 0) {
        fprintf(stderr, "error: no module files found under '%s'\n", root);
        exit(1);
    }
    printf("Found %d module file(s) under '%s'\n", G_FileCount, root);

    /* ---- 2. Register every file as a graph node ---------------------- */
    /* (See the comment above for why this is necessary.) */
    for (int i = 0; i < G_FileCount; i++)
        ensure_graph_node(G_Files[i].rel_path);

    /* ---- 3. Register intrinsics ------------------------------------- */
    register_intrinsics();

    /* ---- 4. Pass 1: declarations ------------------------------------ */
    pass1_declarations();
    printf("Symbol table: %u name(s), storage[%u] slot(s) used\n",
           (unsigned)G_Table.count,
           (unsigned)G_Table.global_storage_pointer);

    /* ---- 5. Validate entry ------------------------------------------ */
    validate_entry(entry);

    /* ---- 6. Topological sort ---------------------------------------- */
    int order[MAX_MODULES];
    int n_order = build_and_validate_project_graph(order);
    printf("Topological order: %d node(s)\n", n_order);

    /* ---- 7. Open the output file ------------------------------------ */
    G_Out = fopen(out_path, "w");
    if (!G_Out) {
        fprintf(stderr, "error: cannot open '%s' for writing: %s\n",
                out_path, strerror(errno));
        exit(1);
    }

    /* ---- 8. Prologue ------------------------------------------------ */
    uint32_t static_end = G_Table.global_storage_pointer;
    uint32_t heap_words = HEAP_WORDS_DEFAULT;
    uint32_t total_size = static_end + heap_words;
    backend_emit_prologue(G_Out, static_end, total_size);

    /* ---- 9. Intrinsic bodies ---------------------------------------- */
    backend_emit_intrinsics(G_Out);

    /* ---- 10. User module bodies, in topological order ---------------- */
    int user_emitted = 0;
    for (int i = 0; i < n_order; i++) {
        const char* rel = G_Graph_path(order[i]);
        if (!rel) {
            fprintf(stderr,
                    "internal error: graph node %d has null path\n", i);
            exit(1);
        }
        if (is_intrinsic_path(rel))
            continue;

        ModuleFile* mf = find_module_file(rel);
        if (!mf) {
            fprintf(stderr,
                    "internal error: module '%s' in graph has no source "
                    "file (was it removed during compilation?)\n", rel);
            exit(1);
        }
        user_emitted++;
        printf("  [%d] %s\n", user_emitted, rel);
        pass2_module_code(mf->rel_path, mf->abs_path);
    }

    /* ---- 11. main() wrapper ----------------------------------------- */
    backend_emit_main(G_Out, entry);

    if (fclose(G_Out) != 0) {
        fprintf(stderr, "error: failed to close '%s': %s\n",
                out_path, strerror(errno));
        exit(1);
    }
    G_Out = NULL;

    printf("Wrote '%s' (%d user module(s) emitted)\n", out_path, user_emitted);
}