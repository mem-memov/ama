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
supported toolchain, write checked C prototypes/adapters for the six bindings,
and verify native-entry inlining while allowing internal C helper/library calls.
Specify metadata for ownership, generations, current operation, return links and
per-instance acquisition-refusal snapshots. Do not expand syntax merely to fill
an implementation detail.

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
operation-scoped clone mappings and internal-reference fixups, including whole-list
boundaries. Check self-identity and ownership ancestry before cloning; test static
and runtime overlap refusal and no-op self-assignment. Verify initialization/copy/
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
Implement intrusive pool free lists and safe trailing-block reclamation. Test
order-preserving list removal, ownership destruction, stable references, busy
refusal, and reclaim/regrow generation safety. Keep optional queue specialization
in libraries. Support application-managed work queues without adding an implicit scheduler.

Deliverable: documented library contracts and representative complete examples.

### Stage 7: Explicit asynchronous tasks

Implement the task-phase protocol, visible registrations, ready queues, and
native reactor boundary from ASYNC-01 through ASYNC-12. Test readiness already
present at registration, late and duplicate completions, cancellation races,
restart epochs, detached native-record lifetime, and finite scheduling rounds.
Keep `%` as an immediate-caller return and make nested waiting explicit. Publish
the task library and host interfaces with working complete examples.

Deliverable: a responsive cooperative task system with no borrowed Ama state in
callbacks and no hidden change to ordinary calls.

### Stage 8: Isolated parallel contexts

Parameterize generated execution by an explicit context. Give every context its
own mutable runtime and constant lifecycle. Implement audited native workers and
bounded typed message channels under PAR-01 through PAR-08. Exercise independent
contexts with colliding local identity numbers, channel ordering/backpressure,
close/abort, quiescent executor transfer, and one context failing while another
continues. Use process isolation for unaudited native dependencies.

Deliverable: independently executing contexts communicating through explicit
messages, without sharing mutable Ama instances or checked references.

### Stage 9: Release hardening

Run the acceptance matrix, sanitizer checks, failure injection, reproducibility
checks, diagnostic review, and target-specific stackless verification. Update
README, EBNF, ABI documentation, and contributor instructions together. Publish
explicit limitations and supported toolchain versions.
Deliver the directory relocation utility and its source-graph, preservation,
rollback, and recovery evidence under TOOL-01 through TOOL-04.

Deliverable: a reproducible build with a conformance report, not merely a working demo.

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
  remove/reinsert, suspended/nested cursor correction, O(K) lexical-slot
  enumeration, potential-based termination bounds, busy/refusal cleanup,
  absent membership guards, and independent continuation-copy mappings.
- Generate typed native layouts/entry adapters and exact C prototypes implementing
  the six `.abi` bindings and LIFE-01 through LIFE-05. Fix C declaration spelling,
  representation includes, alignment checks, and byte-view typedefs in the backend.
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
- Keep documentation consistent with this specification and record actual
  acceptance evidence before claiming implementation support.

These tasks may expose a genuine contradictory requirement. Record and fix that
contradiction explicitly; an unwritten function signature or unchosen allocation
size is not by itself a new design question. Do not call a feature complete until
its implementation and acceptance evidence exist.
