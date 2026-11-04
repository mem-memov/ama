#include "ama.h"

/* ============================================================================
 * Dependency graph and topological sort
 * ============================================================================
 *
 * Every module is a node. An edge A → B means: A's code refers to module B
 * (through an alias in A's params section). Because the language forbids
 * recursion, the graph must be a DAG. The topological sort produces a build
 * order where every callee precedes its caller — required because the C
 * backend emits functions without forward declarations.
 *
 * Three kinds of nodes can exist in the graph:
 *
 *   1. Modules with symbols (params, returns, locals, aliases) — created
 *      from G_Table during the graph build.
 *   2. Modules without any symbols (e.g., a `main` with only local variables)
 *      — created ahead of time by ensure_graph_node() when the driver scans
 *      the project tree.
 *   3. Intrinsics (sys/*) — created from G_Table because register_intrinsics
 *      declares their parameters and returns.
 *
 * A node with no edges still appears in the topological order and is emitted
 * if it has a source file. This covers files with only local variables,
 * whose symbols are created during pass 2 (after the sort).
 *
 * Implementation choices:
 *
 *   - Fixed-size arrays (MAX_MODULES from ama.h). No dynamic allocation.
 *     Exceeding MAX_MODULES is a hard error at the point of overflow.
 *
 *   - Three-color DFS (white / gray / black) for cycle detection. A back
 *     edge to a gray node means a cycle; the error message shows the full
 *     path of the cycle, not just the immediate edge.
 *
 *   - Post-order DFS produces leaves first, callers last. This is the order
 *     in which the driver emits module bodies to out.c.
 *
 *   - visited[] is reset at the start of build_and_validate_project_graph,
 *     so the function can be called more than once (e.g., from tests).
 *
 * The graph is entirely internal to this file: GraphNode, G_Graph, G_NodeCount
 * are static. The driver uses only the three public functions declared in
 * ama.h:
 *
 *   ensure_graph_node()                 — pre-register a file as a node
 *   build_and_validate_project_graph()  — build, validate, return order
 *   G_Graph_path()                      — read a node's path by index
 */

typedef struct {
    char path[MAX_PATH_LEN];
    int  adj[MAX_MODULES];
    int  adj_count;
    int  visited;      /* 0 = white, 1 = gray, 2 = black */
} GraphNode;

static GraphNode G_Graph[MAX_MODULES];
static int       G_NodeCount = 0;

/* Current DFS path — used only to print a readable cycle in error messages.
 * Reset before each topological sort; not part of the public API. */
static int g_path_stack[MAX_MODULES];
static int g_path_top = 0;

/* ============================================================================
 * Node and edge management (internal)
 * ============================================================================ */

/* Find an existing node by path, or create a new one. Exits on overflow
 * (too many nodes or path too long). */
static int get_or_create_node(const char* path) {
    for (int i = 0; i < G_NodeCount; i++)
        if (strcmp(G_Graph[i].path, path) == 0)
            return i;

    if (G_NodeCount >= MAX_MODULES) {
        fprintf(stderr,
                "error: too many modules in dependency graph "
                "(max %d); increase MAX_MODULES in ama.h\n",
                MAX_MODULES);
        exit(1);
    }

    size_t len = strlen(path);
    if (len >= MAX_PATH_LEN) {
        fprintf(stderr,
                "error: module path too long (%zu bytes, max %d): "
                "'%.40s%s'\n",
                len, MAX_PATH_LEN - 1, path,
                (len > 40) ? "..." : "");
        exit(1);
    }

    int idx = G_NodeCount++;
    memcpy(G_Graph[idx].path, path, len + 1);
    G_Graph[idx].adj_count = 0;
    G_Graph[idx].visited   = 0;
    return idx;
}

/* Add a directed edge from → to. Duplicate edges are silently ignored
 * (multiple aliases to the same module are common and harmless). */
static void add_dependency(const char* from, const char* to) {
    int a = get_or_create_node(from);
    int b = get_or_create_node(to);

    for (int i = 0; i < G_Graph[a].adj_count; i++)
        if (G_Graph[a].adj[i] == b)
            return;

    if (G_Graph[a].adj_count >= MAX_MODULES) {
        fprintf(stderr,
                "error: too many dependencies from module '%s' "
                "(max %d)\n",
                G_Graph[a].path, MAX_MODULES);
        exit(1);
    }

    G_Graph[a].adj[G_Graph[a].adj_count++] = b;
}

