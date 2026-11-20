# Implementation roadmap

[Specification index](../README.md)

This chapter is an informative sequence of engineering work. It is not a second
definition of Ama and does not claim that any listed stage is implemented. The
language, grammar, and implementation contracts elsewhere in `spec/` determine
the target behavior at every stage.

## Build a coherent implementation in stages

### Stage 0: Freeze backend and native-ABI artifacts

Use the agreed in-place pools, native-state boundary, lifecycle protocol, shared
constants, status representation and host outcomes as fixed inputs. Select the
supported toolchain and write checked C prototypes/adapters for the six bindings
and PROTO-03's fixed `..integer` host-size constructor. Verify native-entry
inlining while allowing internal C helper/library calls.
Specify metadata for ownership, generations, current operation, return links and
per-instance acquisition-refusal snapshots. Separate this logical state from
RT-02's compact physical layouts and state GEN-03's preservation obligations.
Do not expand syntax merely to fill an implementation detail.

Deliverable: an executable native-ABI harness and source-linked contract checklist
covering the settled design, including library refusal-context reading.

### Stage 1: Source, identity, and syntax

Implement strict UTF-8 decoding, pinned XID tables, unambiguous filesystem lookup,
source spans, structural `name+/` submodule mapping, collision-checked hashed
identities, module-wide variable namespaces,
draft exclusion, layout parsing, AST construction, and full target-syntax diagnostics. Add the
fixed line-oriented `.abi` reader and precise malformed-record diagnostics.
Preserve literal bytes. Parse suspension/read forms without implementing runtime behavior.

Deliverable: deterministic parsed projects and isolated parser tests implementing
the specified grammar without implicit numeric interpretation.

### Stage 2: Types, ownership, and explicit IR

Resolve declarations and library contracts, including symbolic `.abi` bindings
and compile-time capability checks. Build owned layouts and interface
layouts; detect eager ownership cycles. Represent lifecycle actions, acquisition,
calls, result routing, and continuations explicitly in typed IR. Implement
exact capability/liveness analysis for any optional storage/check elimination;
retain the general representation whenever its proof obligations are unmet.
Implement operation-scoped clone mappings and internal-reference fixups, including
whole-list boundaries. Check self-identity and ownership ancestry before cloning;
test static and runtime overlap refusal and no-op self-assignment. Verify initialization/copy/
move/destruction failure cleanup before execution integration.

Deliverable: validated typed projects and inspectable layout/control-flow plans.

### Stage 3: Minimal stackless execution slice

Create per-module stable-address block pools, the startup root instance, and
direct typed instance access without default module objects. Generate two modules, each with two instances, and exercise nested same-type
execution, busy refusal, retained results, early return, and restoration of the
current-instance selection using labels and return selectors. Grow pools during
nested execution and verify that existing slot addresses and caller data remain
unchanged. Invocation and return must not transfer entire instance payloads.
Emit operation/call-site source maps and test active-chain diagnostics before
cleanup, including failures that occur before the root body starts. Implement
host completed/yielded/failed outcomes and test explicit root resume/destruction
and standard-launcher rejection of unhandled root yield.
Use the chosen conforming primitive storage/allocation strategy; do not introduce
C module calls as a temporary production backend.

Deliverable: a small end-to-end program proving the storage/control model.

### Stage 4: Native integration and lifecycle specialization

Generate all C types and native interfaces in one translation unit. Integrate
companion implementations, mandatory entry inlining, lifecycle code,
source mappings, all six symbolic `.abi` contracts, and unsupported-native diagnostics.
Integrate common technical-status reporting and continued cleanup on destroy failure.
Derive owned-copy capabilities and reject unsupported copies before emission.
Verify helper signatures through generated typed C uses. Verify that native helpers
and external library calls remain permitted under the supported toolchain.

Deliverable: independently typed native modules participating in stackless execution.

### Stage 5: Yield, resume, and explicit continuation copy

