# Running the retained-assembly debugger client

T154 supplies a small executable client in `tools/debugger/main.c` and a reusable
command loop in `src/tools/debugger/`. It executes a project only on an explicit
`run` or `resume` command. `inspect` reads actual state at rest through the
[checked runtime API](debugger-inspection.md); it neither runs Ama nor advances a
saved traversal boundary. This is a development client built with a retained
project assembly and its matching SDK, not an attachment to an existing binary
or an external editor/debugger integration.

## Build and use

For a project using the bundled integer module, the following commands build the
ordinary verified executable and a separate debugger executable from the same
retained assembly. Set the repository location explicitly:

```sh
AMA_REPO=/path/to/ama
AMA_SDK="$AMA_REPO/src"
"$AMA_REPO/ama" --emit app .main --out generated --sdk "$AMA_SDK" \
    --standard-root "$AMA_REPO/stdlib/modules" --compile
gcc -std=c11 -O2 -D_POSIX_C_SOURCE=200809L \
    -I "$AMA_SDK" -I generated "$AMA_REPO/tools/debugger/main.c" \
    "$AMA_SDK/tools/debugger/run.c" "$AMA_SDK/tools/debugger/commands/read.c" \
    "$AMA_SDK"/tools/debugger/output/*.c "$AMA_SDK"/tools/output/json/*.c \
    "$AMA_SDK/source/unicode/decode.c" -o ama-debugger
printf 'run\ninspect\nresume\ninspect\nquit\n' |
    ./ama-debugger --responses debug.jsonl
```

The response file must not already exist: the client creates it exclusively and
refuses to truncate existing data. It flushes each response before accepting the
next command. Native stdout remains stdout; native diagnostics and client IO
failures use stderr. This separation keeps ordinary native output out of JSON.
The generic C loop borrows its streams; its host must likewise keep the response
stream separate from native output.

Use the same supported compiler, SDK, include roots, native library dependencies
and applicable C/link flags as the emitted project. External libraries may need
additional linker arguments after the source inputs. The example above needs no
external native library. `--emit --compile` verifies and publishes `generated/program`;
the separate C command builds the development debugger consumer, and is not an
additional compiler-verified published artifact. Do not replace the retained
assembly, generated headers or SDK underneath that consumer. This internal host
boundary is distinct from the [checked compiled-library ABI](c-library-execution.md).
Do not add a second translation unit containing `project.c`: the launcher already
includes it once, and Ama labels remain in the single generated dispatch function.

## Commands and outcomes

Each input line is exactly one of these ASCII spellings:

| Command | Effect |
|---|---|
| `run` | Ask the generated host to enter its root at entry. |
| `resume` | Ask it to resume the root's actual saved position. |
| `inspect` | Acquire a fresh exclusive inspection session, stream state, release it. |
| `dispose` | Explicitly dispose the host; subsequent execution is refused. |
| `quit` | Acknowledge the command, then leave the loop and dispose the host. |

Lines end with LF. An unterminated final line is also accepted. Leading/trailing
spaces, CR, NUL, extra arguments, unknown words and empty lines are invalid. A line
longer than the longest accepted spelling is consumed completely and rejected;
its valid-looking prefix never executes. EOF ends the loop and disposes the host,
including EOF before first execution or after a yield. There is no automatic
resume loop and no implicit recreation after disposal.

Every command produces one JSON object followed by LF. `schema` is
`ama.debugger.v1`; `command`, `data`, and `status` distinguish the requested
operation, its observation and its technical result. Invalid input reports
`command: "invalid"` without a host callback. Invalid commands and refused
operations do not prevent later explicit commands, but make the final process
exit unsuccessful. The generated host decides entry/resume eligibility; the client
does not infer it from displayed activity or translated messages.

Execution data contains `outcome` (`yielded`, `completed`, `failed`) and a
context-qualified `root`. Inspection data contains `context` and `instances`.
A failed admission has null data and a nonzero command status. The snapshot's
program status is separate from the inspection command's status. In particular,
refusing to resume a completed root does not change its stored state, while an
accepted technical failure in the generated host disposes its resources; this
launcher then refuses inspection rather than reconstructing dead instances.

An input/output failure stops the command loop. Already completed execution and
native effects remain completed. Inspection releases its lease even if streaming
fails, and host disposal is still attempted. A partial JSON line is unusable; a
failed write or flush yields an unsuccessful exit. The response file is retained
for diagnosis. Final host status and response-file close failure also produce an
unsuccessful exit. A successful command response does not promise that later
host disposal or file close will succeed.

## What inspection shows

