# Native entry and lifecycle integration

T058 checks the C assembly boundary defined by
[STACK-01](../../spec/implementation/compiler.md#stack-01),
[STACK-02](../../spec/implementation/compiler.md#stack-02),
[STACK-03](../../spec/implementation/compiler.md#stack-03),
[PROTO-09](../../spec/implementation/native-interface.md#proto-09) and
[CABI-03](../../spec/implementation/native-interface.md#cabi-03).
The implementation already connects typed native adapters and runtime lifecycle
services; this checkpoint extends compiled failure evidence rather than creating
a second callback dispatcher.

## One checked synchronous native boundary

The [entry emitter](../../src/codegen/control/native.c) opens a native borrow,
binds the exact nominal interface, captures the caller's refusal as data and
calls the qualified entry directly. Its mandatory always-inline declaration is
checked under the supported compiler profile, including unoptimized builds.
After normal C return, the generated code closes the borrow and validates the
technical status before performing Ama result actions. No unfinished native C
activation remains attached to an Ama continuation.

[Protocol adapters](../../src/codegen/emit/native.c) call the exact declared
lifecycle hooks using actual typed payload pointers. Default, literal and clone
are alternative fresh constructors; generated layout does not default-construct
a literal or clone destination first. Destruction belongs to runtime ownership
cleanup. Each callback's completion is checked by its runtime operation; malformed
transport is a technical error, never a success because its code alone is zero.
Ordinary C helpers and recursion inside a native implementation remain permitted.

The finite nominal type and ABI inventories determine all adapters. Missing
capabilities remain absent rather than being guessed from helper names. C checks
handwritten signatures against declarations; toolchain verification rejects
surviving native entries and ordinary Ama entry functions before publication.
The root execution function remains the single Ama label dispatcher.

## Compiled evidence

`make -C tests/unit/codegen/execution` runs
`it_checks_malformed_lifecycle_statuses_in_actual_generated_execution` through
all four payload lifecycle hooks. The test returns the invalid combination
`AMA_OK` with a nonempty message length and no message pointer. Default, literal
and clone failures prevent the probe body; a destruction failure preserves its
already emitted output and makes disposal unsuccessful. Existing cases check
malformed entry/condition status, body technical failure, closure of native
borrows, internal recursive helpers and rejection of noninline/wrong-signature
entries or compiler flags that weaken the required profile.

`make -C tests/unit/codegen/layouts` independently composes actual over-aligned
native C layouts, lifecycle counters and borrowed views with generated fields.
`make -C tests/unit/codegen/assembly` executes shared-before-root lifetimes and
standalone assembly/interface agreement. Runtime construction, clone and
replacement suites separately exhaust allocation and callback failures; those
checks complement rather than replace the compiled boundary.

Changed test fixture, protocol cases and execution harness retain named per-file
coverage in the documentation inventory. This checkpoint does not claim selected
result-tree, traversal or other later execution adapters; their explicit codegen
rejections remain visible until their roadmap tasks are completed.
