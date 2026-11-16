# Retained results and immutable namespace reads

T065 accepts compiled read behavior under
[CALL-03](../../spec/language/calls-and-results.md#call-03),
[CALL-04](../../spec/language/calls-and-results.md#call-04),
[CALL-08](../../spec/language/calls-and-results.md#call-08),
[COPY-03](../../spec/language/values-and-references.md#copy-03),
[CONST-02](../../spec/language/declarations.md#const-02) and
[TYPE-06](../../spec/language/declarations.md#type-06).
The existing typed transfer/runtime paths supply the implementation; this
checkpoint composes them with real source, C compilation and native effects.

## Values and execution positions are separate

An instance read acquires its selected retained interface, skips entry preparation
and body execution, then performs its output actions. An owned result uses the
ordinary clone boundary: declared parameters/results/body fields and owned children
are value data, while saved positions and transient invocation metadata are not
copied by an ordinary transfer. Native data uses the declared clone hook.
A reference result copies identity without acquiring or cloning its referent,
so it does not alter that target's saved position.

A wrapper's owned task executes a native probe once and yields. Copying that task
as a result preserves its native counter, but resuming the copy refuses because
ordinary copying cleared the saved position. A reference result still resumes
the original at its next statement. A later retained read copies the original's
new counter state without restarting its body or resetting its interface. Starting
that copied task increments the copied counter again. These independent effects
distinguish value copying, reference identity and execution position.

The generic ownership mapper follows every declared field descriptor; semantic
parameter/result/body roles do not filter the value boundary. Existing full-key
clone/replacement/forest tests supply the recursive field/reference remapping
proof. [Executable cursors](list-traversal-execution.md) and
[list-service frames](list-service-frames.md) have their own acquisition,
origin and lifetime integration evidence.

## Constants have no default instance

A static namespace read resolves its selected shared declarations and skips
instance acquisition, interface reset and body dispatch. Qualified members use
the same immutable context-owned value storage. Dependency initialization precedes
copies from another constant namespace; no constructor is run at compiler time.

The compiled palette contains both constants and an executable body that owns
an alarm. Its native constructor prints a marker if any accidental default
instance is created. Namespace and qualified-member reads print only the expected
constant values; the alarm never constructs or executes. A constants-only importer
copies the palette's value and exposes it through another inferred receiver.
Descriptors may exist for executable types without allocating an instance.

## Refusal before mutation, failure after committed outputs

A retained read of a busy target refuses before receiver selection/replacement.
Replacing an owned parent destroys its old child and invalidates a saved child
reference. A later read through that stale reference also refuses before changing
an existing receiver. Both cases keep the receiver at 42 and expose the initiating
caller's distinct Busy/stale refusal snapshot.

Another producer mutates a result in its body before being read. Its retained
read transfers that actual stored value, commits its first receiver, then fails
native conversion for the next indexed receiver. Native destructor effects show
the old receiver removed and the new value retained until terminal cleanup. The
producer's probe runs once. Neither interface reset, body reexecution, rollback
nor the read's refusal handler occurs after that technical conversion error.

## Evidence

`make -C tests/unit/codegen/reads` runs five explicit compiled cases. The
independent parent entry is `make -C tests/unit/codegen reads`. The same leaf
instruments actual generated runtime/native code when its compiler flags request
ASan/UBSan; it does not instrument only the assertion runner.

All new maintained leaf/support/Makefile inputs have named per-file coverage.
Existing transfer and construction suites prove individual clone/replacement
cleanup and initialized-prefix behavior; the compiled cases verify the actual
frontend/IR/storage/emission/native boundary. This evidence adds no alternate
read syntax or language requirement. [List services](list-service-frames.md),
[traversal](list-traversal-execution.md) and [asynchronous consumers](asynchronous-host.md)
retain their separate operation and ownership proofs.