Lower branch/loop suspension positions, preserve execution state in instances,
separate return links from resume positions, and implement independent continuation
copy across the complete ownership tree. Test recursive position preservation for
`%`, recursive clearing for ordinary copies, and refusal on any busy owned descendant. Enforce output-only resume arguments; integrate the explicit failure-status
protocol without introducing exception syntax.

Deliverable: resumable instances with failure-safe cloning and no copied active chains.

### Stage 6: Lists and complete library surface

Implement typed list-service methods, checked membership records and live
foreach boundaries over contiguous order arrays, plus index/truth
mappings, shared constant sections and constants-only modules, resource native types, and necessary scalar operations.
Implement intrusive pool free lists and safe fully-free-block reclamation,
including sparse-directory interior reclamation when selected. Test
order-preserving list removal, ownership destruction, stable references, busy
refusal, and reclaim/regrow generation safety. Keep optional queue specialization
in libraries. Support application-managed work queues without adding an implicit scheduler.

Use compact local order indices without weakening full membership identities,
and select the documented exact lexical-scan or active-cursor correction strategy.
Construct lengths through the fixed typed bridge. Native text/byte values may
share immutable backing under PROTO-06 while retaining independent Ama values.

Deliverable: documented library contracts and representative complete examples.

### Stage 7: Compiled Ama libraries and C consumers

Implement [EMBED-01 through EMBED-08](c-library.md) using the existing typed
project, single generated C assembly and checked context/host boundary. Resolve
explicit exports and supported public value schemas before emitting a consumer
header; the header exposes opaque identities and independent native values,
not the internal instance representation or the native-authoring interface.
Validate target and ABI compatibility before creating a context. Initialize its
constants and explicitly construct selected instances without automatically
starting their bodies. Preserve entry/restart, output-only resume, retained reads
and the three Ama execution outcomes; API rejection remains a separate result.

Package that assembly as a static or supported shared library. Link it from a
multi-translation-unit C host alongside another generated Ama component, including
components that use the same logical module names. Verify public symbol
isolation and pin executable code until its contexts, callbacks and exported
allocations have been released. Test independent snapshots, especially one
application-failure value containing its condition and description, through
context disposal and injected allocation failures.

Then embed a separately compiled child component inside a native C module of an
outer Ama project. The native payload owns an opaque foreign context as a native
resource; it does not hold a hidden Ama reference into the outer project. Use
independent public C values across the boundary and make that resource
noncopyable unless a genuine independent clone is provided. A child yield returns
normally through the adapter's C activation. The outer module then explicitly
chooses completion or its own yield; later resume starts a fresh native
activation. Exercise child failure supervision separately from deliberate
technical-failure propagation to the parent, and reject reentry into either
already active context.

Deliverable: reproducible standalone C-host and Ama → native C → child Ama
examples, with typed public headers, exact build/link commands and lifecycle,
suspension, failure and unload evidence for every EMBED requirement.

### Stage 8: Explicit asynchronous tasks

Implement the task-phase protocol, visible registrations, ready queues, and
native reactor boundary from ASYNC-01 through ASYNC-12. Test readiness already
present at registration, late and duplicate completions, cancellation races,
restart epochs, detached native-record lifetime, and finite scheduling rounds.
Keep `%` as an immediate-caller return and make nested waiting explicit. Publish
the task library and host interfaces with working complete examples.

Deliverable: a responsive cooperative task system with no borrowed Ama state in
callbacks and no hidden change to ordinary calls.

### Stage 9: Isolated parallel contexts

Parameterize generated execution by an explicit context. Give every context its
own mutable runtime and constant lifecycle. Implement audited native workers and
bounded typed message channels under PAR-01 through PAR-08. Exercise independent
contexts with colliding local identity numbers, channel ordering/backpressure,
close/abort, quiescent executor transfer, and one context failing while another
continues. Use process isolation for unaudited native dependencies.

Deliverable: independently executing contexts communicating through explicit
messages, without sharing mutable Ama instances or checked references.

### Stage 10: Release hardening

