# Suspension and continuation contracts

## Control boundary

`src/codegen/control/emit.c` lowers a yield into a saved selector and a runtime
return. `src/runtime/program/calls/return.c` releases the active return link on
both completion and yield. Completion clears the position; yield preserves it.
A nested target remains acquired only until its caller finishes output transfer,
then becomes available/yielded. The caller remains waiting throughout execution.

A saved selector names the next lowered node, including branch and loop edges.
Dispatch validates membership in the current module's finite set of saved
positions. Each target label appears once, even when branches share a successor.
No C local address, native activation or caller continuation is saved as a resume
position. Return dispatch and resume dispatch have separate domains.

The execution suite compiles and runs repeated suspension, loop suspension,
completion refusal and entry restart scenarios. The standalone launcher rejects
an unhandled root yield rather than introducing an implicit scheduling loop.

## Explicit host boundary

Include `project.h` and `project.c` at C file scope instead of the standard
launcher. Zero-initialize an `AmaProjectHost`, then call
`ama_project_execute(&host, AMA_ACCESS_ENTRY)`. Its `AmaHostResult` distinguishes
`AMA_HOST_COMPLETED`, `AMA_HOST_YIELDED` and `AMA_HOST_FAILED`. Explicit
`AMA_ACCESS_RESUME` consumes the root's saved position; entry restarts and resets
only the interface. Neither operation schedules another host call.

`ama_project_dispose` destroys an available root and its ownership tree without
executing saved positions. Disposal is idempotent; failed execution reports its
active trace and disposes before returning. A disposed host cannot restart.
Keep the host's address stable until disposal: its program borrows its world.
The generated assembly's static type/constant registry permits one live host
at a time; a second host or synchronous reentry is refused with `AMA_BUSY`.
This is not a concurrent C API: external threads must serialize host operations.

All pointers local to the execution function are transient. Between host calls,
only the host context, stable instance pools and instance service metadata remain.
Every successful nested yield returns through the current invocation's own link;
a later resume installs a new caller link, so error traces follow the new caller.

## Ownership-wide copy law

`runtime/metadata/clone.c`, `runtime/objects/clone/prepare.c` and
`runtime/program/transfers/lists.c` implement the same projection for every
vertex of the cloned ownership forest: new identity and resources; remapped
internal references; saved position either zero (ordinary copy) or unchanged
(explicit continuation copy). All remaining service fields start fresh. Thus
mixed completed/yielded children do not change their parent's resumability.

Availability preflight protects the entire owned forest before native cloning.
External reference targets are not traversed. Failed preparation restores source
service records and destroys only successfully constructed destination prefixes.
The existing reference map also spans sibling list elements; position copying
changes no ownership edges and therefore cannot introduce an active return chain.
The clone and transfer suites test both projections, nested list descendants,
reference remapping and allocation-failure cleanup. Exact failure injection visits
every allocation in successful scalar-tree cloning for both copy modes.

## Host and failure evidence

The independently runnable [continuation execution suite](../../tests/unit/codegen/continuations/main.c)
compiles real projects with the verified native toolchain. It exercises both arms
of a branch with a shared saved successor, repeated loop yields, output-only
resume, interface reset on entry, independent continuation copies, and the new
caller in a resumed failure trace. Custom launchers inspect the actual
`AmaHostResult`, preserve root identity across host returns, reject reentry and
second live contexts, dispose yielded ownership trees and reject failed resumes.
Rejected host invocations do not mutate an already running execution. Failed
execution itself reports and disposes before returning an empty-root failure.

The host owns a stable context and is used synchronously. Its live-context lease
covers execution and cleanup; native destruction cannot synchronously reenter
that boundary. Disposal traverses ownership, never saved control edges. Therefore
no pending code runs as a side effect of destroying a yielded instance.

Validation includes the complete project suite with release assertions enforced
by the harness and `-Werror`, plus ASan/UBSan on runtime and compiled continuation
projects. LeakSanitizer is disabled because this traced environment cannot run
it; resource fixtures independently check balanced construction and destruction.
