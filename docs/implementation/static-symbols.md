# Scoped static identities and finite metadata

This component owns exact source-backed symbols and ordinary Type-interface
metadata. Its arena borrows one loaded Project; that Project and its source
manager must outlive every symbol, occurrence, signature and diagnostic.
[TPL-02](../../spec/language/templates.md#tpl-02),
[TPL-07](../../spec/language/templates.md#tpl-07) and
[TPL-12](../../spec/language/static-results.md#tpl-12) govern the target semantics.
[T206](static-source.md) records the source-model transition and its current
limits. Earlier T172 acceptance is preserved in [DONE](../../DONE.md#t172),
not presented as acceptance of the changed static-module model.

## Current operations

Primitive insertion/lookup compares complete borrowed name bytes and exact
ProjectModule identities. No Unicode normalization, filesystem search by static
name, compatible-layout matching or fallback namespace supplies an identity.
Symbol roles distinguish parameters, pair definitions and producer receivers;
only parameters contribute specialization inputs; pair and receipt definitions
are public outputs. Lookup has no source-position argument or visibility cutoff.
Counters are checked before
allocation/publication. Duplicate diagnostics borrow the actual prior and later
sites and allocate no diagnostic storage. Every failure retains destructible
partial arena state and does not certify a completed stage.

The owning AST traversal records expression heads, nested inputs, producer
receivers and qualified constant sources. Foreign labels and ordinary export
selectors do not become local occurrences. It follows runtime declaration,
branch, selected-handler, folded-loop and traversal ownership edges once, never
control/argument aliases. Static graphs are traversed separately with definition
context. A stable source-position sort makes physical ordering independent of
worklist scheduling. Later type evidence cannot change that source tree.

Ordinary input/output placeholder Type lines introduce one local Type formal on
first eligible occurrence. Ordinary shared/body/namespace uses cannot introduce
formals; they reuse the complete interface. Original Type-line order supplies
constructor metadata. Static graphs first collect all pair headers, then first
receipts not declared by a pair. Both roles are public and have whole-module
visibility. Later receipts are equality constraints and retain the original
header/first-receipt AST pointer. Foreign labels and export selectors add no
local name. Remaining local value heads form the exact free-input set.

Graph inputs have Deferred actual metadata, even when forwarded unchanged or
used in applications. Requirements from every use, actual Type/Template
metadata and producer/source-kind admission belong to T208/T213 and later
application/result stages. Scope collection does not default unknown inputs to
Type, infer a closed expected signature, execute a graph or admit a runtime Type.
The current concrete frontend still rejects unresolved graphs and exports.
Explicit foreign-signature trees and ordinary root input-pattern records remain
removed because their source productions no longer exist.

## Finite actual signature operations

Actual constructor metadata retains complete original formal names and category
edges. A Type has no nested constructor signature. Deferred metadata remains a
real obligation; it never becomes Type or equality success merely because no
actual argument has arrived. Ordinary constructor skeletons are prepared from
actual local Type formals; no AST signature builder remains.

The comparison primitive matches complete named sets without using source order.
It compares actual immutable metadata, not minimum use requirements or source
patterns. T208 owns minimum requirements in the full solver. A known mismatch
takes precedence over deferred entries in either direction. The caller supplies
and destroys scratch; input metadata and output remain unchanged on allocation
refusal. The old inventory-mutating wrapper and root-subset mode were removed
with their last source-pattern consumer, leaving one operation used by the real
constraint cache. No duplicate public entry point or unused mode remains.

Comparison memoizes exact ordered signature-pointer pairs; a shared DAG is not
unfolded exponentially. Known finite metadata cycle checking uses explicit DFS
records: an active vertex is exactly an ancestor of the current cursor, so an
edge to it proves a cycle. Completed sharing is accepted. Allocation failure
remains distinct from a semantic cycle. Current tests construct finite metadata
directly and verify reordered names, missing/renamed/nested/category partitions,
pending precedence, 900-level depth, shared DAGs, cycles and each
measured comparison/traversal refusal. They do not recreate removed formal ASTs.

## Invariants and costs

A finite occurrence walk consumes one source owner or removes one pending frame
per transition. Each symbol insertion checks one finite existing-name chain;
Type-formal ordinals count genuine earlier introduction positions. For N
occurrences, S local symbols and B maximum compared name bytes, primitive name
lookup/classification is bounded by O(NSB + S squared). Stable occurrence sorting
uses O(N log N) comparisons without allocation. Owned traversal scratch is linear
in represented owners and expressions, with no semantic depth limit.

For a finite metadata graph with V vertices and E edges, explicit cycle traversal
consumes every edge once. Linear exact-pointer memo lookup gives conservative
O(V(V+E)) time and O(V) scratch. Comparison visits at most P distinct ordered
pairs, using O(P) scratch and O(P squared) linear memo lookup plus exact name-set
membership scans. These bounds require no timeout, hash collision assumption,
first-actual choice or guessed cutoff. Deferred information propagates
monotonically; every resolved slot changes from deferred at most once.

[Current source acceptance](static-source.md) records actual commands,
configurations, results and pending migrations. Per-file registration is in
[the documentation registry](documentation/README.md); it checks real named test
interfaces and headers, and does not replace behavioral acceptance.

## T207 whole-graph scope

Input revision: `46524d5`, after accepted T206 source/schema work. The remaining
local model/events/term/constraint/connection changes still belong to unfinished
T182. No spec edit is authorized. The authoritative scope rules are
[TPL-02](../../spec/language/templates.md#tpl-02),
[TPL-12](../../spec/language/static-results.md#tpl-12) and the native boundary
[TPL-13](../../spec/language/templates.md#tpl-13). Runtime receiver/foreach
classification is outside this task.

### Exact contract

For a static graph, first collect the set P of pair headers, rejecting duplicate
headers. Then visit every producer output receiver in physical source order.
A receiver in P adds equality to its pair; otherwise its first receipt creates
one public alias and later receipts reuse that identity. Let O be the resulting
union of pair and receiver names. Free inputs are precisely eligible local
source/value heads not in O. Foreign binding labels, selectors and path
components are excluded. Neither an earlier use nor work scheduling changes a
binder's role. All input/output names have whole-module visibility.

Ordinary modules collect Type formals only from `@<` and `@>` Type lines, in
that order and textual declaration order. Shared/body/namespace uses can reuse
those names but introduce none. Static graph inputs remain Deferred metadata at
this scope boundary: forwarding alone does not pick Type or Template, and no
actual argument supplies a first-actual default. T208 collects minimum use
requirements separately. Native policy rejects its own ordinary free Type
parameters even if unused; a concrete path/export selector introduces no local
parameter.

Public output accounting includes receipt-defined aliases. Dense internal
ordinals select deterministic records, not positional source bindings. Their
rank follows actual defining-header/first-receipt offsets; an earlier equality
receipt cannot move a pair's definition. Parameter rank follows the specified
first eligible local head. Repeated receipts change neither count nor defining
AST pointer. The obsolete lookup-position argument/alias visibility filter is
removed from the API and its real callers, rather than retained as an ignored
compatibility parameter. Query membership remains exact module plus complete
borrowed name bytes, allocation-free and independent of source position.

### Algorithm and ownership argument

Two finite AST passes establish public binders before the already iterative
occurrence walk/classification. A simple set complement then fixes free inputs.
Exact byte membership costs O(SB) for S stored symbols and maximum name length B;
with N heads/receivers total collection is O(NSB), plus O(S squared) numeric rank
assignment and stable occurrence sorting. Borrowed AST/source edges remain
immutable; each symbol is linked only after count/allocation checks. Count
refusal or any allocation failure leaves partial records owned and validity
false. The metadata solver is not invoked to decide names or definitions.

Module-local collection can precede another module's collection because it uses
no foreign scope. Actual path edges retain the already loaded dependency identity;
category/signature checks still happen after all definition records exist. Static
value closure, producer-versus-value admission and execution remain later tasks.

### Steps and acceptance partitions

| Step | Purpose and actual boundary | Required evidence | State |
|---|---|---|---|
| 1 | Remove source-position lookup and make all aliases public sequence members | Exact/cross-module/Unicode names, queries, role pairs, counters and per-allocation refusals | Complete |
| 2 | Collect pairs before first receipts, preserving definition/equality provenance | Receipts before pairs, forward alias uses, repeated receipts, foreign labels and output snapshots | Complete |
| 3 | Classify ordinary Type lines versus graph set-complement inputs | Input/result/shared/body cases, nested heads, forwarding-only Deferred, independent equal spellings | Complete |
| 4 | Compose parser, inventory, native policy and actual source consumers | Focused parent suites, native admission, scopes/source spans, allocation sweeps and sanitizers | Complete |
| 5 | Deliver current docs/evidence and task-only commit | Per-file mappings, actual headers/links/Doxygen, preserved T182 variants and exact staged bytes | Complete |

The source audit found 22 maintained C/header files mentioning the old lookup
API, none in the untracked connection implementation. Query arity changes are
mechanical; obsolete earlier-alias-invisibility tests are replaced with
whole-module lookup and current graph cases. Existing ordinal consumers already
request parameter/result groups; the result group must include both pair and
receipt definitions. Result-definition consumers already retain the two defining
AST forms separately. Public output publication enumerates the producer's output
sequence and separately records its caller's received aliases, so those are
different environments rather than duplicate producer output slots.

Step 1 checkpoint: removed the obsolete lookup position from 35 declarations/
calls in 22 C/header files using exact identifier/argument token boundaries;
comments/literals and the first two arguments are unchanged. Removed forms were
only constants, source offsets, pure source-length queries and the old parameter
declarations. No untracked connection implementation uses that API. Lookup is
whole-module, alias insertion counts a public output and result-group queries
include both definition roles. Runtime variable visibility APIs are untouched.
The four-case primitive scope leaf passed strict O0 C11/Werror
(`/tmp/ama-t207-scope-first.log`, session 78533 collected), including all role-pair
duplicates, every measured insertion allocation refusal and independent counter
overflow for parameters, pairs and aliases. No job remains active. Next collect
public receivers before free inputs and assign complete public record ordinals.
No ambiguity is currently open.

Steps 2/3 checkpoint: separate receipt collection and index assignment are live
operations under `inventory/`; pair collection completes before either. The old
classifier-side alias introduction is removed. Graph inputs are the exact
source-head complement with Deferred metadata; ordinary inputs remain Type.
The now-impossible parameter-as-receiver evidence branch is removed because
receiver names are excluded from the input set before either is classified.

Strict O0/Werror checks all exited 0: expanded public binder leaf
(`/tmp/ama-t207-public-first.log`, session 79314), six statement permutations /
first-receipt / duplicate-header partitions (`/tmp/ama-t207-public-permutations.log`,
session 88863), nested/free/deferred independent scopes and all graph-input
allocation refusals (`/tmp/ama-t207-classify-first.log`, session 16037), ordinary
shared/result/body/namespace compositions and forbidden introductions
(`/tmp/ama-t207-composition-first.log`, session 99553). All sessions are collected.
No failure or warning was observed. Public output snapshots are exercised through
the actual binding-source interface, including receipt-defined aliases. Source
metadata/producer closure and executable admission are not inferred from these
scope checks. Next verify the task-only tree and dependent consumers, then
complete source documentation and per-file evidence before archival/commit.

### Isolated T207 acceptance checkpoint

`/tmp/ama-t207-check-x5v1a4ta` is HEAD `46524d5` plus 38 task-owned maintained
paths. The shared model header is composed from HEAD plus its whole-module
visibility documentation; the unfinished T182 statement catalog and other local
connection/term/constraint changes are excluded. Nothing in `spec/` changed.

All ten exact commands below exited 0 in both configurations, with no compiler
warning or sanitizer finding (strict session 82458 and sanitizer session 21429
collected):

- `make -C tests/unit/compiler/native-admission -j2`;
- `make -C tests/unit/semantic/static/LEAF -j2`, for `scope`, `explicit`,
  `classify`, `composition`, `events`, `native`, `signature`,
  `constraints/signature`, `bindings/static`.

Strict flags: `C_COMPILER_FLAGS=-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L`. Sanitizer flags use `-O1` instead of `-O0` and add
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`, with
`LINKER_FLAGS=-fsanitize=address,undefined -no-pie` and
`ASAN_OPTIONS=detect_leaks=0`. The established container limitation still excludes
LeakSanitizer; counted teardown independently checks every owner and measured
refusal. Logs are `/tmp/ama-t207-{isolated,sanitized}-*`.
Root `make -j4` also exited 0 without warning (session 45485 collected,
`/tmp/ama-t207-isolated-build.log`). These checks are current scope/caller
acceptance, not whole-sprint conformance or executable template support.

No job remains active at this checkpoint. Step 4 is complete; finish current
per-file/source documentation and final delivery review before archiving T207.
The task-only ready registry has actual new public/classification cases and
excludes T182-only source and test rows. The later algebraic fixture migrations
remain with their named T208–T212/T182 owners; old source forms are not restored.

Initial documentation acceptance also passed: `make docs-complete` checked
2700 ready files with zero debts (session 95261 collected); `make docs` checked
2042 file pages, 7917 interfaces, every 432 canonical destination and 6454 HTML
pages (session 46499 collected). Tool versions remain Doxygen 1.11.0,
Python 3.14.4 and markdown-it-py 3.0.0. The ownership check retains 56 untouched
original dirty-input hashes and the earlier results parent unchanged from HEAD.
Only the model's scope comment and task-owned registry rows are delivered from
shared files; unfinished T182 statements/connections remain outside this task.

Delivery checkpoint: [T207](../../DONE.md#t207) is archived with source/caller
acceptance. The queue has T213 ready next. Generate the final reference on these
exact archived inputs into `/tmp/ama-t207-doxygen-final.log`, collect its exit
status and record actual counts in the commit message. Compare staged bytes to
the isolated checked tree before committing. No compiler/test job remains.
No full static parent/conformance or runtime template support is claimed.
