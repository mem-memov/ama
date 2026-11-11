# One project C assembly

`ama --emit ROOT .module --out DIR --sdk /path/to/ama/src` uses the same strict
frontend and typed plan as `--check`. It does not execute constructors or invoke
the C compiler. Generation borrows that validated frontend and owns qualified
identities, complete output bytes and structured diagnostics until destruction.
The existing manifest/lock/staging publisher provides explicit output ownership.

The launcher includes `project.h` at file scope and `project.c` inside one execution
function. The header includes the same native declaration files as isolated
scaffolding, concrete instance layouts, shared-value declarations, native companion
bodies, lifecycle hook adapters and descriptor/pool declarations. Native C bodies
include their interface and explicitly establish/clean their body naming context,
as the generated skeleton already does. No Ama module has a C function.

Concrete field offsets, payload size and alignment come from C `offsetof`, `sizeof`
and `_Alignof`, checked by both C assertions and the runtime descriptor initializer.
Internal compiler-generated storage/helper identifiers derive from registered view
identities, disjoint from authored native prefixes. Shared storage uses registered
constant identities and has no slot/generation/control record. All required native
entry/hook declarations and typed uses share the scaffolding's exact nominal types.

The SDK `runtime/assembly.h` includes runtime definitions once in this same unit.
Private SDK helpers have component-qualified names so inclusion cannot collide
across SDK source files. These helpers perform value/lifecycle operations, not
C dispatch of Ama bodies. The project control fragment owns labels for the typed
control graph. Return dispatch and executable semantics are connected by the
subsequent section 8 tasks; the interim fragment fails explicitly instead of
claiming to run a program. No source construct is silently executed as a no-op.

The compilation database contains only the launcher project translation unit,
with SDK/output includes and explicit C target flags; user flags remain separate
JSON arguments. Native modules are not linked as independent translation units.
Edited/unowned output conflicts still require explicit `--overwrite`.

## Correctness and evidence

There is one descriptor field for every planned storage field, in declaration
order, and no shared field in any instance layout. The C compiler supplies physical
layout; the plan's logical slots are never guessed byte offsets. Each native helper
is qualified by its complete module identity and checked through a typed adapter.
All control selectors are unique by the typed plan's monotone selector allocation.
Emission is finite traversal of the plan and artifacts, with linear output size
and conservative quadratic identity/type lookup. Allocation failure retains the
first diagnostic, publishes no artifacts and frees every temporary owner.

[Assembly tests](../../tests/unit/codegen/assembly/main.c) compose the real frontend,
declaration registry, emitter, SDK and native sources. They compile the generated
launcher under Werror, include two modules with identical local native names, use
shared declarations and legal reference cycles, and inject every emission
allocation failure. [CLI tests](../../tests/integration/check/main.c) also prove
that emission/checking do not invoke C on native inputs containing `#error`.
Execution and final native-inlining acceptance are added with their consumers.
