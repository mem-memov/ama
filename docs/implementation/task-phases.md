# Ordinary task phases

[ASYNC-03](../../spec/language/async-and-parallelism.md#async-03) separates a task's
next scheduling action from its application outcome. This implementation supplies
an ordinary library value, `..task.phase`, and three named constructor modules.
The compiler uses the existing nominal types, calls, retained reads and copies.

## Read and construct a phase

A phase has three owned results of type `..std.condition`, in this order:

| Result | True means |
| --- | --- |
| `ready` | This yielded task can take another step without an event |
| `awaiting` | This yielded task has published its current wait registration |
| `completed` | This task completed its body and its terminal data are available |

Exactly one predicate is true in a value published by the constructors.
Default construction produces Ready. The modules `..task.phase.ready`,
`..task.phase.awaiting` and `..task.phase.completed` have no inputs and return
one `phase` result of this same nominal type. For example:

```text
waiting phase
..task.phase.awaiting
waiting phase !
 > phase
phase
 awaiting >> waiting
waiting
?
 # The phase says that this task is awaiting its already published wait.
```

Retained reads preserve the phase. Ordinary copying produces independent owned
predicates. Explicitly calling a phase value with `!` resets its results to Ready,
following the ordinary result-reset rules. Constructors operate on phase values;
their calls do not enroll, enter or resume the associated task.
Treat the three live predicate instances as observation ports: read or copy them.
Change a phase through a named constructor result or explicit phase reset, so
each publication preserves the one-hot property.

A task producer publishes its phase before every yield or normal completion.
It publishes its visible wait before Awaiting, and retires that wait before
publishing a terminal phase. The constructors do not manufacture a wait.
Task enrollment, native operations and actual event delivery are the following
roadmap tasks; this component establishes their ordinary typed phase boundary.

## Match phase to actual execution

At the checked host boundary, the relation is:

| Actual accepted execution outcome | Valid published phase |
| --- | --- |
| Yielded | Ready, or Awaiting with a published live registration |
| Completed | Completed |
| Failed | Technical failure; the computation cannot be resumed |

This relation is a producer protocol obligation. A host that checks a step compares
its actual execution outcome with the ordinary phase result. An Ama scheduler
uses the declared phase and its own visible registration state. No compiler rule
recognizes arbitrary modules as tasks, inserts enrollment hooks into `!`/`%`,
or converts an unrelated three-condition module to `..task.phase`.

A yielded child returns to its immediate caller. A parent task explicitly copies
or translates the child's phase into its own declared result and executes its own
`%` if it needs to yield. Resuming that parent separately resumes its child through
the saved ordinary call sequence. The tests exercise this exact two-level chain.

Restart deserves care: executable local state persists. The producer explicitly
resets its phase, failure and visible wait state on a fresh entry. The tests first
complete a failing task, then enter it again and require Ready with false failure.
A completed task has no saved position; a further `%` refuses through the usual
runtime boundary.

## Consume terminal data only on completion

A task declares a separate ordinary `..failure` result, with its contained
`description`, when its completion contract needs application failure data.
False failure is compatible with both Ready and Awaiting. It does not authorize
consuming terminal data or treating a yielded task as completed.

The execution tests demonstrate:

1. Ready, false failure, actual yield.
2. Awaiting, a visible wait marker, false failure, actual yield.
3. Completed, a retired wait marker and a true failure containing
   `terminal description`, with a technically completed host outcome.
4. An independent example completes with false failure and empty description.

The wait marker in these phase tests is declared Ama state used to observe the
protocol ordering. Full enrollment identities, registration generations,
submission tokens and native event storage are accepted by subsequent tasks.
Only the completed step's declared completion data may be consumed as the
terminal result, irrespective of default values readable before then.

## Representation and invariants

The observable representation is the tuple `(ready, awaiting, completed)`.
For the three constructor choices, selection produces the corresponding basis
vector of length three. Its entries are Boolean and their sum is one. Invalid
native constructor choices fail before touching the caller's prepared vector or
any target field. The native choices are private C constructor arguments;
Ama callers select an ordinary named constructor module.

`phase+/native/selection.h` computes that vector independently of storage.
`phase.c` publishes the prepared predicates to the exact generated write view.
The operation allocates nothing and invokes no callbacks or Ama code. The native
entry has exclusive synchronous access to the initialized result graph, so no
observer can see an intermediate field update. All predicates are declared Ama
fields and use their ordinary ownership, cloning and finalization.

The phase's native payload is an empty resource-free marker. There is no hidden
task reference, saved position or duplicated native phase discriminator. The
ordinary clone preserves the tuple componentwise; therefore it preserves the
one-hot invariant. Constructors, defaults, copies and explicit reset cover the
library's publication operations. Reading a field does not alter that invariant.
Direct native mutation must obey this same library contract.

Selection and publication use constant time and constant auxiliary storage.
They neither allocate operation records nor acquire scheduler resources.

## Run the evidence

```sh
make -C tests/unit/async/phase
make -C tests/unit/codegen/task-phases
```

The first leaf exhausts all byte choices plus `UINT_MAX`, verifies rejection
leaves output unchanged, and compiles the real native component against generated
SDK declarations. Its independent C consumer checks publication through the real
write view and invalid-choice atomicity without constructing a label program.

The second leaf compiles actual library constructors with generated Ama code.
It checks defaults, retained reads, independent copies, explicit reset, actual
entry/resume outcomes, restart, terminal failure description, successful
completion, nested waiting propagation and refusal after completion. C observer
state contains scalar test observations only; no borrowed view or Ama identity
escapes a call. Both leaves support the repository's ASan/UBSan flags.

Per-file test cases, standalone header commands and purposes are registered in
[the current documentation inventory](documentation/ready.json).