/* ============================================================================
 * DFS topological sort
 * ============================================================================
 *
 * Three-color algorithm. On detecting a back edge to a gray node, prints the
 * full cycle (from the repeated node around to itself) and exits. The full
 * path is much more useful than just the two endpoints: the user sees exactly
 * which modules form the cycle and in what order.
 *
 * Post-order: callees appear in order[] before callers. The driver emits
 * modules in this order, so no forward declarations are needed in out.c.
 */

static void dfs_toposort(int u, int* order, int* idx) {
    G_Graph[u].visited = 1;                        /* gray */
    g_path_stack[g_path_top++] = u;

    for (int i = 0; i < G_Graph[u].adj_count; i++) {
        int v = G_Graph[u].adj[i];

        if (G_Graph[v].visited == 1) {
            int cycle_start = -1;
            for (int j = 0; j < g_path_top; j++) {
                if (g_path_stack[j] == v) {
                    cycle_start = j;
                    break;
                }
            }
            fprintf(stderr,
                    "error: recursion is forbidden. Cycle detected:\n");
            if (cycle_start >= 0) {
                for (int j = cycle_start; j < g_path_top; j++)
                    fprintf(stderr, "  %s ->\n",
                            G_Graph[g_path_stack[j]].path);
                fprintf(stderr, "  %s (back to start)\n",
                        G_Graph[v].path);
            } else {
                fprintf(stderr, "  %s -> %s\n",
                        G_Graph[u].path, G_Graph[v].path);
            }
            exit(1);
        }

        if (G_Graph[v].visited == 0)
            dfs_toposort(v, order, idx);
    }

    g_path_top--;
    G_Graph[u].visited = 2;                        /* black */
    order[(*idx)++] = u;
}

/* ============================================================================
 * Public API
 * ============================================================================ */

/* Pre-register a module as a graph node. The driver calls this for every
 * scanned file before pass 1, so that files without any symbols still appear
 * in the topological order and get emitted. */
int ensure_graph_node(const char* path) {
    return get_or_create_node(path);
}

/* Build the graph from G_Table, validate it (no cycles), and return the
 * topological order in order_out. Returns the number of nodes written;
 * order_out must have room for at least MAX_MODULES ints.
 *
 * Steps:
 *   1. Reset visited[] and the path stack (repeated calls are safe).
 *   2. Create nodes for every module in G_Table.
 *   3. Create edges from every alias.
 *   4. DFS post-order over all nodes.
 *   5. Copy the order to the caller's buffer.
 */
int build_and_validate_project_graph(int* order_out) {
    /* 1. Reset state. */
    for (int i = 0; i < G_NodeCount; i++)
        G_Graph[i].visited = 0;
    g_path_top = 0;

    /* 2. Nodes for every module in the symbol table. */
    for (uint32_t i = 0; i < G_Table.count; i++)
        get_or_create_node(G_Table.symbols[i].module_path);

    /* 3. Edges from aliases. Normalize path_link (".sys.math.add" →
     *    "sys/math/add") so it matches the module_path format. */
    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (s->kind != SYM_ALIAS || s->path_link[0] == '\0')
            continue;

        char callee[MAX_PATH_LEN];
        normalize_path(s->path_link, callee);
        add_dependency(s->module_path, callee);
    }

    /* 4. DFS post-order over all nodes. */
    int order[MAX_MODULES];
    int idx = 0;

    for (int i = 0; i < G_NodeCount; i++)
        if (G_Graph[i].visited == 0)
            dfs_toposort(i, order, &idx);

    /* 5. Copy to caller's buffer. */
    for (int i = 0; i < idx; i++)
        order_out[i] = order[i];

    return idx;
}

/* Accessor for the driver. Returns the path of the node at index, or NULL
 * if the index is out of range. The returned pointer is stable for the
 * lifetime of the process. */
const char* G_Graph_path(int index) {
    if (index < 0 || index >= G_NodeCount)
        return NULL;
    return G_Graph[index].path;
}