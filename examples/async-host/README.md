# A native worker and an unloaded Ama component

The host uses the checked C API without compiler SDK headers. Its separately
resident backend owns native callback endpoints and binary bytes. The Ama root
submits work through a typed operation reference, yields to its immediate C host,
and resumes only when the host explicitly calls it again.

From the repository root, choose a new output directory:

```sh
make
python3 tools/examples/async_host/run.py --out /tmp/ama-async-host-demo
```

The builder records every exact command in `commands.json`, compiles the native
backend, invokes `ama --library-build`, and builds the consumer from public ABI
metadata and its matching public header. It does not inspect generated C to guess
names or overwrite an existing destination. `--compiler`, `--cc` and `--sanitize`
select the compiler executable, C compiler and Address/UndefinedBehaviorSanitizer
instrumentation.

The final output is:

```text
asynchronous component lifetime accepted
```

One constructor failure is deliberately reported on stderr. It verifies that a
failed native queue creation releases its newly acquired code credit; the example
still exits successfully when this and every other assertion holds.

Read these parts in order:

1. [The Ama root](project/packages/test/fixture/v1.0.0/src/main) submits through `//task/operation`, then yields.
   Its next explicit resume commits a completed or cancelled ordinary outcome.
2. [The native submit adapter](project/packages/test/fixture/v1.0.0/src/submit.c) borrows the declared operation
   reference only while obtaining independent native endpoint rights.
3. [The backend](backend/backend.c) either completes before submission returns or
   parks a real native worker. It owns no Ama view, reference or payload address.
   The host can release/join that worker after the Ama context has disappeared.
4. [The SDK-free consumer](consumer/host.c) checks pending-context disposal,
   both completion timings, both cancellation/arrival orderings, restart with a
   late old callback, independent exported values, constructor failure and actual
   repeated `dlclose`/`dlopen`. Old native image-domain data never becomes a new
   valid token domain.

`//task/events` acquires one native code credit for its creator component. That
credit follows the service through its method leases, callback endpoints and
owned buffers. Closing a context detaches its waits but does not consume a
callback's independent credit. Consequently `code_can_unload` stays Busy until
the last callback/buffer acknowledges, even after context disposal. Exported
values/diagnostics can independently keep it Busy afterward.

The supported loader excludes API calls and native creator-code frames through
its unload barrier. This host joins workers and waits for release calls to return
before testing quiescence and calling `dlclose`. A counter reaching zero is not a
replacement for that barrier. The backend itself remains resident because the
host links it independently; a separately loaded native provider also owns its
own residency obligation.

The [implementation contract](../../docs/implementation/asynchronous-host.md)
records the native service API, ownership equations and acceptance tests. The
[Ama-only scheduling example](../async/README.md) shows explicit typed FIFO
membership and requests arriving between task steps.