Run the acceptance matrix, sanitizer checks, failure injection, reproducibility
checks, diagnostic review, and target-specific stackless verification. Update
README, generated ABI headers, derived documentation and contributor instructions
together. Canonical specification changes still need prior explicit user
authorization; do not modify it to match implementation limitations. Publish
explicit limitations and supported toolchain versions.
Deliver the directory relocation utility and its source-graph, preservation,
rollback, and recovery evidence under TOOL-01 through TOOL-04.

Deliverable: a reproducible build with a conformance report, not merely a working demo.

<a id="stage-11-static-templates-with-explicit-design-boundaries"></a>

### Stage 11: Ordinary templates and separate static modules

The target contract separates ordinary runtime declarations from structural
Type/Template computation. Ordinary input/output declaration expressions introduce
scoped Type operands under [TPL-02](../language/templates.md#tpl-02).
[TPL-15](../language/templates.md#tpl-15) admits named structural applications
and export selections that settle one Type per concrete specialization. Static modules
use the exclusive `@...` graph form under
[TPL-12](../language/static-results.md#tpl-12), with named applications, public
result names and an explicit named-export bridge to ordinary field types.
The template, static-module and grammar chapters define the source forms;
this roadmap does not preserve an alternative older interface.

Recognize a layout-only `@...` body as the nominal static marker in
[TPL-14](../language/static-results.md#tpl-14). Preserve marker Type identity in
static aliases, bindings, constraints and source tools while keeping it outside
all runtime storage and C boundaries. Implement the separately specified
[phantom constructor](../language/phantom-types.md#tpl-16) with marker constraints
and complete specialization identities. Phantom declarations contribute no
runtime storage, interface positions, native ABI fields or lifecycle edges;
ordinary owners retain their own identity and lifecycle.

Plan narrow components in prerequisite order: placeholder/name recognition;
ordinary interface inventory; static graph parsing and whole-graph binder/input
classification; exact Type/Template usage requirements; named constructor
applications, partial residuals and fixed captures; public result definitions
and actual-origin named reads; exact structural equations within static graphs
and ordinary typed constraints within their permitted context; pending
repeated-key comparisons without speculative expansion; complete
static-producer input determination and named export selection;
then validation of definition dependencies, nominal creation paths, ownership,
constants, operations and layout. Preserve ordinary runtime positional calls,
receiver/foreach lexical classification and initialization semantics.

Follow [TPL-11](../language/templates.md#tpl-11) for identical-key closure,
unequal-key rejection, pending comparisons and cache-independent incoming and
outgoing path proofs. Check static result-definition cycles separately; useful
equations alone never prove successful producer validation. Test cold/warm
caches, reordered independent constraints, finite repeated nesting and ordinary
closed recursive reference/list graphs. No arbitrary recursion budget or guessed
unknown equality is permitted. Proven decreasing static recursion and natural
number parameters remain deferred with dependent types.

Resolve dependent shared constants and explicit namespaces using ordinary Type
parameters and complete declaration expressions under TPL-15. Constant-read
namespaces still resolve independently under CONST-03 and accept no inline
binding block. Declaration expressions and typed connections supply only exact
permitted equations; a literal or requested member name cannot choose a static
producer's missing arguments. Concrete native declarations follow the same
expression checking while own free native parameters remain forbidden.

Then implement specialized layouts/emission and source diagnostics, concrete
`@C` validation and ordinary source-package dependency closure under
[TPL-13](../language/templates.md#tpl-13). Reject native definitions with own
free Type parameters; concrete native Types remain valid arguments and
ordinary dependencies. Export ordinary concrete roots or facades under
[EMBED-01](c-library.md#embed-01), never static modules or constructor metadata.

Each component needs its own exact argument, unit cases and real composition
cases. Cover forwarding, returned partial Templates, open named usage
requirements, multiple public outputs, output-based nominal inversion, ordinary
export selectors and all rejected obsolete inline/positional static forms.
Update implementation and conformance claims only with actual acceptance
evidence; specifying the new target does not establish compiler support.

Use ordinary templates for algorithms over current list primitives first.
Moving the list foundation remains a separate deferred boundary requiring
[TPL-09](../language/templates.md#tpl-09), lower-level storage/lifecycle contracts
and evidence preserving LIST-03 through LIST-13.

Deliverable: documented and tested specialization with separate static graphs,
explicit supported boundaries and a current acceptance matrix. Specification
approval alone does not complete any compiler implementation task.

## Keep engineering choices separate from language changes

The canonical contracts are in the [language chapters](../README.md#language-chapters)
and the implementation chapters. An unwritten helper or unchosen block size is
an engineering task; it does not reopen a source-language decision.

- Specify exact algorithms and general mathematical/algebraic arguments under
  ARCH-05/ARCH-06; derive invariant and failure tests under TEST-08.
- Build the nested compiler responsibility tree in ARCH-01 through ARCH-04,
  its independently runnable component/composite test hierarchy, and the
  dependency registry/closure/invalidation pipeline in DEP-01 through DEP-03.
- Vendor/generate pinned Unicode XID tables and record the edition; implement
  strict decoding, locale-independent `name+/` lookup, namespace checks, and the agreed
  draft/active-file boundary with source-located diagnostics.
- Implement the six typed list methods and membership/collection identities in
  LIST-04 and LIST-08 through LIST-13, plus FLOW-05 foreach scope/cursors and
  CONT-04 continuation handling; publish library paths/types and traversal
  examples, including insertion/removal on both sides of live boundaries,
  append/clear, swap repeat/skip behavior, atomic reference replacement versus
  remove/reinsert, suspended/nested cursor correction, the documented O(K) scan
  or O(A) exact registration strategy, potential-based termination bounds,
  busy/refusal cleanup, absent membership guards, and independent
  continuation-copy mappings.
- Generate typed native layouts/entry adapters and exact C prototypes implementing
  the six `.abi` bindings, PROTO-03's fixed size bridge, and LIFE-01 through LIFE-05.
  Fix C declaration spelling, representation includes, alignment checks, and
  byte-view typedefs in the backend.
- Implement per-instance refusal snapshots and a read-only library status reader;
  publish its normal interface without adding special source names or bindings.
- Allocate the common nonzero status codes without collisions; reserve the invalid
  index code and retain `AmaStatus`'s fixed code/message/length representation.
- Implement pool block sizes, generation widths/history storage, clone staging,
  free lists, return dispatch, and operation source maps under the specified invariants.
- Separate shared `@=` declarations/storage from instance layouts and pools;
  recognize complete constants-only files and implement static path-plus-`>>`
  constant reads and qualified constant sources in initializers/assignments/inputs,
  cross-module dependency-ordered startup and reverse-order shared cleanup.
- Implement lifecycle state bookkeeping, partial-construction cleanup, terminal
  destruction-error reporting, and host disposal. Integrate cooperative task
  cancellation and isolated contexts without preemptive cancellation or
  resumable failed execution.
- Verify mandatory native-entry inlining under the supported toolchain and preserve
  allowed internal C calls. Complete unit/integration/sanitizer/negative checks.
- Implement the explicit compiled-library boundary from EMBED-01 through EMBED-08:
  export resolution, compatible consumer headers, independent public values,
  context/instance lifecycle, entry/resume/read, static/shared packaging and code
  lifetime. Test both ordinary multi-file C consumers and a native Ama companion
  owning a separately compiled child context, with explicit suspension/failure
  supervision and no context reentry or hidden cross-component Ama references.
- Keep documentation consistent with this specification and record actual
  acceptance evidence before claiming implementation support. Compiler Doxygen
  explanations link canonical file/rule anchors under ARCH-06; validate source
  and rendered-documentation navigation without modifying canonical inputs.

These tasks may expose a genuine contradictory requirement. Record and fix that
contradiction explicitly; an unwritten function signature or unchosen allocation
size is not by itself a new design question. Do not call a feature complete until
its implementation and acceptance evidence exist.
