# Visible task ownership and enrollment

This component implements the ownership and identity boundary of
[ASYNC-04](../../spec/language/async-and-parallelism.md#async-04).
It supplies small ordinary scalar modules and an executable, nominally typed Ama
ownership graph. Native completion storage and event delivery follow in T092–T093.

## A task has an owner as well as references

The [record module](../../tests/unit/codegen/enrollment/project/record) owns its
task in the persistent local field `owned task`. It exposes a checked `task&`
result and separately declared `epoch`, `generation`, `enrolled` and `waiting`
fields. The scheduler owns a list of these records. Removing a record destroys
its task even when old registrations still hold references to that task.

The task is local storage deliberately: entering the record to initialize its
public interface does not reconstruct that local task. Thereafter the enrollment
protocol operates on selected public scalar children and the returned task
reference. It never reenters the record initializer to reset live identity history.
The record's initialization inputs allow boundary-test seeds; normal enrollment
starts both generation values at zero.

A [registration](../../tests/unit/codegen/enrollment/project/registration) contains
an ordinary checked `task&`, an owned epoch snapshot, an owned wait-generation
snapshot and an owned live flag. These are declared Ama fields. Its reference
keeps no task alive. Copying a registration copies its scalar data and reference;
it does not clone the referenced task.

This is one concrete nominal task family. Applications with another task type
provide their corresponding typed record and registration, or maintain explicitly
dispatched queues for several such types, as ASYNC-04 permits. The scalar modules
are reusable; their interfaces contain no universal module pointer or task cast.

## Scalar modules

| Module | Ordinary behavior |
| --- | --- |
| `..task.generation` | Default zero; `!` reserves and publishes its successor; truth means nonzero |
| `..task.generation.same` | Accepts owned `left` and `right` generations and returns one ordinary condition `result` |
| `..task.flag` | Default false; `!` accepts `next` of type `..std.condition`; truth reads the retained Boolean |

A generation represents a value in `0..SIZE_MAX`. It supports independent native
cloning and counted unsigned decimal literals. Zero is reserved for an unissued
epoch or wait generation. Actual counter fields advance only through `!` for the
lifetime of their associated task. Literal initialization and ordinary copies
are useful for initial values and stamp snapshots; callers preserve live counter
history rather than assigning old snapshots back into it.

The representation is a native scalar in its declared Ama field, just like the
library's other scalar values. It contains a number, not an encoded Ama identity.
The flag likewise stores one Boolean in a declared typed value. Neither native
payload stores task references, owner pointers, wait connections or continuations.

For example, after constructing a record and initializing its public task port:

```text
record
 epoch !
 generation !
 enrolled !
  < |1
 waiting !
  < |1
record
 task& >> task&
 epoch >> epoch
 generation >> generation
```

These are ordinary selected child calls and retained copies. Reading the record
does not execute its body; advancing its epoch does not enter its task.

The flag setter validates that its typed native condition input contains zero
or one. Malformed native producer data fail technically before changing the
retained Boolean. Omitting `next` supplies false through normal input defaults.

## Enrollment, replacement and restart

The scheduler's explicit protocol performs these operations without yielding
between the metadata change and its associated control action:

1. **Enroll:** initialize an owned record, advance its epoch, and publish its
   enrolled flag. Reserve a wait generation and publish a registration before
   the corresponding awaiting task step.
2. **Replace a wait:** advance its generation first, detach the former registration,
   then publish the new registration with the current epoch and new generation.
3. **Restart:** advance the epoch first. That immediately makes all former epoch
   stamps ineligible. Detach the previous wait, prepare the next registration,
   and publish the new state before invoking task entry `!`.
4. **Remove enrollment:** clear the enrolled/waiting flags and detach registrations
   before releasing task ownership or allowing reenrollment.
5. **Reenroll the same live task:** preserve the record's counter history and
   advance its epoch again. A newly owned task may start at zero because it has
   a different checked identity.

The [task example](../../tests/unit/codegen/enrollment/project/task) receives a
reference to its already published registration, publishes Awaiting, and yields.
It clears that registration's live flag before publishing Completed. The scheduler
then clears its waiting flag before importing another event. It alone controls
the enrolled task's entry/resume/restart ports. An ordinary caller bypassing that
protocol does not obtain an implicit repair of queues, epochs or registrations.

The context has one executor. Other host operations cannot inspect an intermediate
running protocol update, and retained native callbacks only queue records under
ASYNC-05. The implementation's explicit sequences contain no parent suspension
between reservation, invalidation and publication. Event reconciliation adds the
native token and readiness transaction at the following component boundaries.

## Accept the complete identity

The [validator](../../tests/unit/codegen/enrollment/project/validate) first copies
the registration's declared data. It requires a live stamp and acquires its checked
task reference. An empty, stale or busy target follows the ordinary refusal branch;
no target action executes on failure. A separate completed read obtains the task's
owner reference before accessing that owner's metadata. This avoids trying to
read an ancestor while its task read remains protected.

The current owner must be enrolled and waiting, with nonzero epoch and generation.
Both current numbers must equal the registration's snapshots. Future event import
also matches the native operation identity under ASYNC-05. Task reference identity
and numeric stamps have different jobs: a new task may legitimately have the same
numbers as a destroyed task, but an old checked task reference remains invalid.

The owner association is established by this typed scheduler's initialization and
exclusive control of task entry. It is part of its library protocol; arbitrary
clients do not replace that association or reset a live record's history.
A registration is current only at a coherent scheduler import boundary.

## Why exhaustion cannot revive an old registration

`generation+/native/advance.h` prepares a successor with an inclusive bound `L`.
It accepts exactly `current < L`, then computes `current + 1`. Hence the result is
representable, strictly larger than current and at most `L`. Rejection leaves the
prepared output untouched. The actual entry uses `L = SIZE_MAX` and changes its
scalar only after successful reservation. Exhaustion is an immutable technical
failure, before wait detachment, a new registration or another task step.

For one task lifetime, successful epochs strictly increase. Within each run,
successful wait generations strictly increase; this implementation preserves that
history across runs as well. Therefore a former pair cannot become current again.
After task destruction, checked pool/slot/generation identity prevents reuse from
reviving the former reference. Acceptance requires the conjunction of these laws,
not an operating-system descriptor, native address or numeric stamp alone.

Counter reservation, comparison and flag updates use constant time and space.
Decimal parsing is linear in its counted input. For each accepted digit, the
parser checks `prefix <= (SIZE_MAX - digit) / 10` before multiplication; its local
prefix is therefore the exact represented decimal prefix. A malformed digit or
overflow returns before publishing a replacement scalar.

## Executable evidence

```sh
make -C tests/unit/async/generation
make -C tests/unit/codegen/enrollment
```

The independent leaf exhausts small finite bounds, checks the exact host maximum,
invalid counted literals and output preservation. It also compiles the actual
native scalar with generated ABI declarations and directly checks entry, clone,
truth, literal failure, exhaustion and destruction.

The composition leaf compiles real Ama fixtures and the actual library modules:

- [Restart](../../tests/unit/codegen/enrollment/project/restart) rejects old wait
  generations and old epochs while preserving the same task's worker state.
- [Removal](../../tests/unit/codegen/enrollment/project/removal) removes enrollment,
  reenrolls the same live task, then destroys/recreates its owner. Old references
  fail even when the replacement uses the same numeric stamps.
- [Ordinary calls](../../tests/unit/codegen/enrollment/project/ordinary) leave epoch,
  generation and enrollment unchanged until explicit scheduler operations occur.
- [Epoch exhaustion](<../../tests/unit/codegen/enrollment/project/exhaust epoch>)
  and [wait exhaustion](<../../tests/unit/codegen/enrollment/project/exhaust wait>)
  preserve the prior boundary and prevent a second task step.
- A deliberately malformed native condition exercises the flag's checked
  publication boundary instead of silently accepting an out-of-range Boolean.

The worker fixture is address-sensitive and counts calls, so task reconstruction,
accidental replay or execution after owner removal changes observations. Root
cleanup abandons outstanding continuations and releases owned native workers;
it never resumes them during destruction. Native operation retirement and callback
acknowledgment are subsequent tasks, not properties claimed by these scalar tests.

Per-file purposes, real cases and independent header commands are recorded in
[the documentation inventory](documentation/ready.json).
