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
`ama_project_execute(&host, AMA_ACCESS_ENTRY)`. Its `AmaProjectResult` distinguishes
`AMA_PROJECT_COMPLETED`, `AMA_PROJECT_YIELDED` and `AMA_PROJECT_FAILED`. Explicit
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
