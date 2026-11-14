# Ordinary call and read bindings

T035 validates normal entry, resume, retained-result and constant-read bindings
under [CALL-01](../../spec/language/calls-and-results.md#call-01),
[CALL-06](../../spec/language/calls-and-results.md#call-06),
[CALL-07](../../spec/language/calls-and-results.md#call-07) and
[CALL-08](../../spec/language/calls-and-results.md#call-08).
[Direct selected-result trees](selected-operations.md) are typed by T036.
[Intrinsic services](list-interfaces.md) are specialized by T037.

## Resolve the interface, then the caller operand

[Target resolution](../../src/semantic/calls/resolve.c) selects an explicit
instance type or a static constant namespace. It creates no default instance.
Reference and indexed instance targets retain their declared exact nominal type.
A shared value cannot be invoked, resumed or used as an instance-result target.
A whole list cannot expose a retired dispatcher/retained interface.

[Field resolution](../../src/semantic/calls/fields.c) selects exactly one role:
parameters for inputs, results for instance outputs and constants for namespace
reads. Names match the base name plus the declared reference/list suffixes;
positional bindings select that role's independent ordinal. A known private or
wrong-role declaration is retained as diagnostic context rather than imported
into the interface. An extra placeholder is still an extra argument.

[Value resolution](../../src/semantic/calls/values.c) interprets supplied sources
and receivers in the initiating caller's namespace. A binding label belongs to
the target interface; a same-spelled operand still belongs to the caller. The
[transfer checker](typed-transfers.md) applies identical nominal/storage rules to
all four output modes. Declared defaults are checked in their declaring module,
not reinterpreted as caller expressions. No default or module body executes during
semantic checking.

An ordinary label may occur only once in its input or output sequence. Distinct
result labels may nevertheless write the same caller receiver. Bindings retain
source order and independent interface positions; later IR uses those separate
facts for text-order preparation and declaration-order installation. The duplicate
binding diagnostic now retains all three necessary locations: the later binding,
the original interface declaration and the earlier binding. No reporting allocation
or repair heuristic is needed.

## Omission is explicit metadata

A positional `<` consumes a parameter position and selects its declared default
at runtime. A positional `>` consumes a result position but allocates no source
or receiver and performs no copy. Both retain `TRANSFER_DEFAULT`; omitted trailing
bindings are allowed without manufacturing extra bindings. Thus an omitted
resource input/output need not support cloning. Supplying an owned resource copy
still requires static copy capability, even if a later runtime alias check could
find identical storage.

A literal input borrows its full original payload, including comment-like bytes
and repeated/trailing spaces. Its type comes from the selected owned scalar
parameter and must support the native literal protocol. No hidden caller variable
or eager default-then-overwrite path is introduced.

The parser rejects invalid mode/style combinations before semantic consumers:
resume inputs, entry `>>`, read `>`, empty read `>>`, bare module-path invocation,
inputs after outputs, named placeholders and mixed named/positional ordinary
bindings. Selected operations have their own interfaces and are not a relaxation
of this consistent ordinary-binding mode.

## Invariants and cost

After each successful binding, the appended prefix contains exactly the parsed
ordinary bindings in source order. Each has one role-correct field and a checked
caller source/receiver, or an explicit omission. Comparing earlier field pointer
identities detects duplicate requests independently of caller receiver aliasing.
A failure leaves the semantic program unresolved and partial records owned for
teardown; no runtime target has been acquired or reset.

For B ordinary bindings and D target declarations, exact field lookup and duplicate
checking cost O(BD + B²) plus name-byte comparison and source/access resolution.
Records use O(B) storage plus their resolved access chains. The algorithm is finite
and allocates no diagnostic fallback. Runtime evaluation order is expressed by
later planning stages, rather than inferred from symbol lookup order.

## Evidence

Run `make -C tests/unit/semantic/calls`. The new leaf checks all 16 source/receiver
form pairs for inputs and each entry/resume/retained output mode, plus compatible
shared output modes. It inspects independent interface positions, explicit
omission metadata, unchanged literal payloads, caller-versus-callee ownership,
repeated caller receivers, suffix/role/arity rejection and noncopyable omissions.

Five duplicate-label operation modes require the declaration and both binding
sites. Parser-mode/style failures must occur before semantic analysis. Each
allocation in a real project containing indexed sources/receivers, literal input,
entry, resume and retained read is independently failed and all owners must
unwind. Existing semantic diagnostic tests additionally check nominal mismatch
and missing literal/clone protocol declaration sites. The recursive semantic
suite includes this leaf explicitly.

[T064 compiled binding acceptance](compiled-bindings.md) observes raw literal
bytes, constructor selection, inferred receiver forms, positional holes and
sequential output progress through actual native code.
