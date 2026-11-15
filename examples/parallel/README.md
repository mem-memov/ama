# Two isolated actors and an explicit supervisor

This executable example gives a sender and a receiver separate Ama execution
contexts and separate native endpoint owners. Their generated labels run on two
POSIX executor threads. Only independently owned typed message bytes cross the
channel. A failed receiver does not make the sender's context fail.

Run from the repository root:

```sh
make -C tests/unit/codegen/parallel-supervision
```

The test driver uses the compiler's actual source checker, resolved IR, native
bridge producer and C toolchain. It creates a fresh disposable assembly and
compiles [this host](host.c) against its matching generated project interface.
This is a compiler composition example, not an SDK-free prebuilt-library client.
For the supported dynamic C boundary and a native-only worker, run the separate
[asynchronous host example](../async-host/README.md).

Successful execution prints:

```text
parallel supervision accepted
```

Two deliberate receiver failures appear on stderr. They are the inputs to the
two tested policies, and the executable succeeds only after checking both.

## Read the actors and host together

The maintained Ama sources are shared with the channel-wait composition tests:

- [Producer task](../../tests/unit/codegen/channel-waits/project/producer): encode
  independent data, observe Full, register capacity, recheck before suspension,
  then retry or commit explicit cancellation.
- [Receiver task](../../tests/unit/codegen/channel-waits/project/consumer): take
  bytes before decoding, or await native readiness while leaving other work free.
- [Root](../../tests/unit/codegen/channel-waits/project/main): run ready work,
  return to the C host with `%`, and resume only the selected task. Its native
  probes deliberately expose the controlled failure/cancellation scenarios.
- [Host](host.c): transfer ports during serialized setup, start one executor per
  context, coordinate native entry overlap and select a peer policy explicitly.
- [Bridge driver](../../tests/unit/codegen/parallel-supervision/main.c): bind exact
  producer model names, configure probes and check the resulting executable.

The initial setup fills and drains one message, leaving the producer suspended
at a real capacity wait. Two workers then enter independent contexts together.
Their native synchronization proves overlapping execution without sleeps. The
receiver can await the next message; a supervisor condition variable observes
actual dispatch boundaries and rechecks native readiness before sleeping.

## Three scenarios

| Scenario | Receiver | Sender |
| --- | --- | --- |
| Normal | Decodes the next independently owned value | Sends after capacity becomes available |
| Continue peer | Fails technically; cleanup aborts delivery | Completes normally with ordinary aborted-delivery data |
| Cancel peer | Has the same technical failure | Supervisor requests cooperative cancellation; task detaches its wait |

The last two scenarios use the same receiver failure. The different sender
outcomes follow from host policy, not failure contagion. The host joins both
executors before disposing contexts, releasing observer rights or starting the
next scenario. Exact atomic allocation counters verify cleanup after every run.

Factory role configuration is copied into each native probe before threads start.
It never supplies an Ama current instance, pool, resolver or return selector.
While workers run, that setup configuration is immutable. Observed event rights
contain native identity only; callbacks cannot inspect Ama fields or resume code.

The [parallel acceptance contract](../../docs/implementation/parallel-supervision.md)
records the full isolation matrix, native dependency preconditions, separately
compiled/nested component boundaries and process-isolation limits.
