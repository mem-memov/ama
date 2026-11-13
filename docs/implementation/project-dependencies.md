# Required project dependency closure

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

This implements the source-loading part of DEP-01/DEP-02 and SRC-01 through
SRC-05 in the [technical specification](../../spec/README.md).
The public boundary is [project.h](../../src/project/dependencies/project.h).
It consumes configured roots and one valid logical entry path and produces the
required, source-mapped module graph. It does not instantiate modules or claim
that a syntactically valid graph is a well-typed executable program.

## Domain, ownership and failures

Call `project_init`, then `project_load` exactly once, inspect, then
`project_destroy`. The allocator must remain usable until destruction; the
filesystem must remain stable while indexing roots. Root/exclusion strings and
the entry name are borrowed only during loading. All resulting paths, diagnostic
subjects, source bytes, ASTs and dependency records belong to the project.
Pointers remain stable until destruction. Destroy ASTs before the source catalog;
AST spans borrow its immutable source texts. Destroying a failed partial project
releases the same owners as destroying a successful one.

A successful load sets `valid` and returns `PROJECT_OK`. `project_find` publishes
records only for a valid project. Each record owns one AST and a list of outgoing
dependency occurrences, each retaining its exact requesting span and shared target
record. `PROJECT_MODULE_LOADING` means registered but not yet parsed/scanned;
`PROJECT_MODULE_READY` means that its own syntax and outgoing edges are complete,
not that semantic resolution or every target's syntax has already completed.
Only project success certifies completeness of every required record.

The existing catalog indexes every configured source identity and checks exact
paths, aliases, ambiguity and UTF-8, including optional ABI texts. Its eager source
storage is distinct from required AST parsing. Unused files are not parsed as
active module syntax; unused drafts remain inactive. Catalog failures retain their
original structured diagnostic and conflicting physical paths. A required missing
module, draft or syntax error fails the project and reports the reference chain.
A constants-only source is permitted as a dependency, but cannot be the executable
entry. A draft entry is diagnosed at its source. Missing entry errors own the
supplied entry name even when no source exists.

Allocation failure, including diagnostic-chain storage failure, produces an
allocation-free out-of-memory diagnostic and leaves the project invalid. This
fallback cannot promise a complete dependency trace. Failed partial records are
owned solely for cleanup and diagnostic lifetime, never usable resolved output.
The service writes no files and never invokes native code.

## Responsibilities and exact algorithm

- [Orchestration](../../src/project/dependencies/project.c) builds the catalog,
  registers the entry, drains a growing FIFO registry and publishes success.
- [Registry](../../src/project/dependencies/registry/records.c) compares full
  logical paths byte for byte, returning the existing record or registering one
  new record before its parsing begins. Every dependency occurrence has its own
  edge; occurrences never duplicate the target AST.
- [Required parsing](../../src/project/dependencies/registry/parse.c) rejects
  drafts, preserves parser diagnostics and rejects a constants-only entry.
- [AST discovery](../../src/project/dependencies/discover/ast.c) traverses
  constants, parameters, results and body in that order. An explicit linked work
  list visits statement blocks in lexical depth-first order. Declaration types,
  qualified constant initializer/assignment/input sources and constant-read
  namespaces contribute edges. Call alternatives, branches and loops contribute
  their nested blocks. Variable calls, references, literals and index accesses do
  not invent additional imports. Referenced variable types were already declared.
- [Error reporting](../../src/project/dependencies/report/error.c) retains the
  original syntax diagnostic and adds requesting spans, from immediate requester
  back to the entry. A missing/draft requirement itself is the primary location.

The module registry is also the FIFO queue: appending newly discovered identities
keeps their addresses stable and places them after all previously registered
modules. First-requester links form a tree rooted at the entry. Alternate edges
remain in the graph but cannot replace those links or introduce a trace cycle.
Source order, rather than directory enumeration, chooses the first requester.
For a successful catalog, changing root order does not change identity resolution.

## Laws and correctness argument

Let `R(m)` be the finite ordered sequence of module identities referenced by the
syntax of module `m`, according to the discovery rules above. The desired closure
is the least fixed point of `F(S) = {entry} union union(R(m) for m in S)`.

1. **Injective registry.** Initially the registry is empty. Every insertion first
   compares the complete requested identity against all existing records. A match
   returns that record; otherwise exactly one new record is appended. By induction
   there is at most one record per logical identity, regardless of cycles or diamonds.
2. **Sound registration.** The entry is required explicitly. Every other insertion
   comes from an edge of an already registered module. Induction over insertion
   order therefore places every registered identity in the least fixed point.
3. **Complete syntax traversal.** A pending block points to its next unvisited
   node. Advancing that pointer visits the node once; pushing its syntactically
   owned child blocks schedules every child before the following sibling. Empty
   blocks are skipped. Induction over the finite AST covers every node exactly
   once and extracts precisely the specified edge forms, preserving their spans.
4. **Complete closure.** The FIFO cursor processes each registered record once.
   Every processed record contributes all of `R(m)` before becoming ready. When
   the queue is exhausted successfully, the registered set contains the entry and
   is closed under `R`. Sound registration plus closedness proves equality with
   the least fixed point, not merely agreement on particular examples.
