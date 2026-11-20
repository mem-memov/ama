# Static-module source parsing and T206 evidence

The current spelling and path handoff is recorded in
[T231](versioned-source.md): static names start with one dot and module paths use
slash roots. Current language rules are in
[the canonical grammar](../../spec/grammar.md#static-template-syntax-and-design-boundaries).
Complete ordinary declaration constructor expressions are a separate T226
implementation gate; the historical simple-selector parser acceptance below
must not be read as acceptance of that newer requirement.

## Historical T206 input and acceptance

The following record describes T206's stated input and checks, not a competing
current language contract. Its tested source forms and later task numbers are
revision-specific; use the canonical specification for current permitted syntax.

## Task and authority

[T206](../../DONE.md#t206) starts at `c28d8a0`, after the actual T203 audit and
T214 removal of the unused interface-preview API. Preserve the remaining local
T182 source and evidence changes. No spec edit is authorized by this task.

Implement the canonical [module/declaration grammar](../../spec/grammar.md#module-files-and-declarations)
and [static syntax](../../spec/grammar.md#static-template-syntax-and-design-boundaries),
[SYN-02](../../spec/language/source-and-names.md#syn-02),
[SYN-03](../../spec/language/source-and-names.md#syn-03),
[SYN-04](../../spec/language/source-and-names.md#syn-04),
[SYN-06](../../spec/language/source-and-names.md#syn-06),
[TPL-12](../../spec/language/static-results.md#tpl-12) and
[COMP-01](../../spec/implementation/compiler.md#comp-01).

## Required shapes and component boundaries

Recognize `@...` as the longest annotation token and an exclusive complete module
kind. Keep its graph in a distinct source-owned AST block, never an ordinary
runtime body. A graph statement is either a public-name/value pair or a concrete
producer path with named inputs followed by at least one named output. Every
nested value has inputs only. Labels and operands are nonempty complete static
names; no static positions, skips or explicit signature-declaration trees remain.

Ordinary sections retain variable-name/type pairs. Their type is a simple path
or placeholder, optionally a concrete path with one named export selector. The
same runtime type shape supplies ordinary constant namespaces. Neither context
accepts an inline static application, projection or standalone static formal.
Initializers and runtime binding positions retain their existing contracts.

The parser decides these roles from the module annotation, complete name/path
spelling and exact physical nesting, never from declarations or type lookup.
Unknown actual producer kinds, export labels, duplicate graph binders and typed
application requirements remain later-stage checks. Preserve the concrete
semantic boundary's refusal of unresolved static syntax until T184 replaces it;
recognizing a static graph must not silently create an executable module Type.

Separate shared head/ordinary-type parsing, graph orchestration, named binding
construction and iterative expression nesting. Delete the old formal parser and
its AST-only signature builder/branches when their source form is removed; keep
actual metadata equality and ordinary runtime binding operations for their real
consumers. Old source-only acceptance cases are removed or replaced, not adapted
through a hidden legacy parser or a source-rewriting test wrapper.

## Parser invariant and progress

One AST arena owns every partial node and pending frame; spans borrow unchanged
source bytes. A value frame retains its physical head depth and exact owning
input, and accepts inputs only. Only a producer root admits the output phase,
after which no input can occur. Each transition consumes a physical code line or
closes a finite pending frame. Thus finite source terminates without C recursion
or a semantic depth budget. Scope/kind solving cannot change the resulting tree.

An actual head opens its grammar-specified argument/initializer region, including
empty layout. A deeper comment cannot invent another region. A parent-depth
comment closes the child where grammar permits; subsequent child code cannot
reopen it. Blank lines add no nesting or section boundary. Preserve exact layout,
source spans, final virtual newline and untouched literal payloads.

## Planned steps and checks

| Step | Intended result | Check | State |
|---|---|---|---|
| 1 | Shared token/activation, exclusive module kind and graph AST ownership | Lexer/module cases, drafts/mixed markers, empty graph and failure teardown | Complete |
| 2 | Named graph/value parser and ordinary simple types/selectors | Inputs-only nested values, named producer outputs, ordinary declarations/constant reads, malformed forms and strict comments | Complete |
| 3 | Remove obsolete syntax/model consumers and update real handoffs | No formal AST/parser/builder or static positional source flags; source dependency/native/concrete admission remain safe | Complete |
| 4 | Replace obsolete tests and verify source ownership | Current-source positive/negative/boundary/deep/refusal cases; actual component and parent commands, sanitizers and root build | Complete |
| 5 | Document and deliver | Per-file named evidence, standalone headers, links and final Doxygen on task-only inputs; archive and commit | Complete |

Actual source review confirms that the old formal AST is referenced by the
inventory, explicit signature builder, semantic guard and lowering switch. Those
are mechanical source-schema consumers, not permission to retain an abandoned
source form. The actual shared lexer also contains a bare-three-dot binding join
which must disappear. Later T207/T208/T209/T211/T212 still own full scope,
minimum-use metadata, application, result closure and ready-export semantics.

## First component checkpoint (completed)

Steps 1 and 2 are implemented. `make -C tests/unit/lexer/tokens -j4` passed.
`make -C tests/unit/parser/module -j4` initially failed to compile a new test
because `<stdio.h>` was missing; after adding the direct include, the exact rerun
passed. `make -C tests/unit/parser/static -j4` ran all ten explicit current-source
cases and its successful automatic cleanup completed. The log contains no
compiler warning or failed assertion. No job from these three runs is active.
These are component results, not completed T206 acceptance.

The source tests now cover exclusive empty graphs, named multiple outputs,
partial values, input-only nested applications, ordinary selectors/initializers,
constant namespaces, rejected obsolete forms, strict comment ownership, 1024
nested applications and every measured allocation refusal. The old formal parser
and expression/interface test files have been deleted. Step 3 then removed
the obsolete formal AST/signature builder and static position/skip fields, update
actual source-schema consumers and replace or delete their old-only tests.
Remaining T182 input changes retain their owner; no specification file changed.

### Caller and removal review

The old formal AST, parser, explicit-signature builder, root-interface pattern
records and their exact-signature declaration comparisons have been removed.
Those source shapes have no producer under the current grammar. Static bindings
now contain neither positional nor skip flags; ordinary runtime requests retain
both behaviors. Their source-only tests were removed. Independent finite
metadata equality, pending/mismatch precedence, DAG pair
memoization, deep comparison and allocation refusal remain tested directly,
without manufacturing old ASTs. Constructor slot preparation still serves real
ordinary Type interfaces.

Dependency discovery now visits the separate graph. Closed pair collection,
ordinary Type classification, occurrence collection and native-policy fixtures
use current source forms. The concrete inventory explicitly refuses all static
module kinds (even empty graphs) and selected exports until T184/T212 admission;
the lowering boundary refuses the two remaining graph node kinds. No graph is
silently translated as an empty executable Type.

Exploratory `make -C tests/unit/semantic/static -k -j4` exited 2: the old higher
application/guard/result/constant fixtures still contain source forms removed by
T202. This is not accepted conformance or a passing regression. Their actual
algebraic behavior needs preservation with current fixtures at the owning
T208/T209/T210/T211/T212 steps; their source-model-specific cases must disappear.
The current syntax task does not claim those later stages work. This diagnostic
run also includes the inherited uncommitted T182 suites and cannot certify a
standalone T206 tree. No legacy parser/source rewriting shim was introduced.

The first compiler build exited 2 because removal of the formal expectation left
one obsolete text in the cardinality-checked table. Removed that text and the
rerun passed. Its sole O2 warning is the already recorded inherited T182 typed
scope warning; the isolated task-only tree must independently build cleanly.
The first parser-parent run exposed the same table mismatch. Its corrected
rerun then exposed stale specification-example ordinals/contexts. Updated the
explicit example manifest (including the explicitly excluded native-context
architecture diagram), and the composition leaf passed. The examples are read
byte-for-byte from the canonical chapters; no specification file was edited.

`make -C tests/unit/project -k -j4` and
`make -C tests/unit/compiler/native-admission -j4` both exited 0.
The expanded events leaf also exited 0. The five-leaf semantic focused run
(signature, public pairs, ordinary classification, composition, native policy)
exited 0 for every leaf. Its attempted `CFLAGS=...` override did not select the
project's `C_COMPILER_FLAGS`; these are default-flag runs, not strict acceptance.
It exposed an unused helper warning in the classifier test; a real original-span
lookup assertion now uses that helper. Strict/sanitizer isolated acceptance is
still pending. All logged jobs through that run have completed.

### Isolated functional acceptance

The task-only tree `/tmp/ama-t206-check-wtkya338` starts at `c28d8a0` and contains
only this task's source changes. Mixed events/model files are composed from HEAD
plus the graph/schema edits; inherited T182 statement catalogs, term/constraint
changes and connections are excluded. Its registry likewise excludes inherited
T182-only sources and cases. All 11 disjoint commands completed with exit 0 under
both configurations below; none emitted a compiler warning or sanitizer finding:

- `make -C tests/unit/lexer/tokens -j2`;
- `make -C tests/unit/parser -j2` (the complete recursive parser parent);
- `make -C tests/unit/project -j2` (the complete recursive project parent);
- `make -C tests/unit/compiler/native-admission -j2`;
- `make -C tests/unit/semantic/static/LEAF -j2`, for each of `signature`,
  `explicit`, `classify`, `composition`, `events`, `native`, `bindings/static`.

Strict configuration: `C_COMPILER_FLAGS=-O0 -g -Wall -Wextra -Werror -std=c11
-pedantic -D_POSIX_C_SOURCE=200809L`. Sanitizer configuration replaces `-O0` with
`-O1` and adds `-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`,
with `LINKER_FLAGS=-fsanitize=address,undefined -no-pie` and
`ASAN_OPTIONS=detect_leaks=0`. LeakSanitizer is unavailable under the previously
isolated container restriction; counted allocator teardown independently covers
all source/AST/inventory/request owners and measured refusals. It is not reported
as a LeakSanitizer pass. `make -j4` in the same tree also exited 0 with no warning.
Logs use `/tmp/ama-t206-isolated-*` and `/tmp/ama-t206-sanitized-*`.

Current named-request cases prepare real graph ASTs and supply explicit scoped
Type metadata through symbol/interface primitives, isolating normalization from
unfinished graph inference. They check named input/output permutations, pending
origin snapshots, nested input-only owners, fixed residuals, duplicate/unknown
labels and each request allocation refusal. The old positional/skip tests and
mixed ordinary/static fixtures in that leaf were deleted. Independent actual
signature cases retain cycle/sharing/refusal coverage after removal of the old
source-only signature composition tests.

Final comment corrections in `sections.h`, semantic static `internal.h` and
static `expression.c` have byte-identical `cc -E -P` output before/after in the
isolated tree; the three recorded SHA-256 values are respectively
`e54a667460c2770c50f67b3f0211d3bb84f1af7fbd60a1b071ddd94347980604`,
`8d8e871f6cda605e51e4ea55856cc86420e4ae3e7b053ef3933134ec2a023e3f`, and
`6def4f379563d03c9988304d8435ea7f1f7bf055a3312015accdd617a3bece68`.
These comment corrections do not change behavior; later substantive cleanup
has its own repeated dependent acceptance below.

All functional jobs were collected before documentation acceptance. The first
`make docs-complete` attempt and its corrected rerun are recorded below.
Navigation and final Doxygen still gate archival and the task commit. Remaining semantic fixture
migrations stay explicit with their existing T207–T212/T182 owners and are not
counted as passing current conformance.

The isolated documentation command initially exited 2 before examining coverage:
`git ls-files` requires a repository and the archive snapshot had no `.git`.
Initialized an empty local Git repository in that temporary tree (no main-tree
index change, no extra commit) and relaunched the same command. The rerun
(session 34969, `/tmp/ama-t206-doc-complete-rerun.log`) completed with exit 0:
2695 ready inputs, zero explicit documentation debts.

For B admitted code bytes, L physical layout lines, E expressions and A binding
edges, parsing costs O(B log R + L + E + A), where R is the fixed Unicode range
count. Each physical line is consumed once; closing pending frames adds at most
one transition per expression. Producer-output existence scans each producer's
own root edge chain once. No traversal multiplies source size by a guessed
nesting budget. AST/layout ownership costs O(L + E + A), with original bytes
borrowed rather than regenerated. Ordinary selectors add one optional bounded
child shape and never consult scope or alter the source cursor retroactively.

Final review added a real known-concrete-Type application refusal to the ordinary/
graph composition leaf. The expanded leaf passed both exact configurations,
exit 0, under `/tmp/ama-t206-final-composition-{strict,sanitized}.log` (session
71727 collected). The namespace declaration comment was corrected without
changing its header's preprocessed bytes. Static request Makefile recipes and
commands are unchanged; its purpose now describes named requests.

The local link/EBNF audit initially found T170's archive link targeting a removed
section in the rewritten current parser contract. Corrected that historical
record to the current contract without changing T170's recorded outcomes.
The complete parent roadmap remains acyclic with 25 active tasks, 78 prerequisite
edges and 37/37 routed acceptance areas before T206 archival. All 214 canonical
requirements and 108 EBNF productions are unchanged.

Current checkpoint: source/caller checks are complete in the isolated tree;
no compiler/test job remains. Documentation completeness passed, and the final
reference/navigation review is active. Higher semantic fixtures that exercise
live algebraic contracts are preserved for migration by their actual owners;
source-only tests of removed machinery were deleted. No full `make test` or
complete static-feature conformance pass is claimed.

### Last-consumer cleanup

The final usage review found two more pieces whose only production consumer was
the removed source-pattern checker: the inventory-mutating signature comparison
wrapper and its root-subset mode. Removed both with their sole-purpose tests.
All actual constraint consumers used exact comparison already. The remaining
arena operation compares complete name sets, recursively, with the same Pending
and known-mismatch precedence. Its memo key is simply an ordered pointer pair;
it no longer stores a mode that every production caller fixed to exact.
For that domain, the old cardinality condition reduces algebraically to unequal
cardinality, and every nested comparison was already exact. Thus accepted
metadata comparison results are preserved while unused API/state disappears.

The old `pending_expression` signature slot was likewise written only by the
removed builder/pattern path and never read after that removal. Deleted the
field, its always-null preparer parameter and the assertion testing that unused
field. The first dependent eight-command strict and sanitizer reruns after that
field cleanup passed, as did root `make` without warning. The subsequent
comparison cleanup requires its own final dependent rerun, including the actual
constraint-signature consumer below; earlier logs are not substituted for it.

The scratch/cache leaf now uses actual ordinary constructor signatures from two
current source files with reordered Type parameters. It checks separate scratch
ownership and every refusal, and calls the real `static_constraint_signature`
consumer to verify reversed-key cache reuse, immutable source metadata and
allocation refusal before either scratch or cache publication. It neither
constructs old signature ASTs nor promotes a fixture to whole-graph acceptance.
The ordinary runtime omission flag in the existing boundary fixture was restored
to its original expression during review; only the removed static field has a
constant false value. Runtime positional behavior remains unchanged.

Initial complete Doxygen generation passed (session 74308): 2695 ready inputs,
2037 file pages, 7904 documented interfaces, all 432 canonical destinations and
6442 total HTML pages, Doxygen 1.11.0/Python 3.14.4/markdown-it-py 3.0.0.
This describes its own pre-final input; delivery regenerates after the final
comparison changes and archival. No generated page is committed manually.

### Source-task acceptance and delivery checkpoint

The final comparison implementation and real cache consumer passed all nine
explicit dependent commands under both strict O0 and O1 ASan/UBSan configurations
(session 14804 / session 86880 collected, `/tmp/ama-t206-delivery-{strict,sanitized}-*`).
The nine commands are the eight dependent leaves listed earlier plus
`make -C tests/unit/semantic/static/constraints/signature -j2`.
No compiler warning or sanitizer finding occurred. Final root `make -j4` also
exited 0 without warning (session 16650, `/tmp/ama-t206-delivery-build.log`).
The parser/project/token commands from the initial isolated acceptance are
unchanged by this metadata-only cleanup. A final EOF-whitespace correction in
`signature/compare.c` has identical preprocessed bytes before and after.

The input-ownership review confirms 56 untouched original T203 dirty inputs by
SHA-256 and one old results-parent Makefile unchanged from T214 HEAD. Its initial
T203-hash mismatch belongs to T214's already accepted obsolete-leaf removal,
not a T206 edit. Shared model/events/main files are delivered as HEAD-plus-task
variants, leaving inherited T182 statements/connections outside this commit.
The task-only ready registry excludes those unrelated source/test rows.

T206 accepts source recognition, AST ownership, current metadata primitives and
the specified source-schema handoffs. It does not accept whole-graph scope,
actual minimum requirements, static equation closure, concrete exports or
execution. Existing T207–T212/T182 tasks retain those features and their live
algebraic fixture migrations. Obsolete tests solely for removed syntax/API/state
were deleted; no legacy parser, test-only source translator or default-type
fallback remains.

The final queue/archive and maintained inputs are prepared before delivery.
Run `make docs` in the same isolated task-only tree, collecting
`/tmp/ama-t206-doxygen-final.log` and its `.exit` status before committing. The
reference must pass on these exact bytes; the actual final counts and completion
are recorded in the task commit message without editing its input afterward.
Only then commit T206 and continue T207. No compiler/test job remains active.

Final staged review caught a source-location defect before commit: the concrete
static-module refusal used bytes 0..4 even though leading comments/blank lines
are valid. It now uses the actual `sections.graph.marker->code` span. The real
frontend boundary case includes that preamble and an inline marker comment and
asserts the exact marker range. Its strict and ASan/UBSan runs both exited 0
(session 63023 collected, `/tmp/ama-t206-marker-{strict,sanitized}.log`), and the
incremental root build exited 0 without warning. Final reference generation is
refreshed once more for these source/test/record bytes before commit; no older
fingerprint is substituted for them.
