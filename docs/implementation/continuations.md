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