5. **Finite cycles.** An edge to an already loading/ready record makes no insertion.
   Hence any finite module cycle terminates without repeating its parser. This
   establishes loading termination; it says nothing about eager ownership-layout
   cycles, constant-initialization cycles or busy-instance execution.
6. **Trace validity.** Each first requester predates its target's insertion, so
   following requester links strictly decreases insertion rank. Every link carries
   an actual source edge. Thus traces terminate at the entry and do not fabricate
   imports, even when the dependency graph contains cycles.
7. **Failure ownership.** A newly allocated record is linked and its AST initialized
   before parsing. Arena allocations are owned even when not yet linked as edges.
   All error exits preserve these owners. Reverse owner destruction therefore
   releases every partial AST, source and graph allocation exactly once.

## Bounds and remaining implementation scope

After catalog construction, let `C` be catalog size, `V` required modules, `E`
dependency occurrences, `N` visited AST nodes/initializers/arguments, `L` the
maximum logical-path byte length and `B` total referenced-path bytes. The deliberately
simple exact linear registry and catalog lookups cost
`O(N + (E + 1) V L + V C L + B)`, plus source parsing costs. Registry lookup is
`O(V L)`. Graph/work-list storage costs `O(V + E + N + B)`, plus catalog and AST
storage. Traversal frames are arena-owned through destruction, so auxiliary
storage is proportional to visited blocks, not just maximum nesting depth.
Failure traces need `O(V)` storage/time. No traversal uses C recursion proportional
to dependency length or body nesting. Filesystem termination and allocations have
the assumptions of the [catalog](module-paths.md) and [source](source-storage.md)
contracts; representable source/arena sizes bound copied span lengths and products.

This is a source closure, not typed IR. Namespace uniqueness, type/capability
checking, graph-specific legality, mandatory native companion validation,
invalidation, CLI checking and code generation remain subsequent tasks.
Optional companion files are retained by the catalog; no C identifiers are guessed
as further Ama dependencies. In particular, accepting `@C` in loader tests does
not certify a complete native module or runnable project.

## Executable evidence

[Unit/composite tests](../../tests/unit/project/closure/main.c) exercise real
catalog, parser, registry and source-lifetime handoffs: diamonds, self/mutual
cycles, all edge forms and nested blocks, unused invalid syntax/drafts, Unicode
submodules without implicit parent loading, multiple root orders, ambiguous roots,
constants-only entries/dependencies, owned missing-entry diagnostics, draft and
syntax traces, and cleanup after every allocation failure on successful and
failing loads. They compare closure membership against reachability for **all 512
directed graphs on three labelled vertices**, including self edges. They also
load 128 nested blocks and a 128-edge dependency chain without recursive walkers.

[Integration tests](../../tests/integration/project/closure/main.c) load handwritten
UTF-8 project files and compare the complete rendered missing-dependency diagnostic
against a handwritten expectation, including both requester spans.

Verification: `make test` with release flags, `-DNDEBUG`, `-Werror` (leaf assertions
remain enabled); focused unit and integration suites with ASan/UBSan; ordinary
compiler build and independently compiled public header. LeakSanitizer is disabled
in this environment; allocation-injection live-allocation counters independently
check cleanup. These checks certify this source-loading boundary, not subsequent
semantic or execution stages.


T147 adds canonical-rule Doxygen purpose/function/type contracts for every closure
phase. Registry admission precedes dependency parsing; only new records receive a
first requester, so requester links strictly follow earlier discovery and cannot
cycle even when loading edges do. Original-source spans and catalog bytes outlive
all project/semantic consumers. Required-draft/current activation refinements and
new AST dependency sources remain T029 and their syntax consumers; existing
closure evidence does not assert those missing forms are already implemented.

## T018 complete-identity registry composition

The registry's key is the complete catalog spelling, including its explicit root
marker and original component bytes. T016/T017 establish its exact source and
encoded identity domains. `project_load_required` also accepts explicit additional
seeds, so the registry can be verified with multiword Project and Standard names
before the new token/parser consumers are completed in T022/T023. These tests use
minimal `@C` sources and make no claim about executable native ABI validation.

Let `S` be the ordered entry-plus-additional-seed sequence. The closure proof above
generalizes by replacing `{entry}` with `set(S)`; duplicate seeds cannot create
another record because registration compares before insertion. Registry order is
first seed/discovery order, independently of physical directory creation or root
order on a successful catalog. The catalog's own list order is a separate scan
implementation detail. The failure proof includes every allocation of a composed
multiword, two-root closure: no failure enables `project_find`, and all owners are
released by project destruction.

[Complete-identity scenarios](../../tests/unit/project/closure/support/identity.c)
check repeated explicit seeds, distinct root families with identical components,
record/source pointer sharing, exact retained physical paths, both root orders,
missing Standard identities without Project fallback, copied diagnostic names and
every allocation failure. Existing exhaustive three-node graph tests continue to
prove the registry composition against independent finite reachability results;
cycle and diamond tests cover repeated source-discovered edges. Run
`make -C tests/unit/project/closure`.

T018 verifies the existing exact registry algorithm; it adds evidence rather than
an alternative identity cache or parser. Activation/draft classification remains
T023 and required-draft semantics T029. New source path syntax remains T022.