Identifiers, status codes, enum values and source coordinates are decimal JSON
**strings**, preserving values wider than a consumer's floating-point numbers.
References contain context, nominal type, slot and generation; generation zero
means empty, and observation never grants future access without revalidation.
Occurrences additionally contain owner, field, incarnation, record and generation;
they are not interchangeable with target references or current list positions.

Physical source names and status/native bytes use `{utf8, hex}`. Hexadecimal bytes
are always exact; `utf8` is null when those bytes are not valid UTF-8. Declared
names use exact original Unicode text. These representations use the shared
[lossless output codec](source-tool-commands.md), without normalization.

Each live instance includes its original nominal name, lifecycle/activity,
owner, saved resume position and exact source row, recorded operation and its
source row, transient caller/outgoing links, and last-refusal status. Missing
mappings remain null. Saved resume positions identify the **next** operation;
for a yield at the end of a foreach body this is the next collection check, so
its location is the collection header, not the `%` line. The client reports this
existing mapping; it does not guess a nearby line or invent invocation history.

Declared fields include name, role, storage ordinal, interface position, nominal
type and field form, scalar target, list acquisition/count/incarnation, and ordered
occurrences with checked target identities. Scalar `elements` is null. Retained
cursors include binding name, local/active/role flags, numerical boundary,
collection identity, current occurrence, target, installed binding roles/value,
and header/repeat selectors. Inactive locals stay absent. The client enumerates
published instances once; it does not recursively follow potentially cyclic
reference/ownership graphs. Enum values are those in the matching SDK's public
runtime headers; this development protocol does not expose native State offsets.

The default launcher registers **no native readers**. Its native view therefore
has `registered: false`, null value and success status: this means unavailable,
not a decoded zero. A custom host can use `AmaDebugClientHost.begin` to call
`ama_debug_begin` with a matching catalog and explicitly authorized
`AmaDebugNativeReader` array. The reusable renderer accepts the runtime's checked
boolean, canonical sign/magnitude integer with byte order, text or bytes. Readers
run synchronously and own their layout knowledge, encoding guarantees and code
lifetime. An adapter refusal is an inspection result, not a mutation of Ama's
program or last-refusal state. A preview accompanying a nonzero native status
must not be treated as a successful value.

There is no single stepping, source breakpoint, C backtrace, GUI, LSP adapter,
unsaved-buffer synchronization, process attachment or pause of arbitrary native
C. Execution runs synchronously until it completes, yields or fails; a module
that never returns to the host cannot be inspected by this client. Native code
that reads stdin shares the command input stream and needs an explicit separate
input channel in a custom host. Hardware faults/undefined native behavior remain
outside managed Ama reporting.

## Ownership, exactness and evidence

The command recognizer stores at most the maximum accepted spelling and drains
all other bytes through the exact line boundary. Finite lines terminate without
truncation. Its work is linear in consumed bytes and its storage is constant.
Each response nests a fixed number of synchronous visitor layers, each with its
own separator/error state. The client allocates no heap, retains no runtime view,
and follows no reference graph. Native reader selection compares exact nominal
IDs. Rendering costs the emitted byte count plus the finite runtime query costs
and the explicit reader-array scans; the runtime's identity/admission proofs
remain those of the checked inspection component.

A session is acquired only for `inspect`, retained through its callbacks, and
released before flushing or accepting another command. Consequently no command
can execute while the client's inspection lease is live. Every valid loop exit
attempts idempotent host disposal, including read failure, failed rendering,
failed flush, explicit quit and EOF. Streams remain caller-owned. Invalid C API
arguments reject before any host callback. Trusted host callbacks must supply
valid immutable status bytes, matching metadata and truthful native adapters.

`make -C tests/unit/tools/debugger` independently tests every appended byte value
for every accepted spelling, invalid-line draining, unterminated input, real
counted runtime contexts, four authorized native formats, repeat observation,
input failure, output failure after execution, and output failure during inspection
with actual subsequent context disposal. It proves invalid commands execute no
host callback, failed acknowledgement stops later execution, and no inspection
lease or native resource escapes cleanup. It also runs under ASan/UBSan.

`make -C tests/integration/tools/debugger` builds ordinary verified projects and
the shipped client from their retained assemblies, then decodes actual process
responses with Python's independent JSON implementation. It checks two yields,
unchanged repeated snapshots, Russian names, scalar/list/cursor fields, distinct
occurrences, exact next-check source mapping, completion and refused resumption,
invalid commands, explicit disposal, EOF, final unterminated command, preserved
existing response bytes, and accepted native literal failure with dead-state
inspection refused. The generated assembly is compiled as an opaque input, never
parsed as a test oracle. Standalone public/internal header compilation and the
per-file evidence registry cover the maintained implementation and test inputs.
