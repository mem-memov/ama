# Stackless execution and native boundaries

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

The emitted launcher has one Ama execution function. Each lowered control
selector has a C label. Branches, loops, calls and early returns use direct label
transfers. Straight-line preparation/replacement nodes are fused into lifecycle
helpers without an intervening Ama transfer. There is no C function per Ama
module, function-pointer Ama dispatcher or saved native activation.

Each active instance stores its caller identity and return selector. A caller
also records its one acquired outgoing target. The busy rule permits at most one
unfinished invocation per instance, so this fixed metadata suffices: nested calls
use different instances' records. Completion restores the caller and selects its
return label; output routing then releases the target. This metadata forms the
explicit return chain, not an implicit C call stack. Instance data stays in its
pool slot throughout execution.

Acquisition-only alternatives store the exact status in the caller's
`last_refusal`. Successful calls do not clear it, and nested calls update their
own caller contexts. `.std.refusal` reads a value snapshot through
`AmaNativeServices` and returns `.std.status` as ordinary data. A nonzero saved
code is not failure of this reader.

Native entry code receives its own native payload, readonly typed parameter
views and writable result views. Generated scopes bind directly to heap storage;
owned child views check availability, while reference/list wrappers do not acquire
referents. Entry scopes bind interface fields only, not unrelated private state.
Views cannot escape. The native operation finishes its C activation before the
next Ama label transfer. Runtime, lifecycle and native helper functions may use
the C stack; they do not preserve unfinished Ama invocations.

Static source maps retain physical UTF-8 filenames, byte columns, logical type
names and operation roles. `#line` directives map emitted operations back to Ama.
Before terminal cleanup, error reporting walks the current instance's return
links and prints the mapped operations. Startup errors use declaration locations
without inventing a root frame. Reporting allocates no new trace buffer and
retains the primary error while cleanup reports secondary failures.

## Invariants and evidence

For every non-host active return link, the caller is waiting and its outgoing
identity equals the callee. An acquired target cannot be entered twice. Induction
on connect/complete/release proves that the current handle identifies the active
instance and that return routing restores its actual caller. Return selectors
are allocated uniquely by the typed plan; the return dispatcher selects among
these explicit continuation labels. A separate resume dispatcher validates saved
positions against the current module before selecting their labels. A completed host root has no pending caller.
Zero is reserved for absent control/source positions; selector exhaustion fails
without wrapping or publishing a partial map. Source maps and return metadata are finite borrowed tables/records; tracing walks
the finite active chain without traversing non-owning object references.

[Execution tests](../../tests/unit/codegen/execution/main.c) cover independent and
nested same-type instances, busy/empty/bounds refusal, per-caller history, retained
reads, persistent native state and pool growth during active calls. Native
payloads assert their own addresses. Unicode paths, constructor failures before
root creation and binding/type diagnostics exercise the real mapping handoff.
A forced allocator that refuses every runtime allocation still produces a mapped
pre-root error. Multi-level native failures report the complete active Ama chain.
The build verifies native entry disappearance in the unstripped project object.

Suspension releases the active caller link and preserves only an explicit saved
position. The [continuation contract](continuations.md) covers recursive copying
and explicit host outcomes. Whole-list execution and retained results are
rejected by semantic checking. Typed list services and [live traversal](list-traversal-execution.md) also use
explicit stable frames and label transfers.

The launcher and [checked C entry/resume operations](c-library-execution.md) now
share the private `ama_project_dispatch` label function. Host wrappers prepare an
explicit root and retain its context lease; ordinary Ama calls remain jumps
inside that dispatcher. Root yield returns normally to the invoking C host.
