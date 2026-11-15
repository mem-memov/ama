# Standard-library implementation

The [product specification](../spec/README.md) is authoritative, including the
[grammar](../spec/grammar.md), [library boundary](../spec/language/library.md) and
[native interface](../spec/implementation/native-interface.md). This document
records retained source and executable evidence; it does not define another
library API. [TODO.md](../TODO.md) tracks conformance work.

## Canonical scalar value

Standard-library paths begin with `..`; ordinary project roots begin with `.`.
The maintained [`..integer`](modules/integer) type is signed arbitrary precision.
Its six declared protocols provide default zero, independent clone, destruction,
counted decimal literals, mathematical nonzero truth and checked host-size index
conversion. Shared `zero` and `one` constants have that same nominal type.

Its additional header-declared `from_size` constructor covers every value from
zero through `SIZE_MAX`, without decimal formatting or default-then-overwrite.
[List length](../spec/language/lists.md#list-04) returns this ordinary owned type;
negative/above-host-range integers remain valid values but refuse index conversion.
The [scalar implementation contract](../docs/implementation/scalar-library.md)
explains literal forms, representation, allocation failures, proof and tests.

Configure the library explicitly:

```sh
./ama --check project .main --standard-root stdlib/modules
./ama --emit project .main --standard-root stdlib/modules --out output --sdk src --compile
```

The compiler recognizes validated nominal identities and protocol signatures.
A project `.integer` remains a different type and gains no fixed size bridge.
There is no implicit numeric payload or conversion for unrelated modules.

## Other scalar types

| Public type | Literal range | Truth | Checked index |
|---|---|---|---|
| `..integer` | Optional minus and decimal magnitude, limited by storage | Mathematical nonzero | Exactly `0..SIZE_MAX`; other values refuse |
| `..std.index` | Decimal `0..SIZE_MAX`; overflow is a constructor failure | Nonzero | Stored nonnegative value |
| `..std.condition` | Decimal zero or one | Stored Boolean value | Zero or one |

The bounded types also own shared `zero`/`one` or `false`/`true` constants and
independent scalar clones. They are distinct nominal types. The modules
`..std.index.next`, `..std.index.add` and `..std.index.before` provide checked
successor/addition/comparison probes with ordinary typed interfaces. Bounded
next/add return `result` then one `failure` value; overflow is an expected
application failure, while before returns only its ordinary predicate result.

## Collections and remaining library work

Lists expose six typed intrinsic methods, fixed membership identities and live
mutable traversal. [Execution and continuation copying](../docs/implementation/list-traversal-execution.md)
are implemented and independently tested. Intrinsic methods use typed inputs,
not constants selecting an action or persistent completion fields.

## Application failures and text resources

[`..failure`](modules/failure) is one copyable condition-capable value with a
public owned `description` result of type `..text`. Default construction is
false/empty. Condition conversion reads its discriminator and retained reads
preserve its description. `..failure.make` accepts one text `message` and returns
one present `failure`; it has no separate success flag. Calling a failure value
with `!` explicitly clears it to false/empty.

[`..text`](modules/text) owns counted UTF-8, including embedded NUL. Native
cloning acquires one nonwrapping share of immutable backing; replacement prepares
new bytes before releasing old storage. Different values retain distinct Ama
identities. Empty text allocates no backing, and read-only truth means nonempty.
Allocation/share exhaustion remains a technical error, never application data.

`..std.refusal` returns its immediate caller's last refusal as `..std.status`.
This is read-only ordinary data with the common C status representation; a
nonzero copied status does not make the reader fail. These immutable diagnostic
message views remain separate from the failure value's owned description.
The [failure/resource implementation contract](../docs/implementation/failure-values.md)
explains states, native producer helpers, sharing invariants and failure evidence.
[Arithmetic and list compositions](../docs/implementation/library-compositions.md)
apply this outcome convention. `..integer.add`, `..integer.next` and
`..integer.before` provide exact signed arbitrary-precision operations. They
return ordinary values/predicates; native resource failures remain technical.

## Ordinary task phases

[`..task.phase`](modules/task/phase) returns `ready`, `awaiting`, and `completed`
as three ordinary owned conditions. Exactly one is true in a constructed phase;
the default is Ready. `..task.phase.ready`, `..task.phase.awaiting` and
`..task.phase.completed` construct the same nominal phase type through normal
calls. Retained reads and independent copies preserve its predicates; explicit
`phase !` resets them to Ready. There is no implicit truth conversion for a phase.

A task publishes its phase on every step. Awaiting requires an already published
wait; Completed authorizes consuming its terminal results. Its separate
`..failure` value describes an application outcome. False failure alone does not
mean completion. [The phase contract](../docs/implementation/task-phases.md)
explains actual yields, restart, nested propagation, representation and tests.
Enrollment and native event values are described below.

## Explicit enrollment values

`..task.generation` is an owned nonwrapping native scalar. It defaults to zero;
`!` advances it after checking `SIZE_MAX`. Unsigned decimal literals and independent
copies support initial values and registration snapshots. Truth means nonzero;
zero is reserved for an unissued epoch or wait generation.
`..task.sequence` advances a private persistent generation on each entry and
returns its owned `value` snapshot. Use it when an owner republishes epochs in
result fields: reentering that owner must not reset the private history.
`..task.generation.same` compares its typed `left` and `right` inputs and returns
one ordinary condition. `..task.flag` owns a Boolean membership/waiting value;
`!` sets it from the ordinary `next` condition input (false when omitted), and
truth reads its retained value.

A scheduler declares these values alongside its typed task owners and references.
It preserves counter history throughout the task's lifetime and explicitly
invalidates old registrations on replacement, restart and removal. An ordinary
`!` or `%` does not discover a scheduler. [The enrollment contract](../docs/implementation/task-enrollment.md)
provides the concrete typed ownership graph, complete matching key, exhaustion
proof and real execution tests.

## Native event values and explicit waits

[`..task.events`](modules/task/events) owns a fixed-capacity native completion
service. Its actual `prepare`, `arm`, `post`, `ready`, `take`, `poll` and `wait` result modules
are invoked directly with selected-result calls; the service and methods cannot
be copied. The current source supports explicit manual notification. Native C
providers separately support synchronous or worker-thread completion.

`..task.token` stores complete immutable native identity without callback rights;
`..task.token.same` compares it. `..task.packet` contains a presence condition and
declared token, signed integer code and binary data results. [`..bytes`](modules/bytes)
provides owned immutable binary values, including zero and non-UTF-8 bytes.

The [registration/import contract](../docs/implementation/task-registration.md)
shows how to publish a typed wait before arm, recheck readiness before Awaiting,
import through generated borrows and reconcile retained readiness. Callbacks do
not update Ama fields or resume tasks. [Typed FIFO membership](../docs/implementation/ready-queues.md) and
[bounded rounds/reactor wait](../docs/implementation/scheduling-rounds.md) are
explicitly composed in Ama. `..task.budget.check` validates positive supported
quotas; `..task.require` rejects broken scheduling predicates.

[`..task.operation`](modules/task/operation) owns one native registration acquired
through an explicit `source&`. Its token result is ordinary data; restarting or
destroying the operation retires its previous token before releasing native rights.
It is noncopyable and can be a private owned field of a suspended task. See
[detachment and callback lifetime](../docs/implementation/task-detachment.md).
`..task.operation.detach` explicitly retires delivery at a cooperative step
boundary. `..task.outcome` publishes one Pending/Cancelled/Completed decision;
its terminal states survive later calls. The
[cancellation contract](../docs/implementation/task-cancellation.md) explains the
commit point, wakeup composition and arrival-versus-commit distinction. The [supported asynchronous host](../docs/implementation/asynchronous-host.md)
combines these operations with real native workers and creator-code retention.

`..task.enrollment` is a noncopyable ownership marker for a scheduler record.
Tasks and external references can still be copied, but a record owning this marker
requires fresh construction instead of cloning its enrollment. See
[task copies and failed contexts](../docs/implementation/task-copy-and-failure.md)
for explicit snapshot adoption, one-consumer native results and cleanup before
Failed reaches the host.

## Native construction and ownership

An extensionless Ama interface may select `@C` and have `.h`, `.c` and `.abi`
companions. The six ABI capabilities and their requirements are defined in the
canonical native-interface chapter. [The implementation ABI guide](../VALUE_ABI.md)
and [namespace contract](../docs/implementation/native-namespaces.md) explain the
checked C boundaries and their tests.

Qualify module-owned C names with `AMA_NATIVE(local_name)`; ABI records contain
local identifiers and the generated context supplies their complete identity.
Use unique include guards and contain temporary C macros. `name+/` is physical
submodule storage, while logical identity omits `+`. Generated projects load
transitive dependencies, inline native entry bodies and permit normal C calls
inside native helper code.

Opaque native payloads own native resources. Ordinary persistent Ama relationships
must occupy declared Ama fields, visible to copying and cleanup. Borrowed views
cannot outlive their operation. The separate compiled-component resource exception
is specified by [EMBED-07/08](../spec/implementation/c-library.md#embed-07); that
exception and compiled-library embedding remain open implementation tasks.

## Verification

Run `make -C tests/unit/native/abi/integer` for native integer invariants,
`make -C tests/unit/codegen/scalar-library` and `make -C tests/unit/codegen/library`
for actual standard-root scalar execution,
`make -C tests/integration/standard-integer` for integer CLI behavior,
`make -C tests/unit/runtime/text-values` for text ownership,
`make -C tests/unit/codegen/failure-values` for complete outcomes,
`make -C tests/integration/failure-values` for their CLI example,
`make -C tests/unit/native/abi/integer-arithmetic` for exact arithmetic,
`make -C tests/unit/codegen/library-compositions` for lists/outcomes/constants, and
`make test` for the recursive component/integration tree. Follow
[CONTRIBUTORS.md](../CONTRIBUTORS.md) for decomposition, Doxygen rule links,
per-file tests and failure/lifecycle checks. Use `make docs` for generated file
reference pages. Current acceptance limits remain explicit in the audit and TODO;
no library representation is inferred from a translated diagnostic message.

The [retained library/test/build contract](../docs/implementation/library-tests-and-build.md)
records file-level ownership, independent payload-fragment checks and byte-exact
fixture exceptions. Doxygen generates the actual C interface/file reference.

Runnable sources: [typed Ama queue](../examples/async/README.md) and
[SDK-free worker host](../examples/async-host/README.md). Native submissions retain
independent endpoint rights; `..task.events` keeps its component code resident
until the final callback/buffer acknowledgment.
