# Constants and instance startup

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

The generated project supplies immutable type, field, source and constant tables.
`runtime/program/` interprets construction data only; it does not dispatch Ama
bodies. `ama_program_start` attaches declaration resolvers, initializes shared
values in the semantic dependency order, then constructs one explicit root.
It creates no callable default instances. Pools remain empty until owned instance
construction actually requires a slot.

Shared values have separate native storage and owned value children. They have
no handle, generation, busy flag or return link. Native payload alignment comes
from the concrete generated type descriptor. A native constructor receives the
original literal bytes, or clones an existing immutable payload, exactly once.
Empty reference fields are representable; shared values cannot acquire mutable
instance references. Constants never execute module bodies.

Owned instance fields are prepared one at a time in declaration order. Finishing
one child makes that private child available to the enclosing construction
operation, without publishing an incomplete root. The private source lookup
requires membership in that operation and a completed lifetime. An earlier child
can therefore initialize a later child by cloning; public lookup still rejects
unpublished identities. Source protection and clone-map remapping reuse the
object runtime. Shared-to-instance copying supplies a complete explicit recipe
for every field, so declaration defaults are not evaluated again.

## Invariants and termination

At each construction boundary, all earlier owned children are complete, the
current child is absent or private, and later fields are uninitialized. The
lifetime prefix counts only completed children. Completing a child advances this
prefix by one; failure cleans the partial child and then the prefix in reverse.
No failed constructor receives a destructor: it must clean its own partial native
resources. Successful native construction adds exactly one destructor obligation.

The constant builder uses explicit parent records; the instance builder uses the
object runtime's work records. Each step visits a field, starts a child or finishes
a value. Finite initializer lists and the semantic rejection of eager ownership
cycles imply termination. Index selection walks the finite resolved access chain
without recursive C calls; at nesting depth d it performs O(d squared) link steps.
Constant construction/cleanup is linear in the constructed value tree excluding
lookup and native-hook costs. Registry lookups are linear in the number of types;
private source authorization is linear in active construction records. Shared
recipe expansion is linear in the copied ownership tree. No guessed layouts or
runtime name interpretation are used.

`ama_program_stop` owns final root cleanup, then reverse constant cleanup, then
pool disposal. On a primary failure it first marks the world terminal so active
service metadata cannot obstruct final disposal. Cleanup keeps the first error
and reports secondary native errors. Callers invoke stop even after failed start;
they must not dispose its pools a second time. Borrowed tables and the program
context outlive all initialized values.

## Evidence and current boundary

[Program tests](../../tests/unit/runtime/program/main.c) cover pool-free shared
initialization, declaration-ordered private copying, references to earlier owned
fields, shared-to-instance cloning without default construction, failure before
root creation, and every allocation failure in startup with exact leak accounting.
[Assembly tests](../../tests/unit/codegen/assembly/main.c) compile and execute the
emitted launcher with real typed native lifecycle hooks and verify destruction
order and literal content. They also compile isolated scaffolding against the
same declarations. Runtime tests run under ASan/UBSan.

These startup tests isolate declarations and lifetime ordering. Separate
[entry preparation](entry-preparation.md), [execution](stackless-execution.md) and
[continuation](continuations.md) suites compose calls, return/yield dispatch and
runtime traces. [Per-context scalar shared storage](shared-storage.md) is now checked through
independent context factories and full failure/cleanup paths. New control/result-tree
execution adapters remain separate tasks; none is inferred from declaration acceptance.
