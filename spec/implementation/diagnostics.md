# Failures, refusals, and execution diagnostics

[Specification index](../README.md)

## Explanation: distinguish normal results, refusal, and failure

A network peer closing a connection can be an expected result. A busy target
can refuse a call before its body begins. An allocation failure is a technical
failure. These outcomes have different control flow and must not be inferred
from translated diagnostic text.

| Outcome | How the caller observes it | Can the current Ama execution continue? |
|---|---|---|
| Expected application failure | One ordinary condition-capable failure value, including its description | Yes, according to the program |
| Refusal to begin, including documented list-method preflight refusal | The operation's own `:` and last-refusal status | Yes, when handled |
| Fatal technical failure | Diagnostic and failed host outcome | No; this context is cleaned up |
| Cooperative task cancellation | The task protocol's ordinary terminal result | Yes, for other tasks |

The task scheduler does not turn `:` into an implicit wait or turn a technical
failure into an isolated task error. Independent execution contexts have their
own failure boundaries under [parallel execution](../language/async-and-parallelism.md).

## Required failure and diagnostic behavior

### Error reporting and execution traces

### Return expected failures as ordinary results

<a id="error-01"></a>

**ERROR-01.** Ama has no language-level exceptions, throw/catch constructs, or
exception propagation through the return chain. Expected application failures
are ordinary declared results that callers inspect explicitly. An operation
whose public library contract reports an expected failure returns one owned
failure-module value for that outcome. The value itself supports the read-only
condition protocol `?` and contains its description in declared Ama storage;
do not split its presence and description into separate operation results or
return an additional success flag for the same outcome.

The failure value has these coherent states:

| State | Condition conversion | Description |
|---|---|---|
| No application failure | False | Empty |
| Application failure occurred | True | Description of this failure |

Default construction creates the no-failure state. Before publishing an outcome,
the operation must establish the complete corresponding value, so a successful
invocation cannot retain a previous failure or its description. A structured
failure code or other typed context may also belong to this module. Its truth
conversion uses the type's declared state, not translated message text, and
must not mutate the value, execute its body, or reset its interface under
PROTO-04. A normal false predicate result, end-of-stream result, or other domain
value is not automatically an application failure; the operation documents
which outcomes are failures.

Expose the description through the failure module's declared result interface
so callers can read it with ordinary retained-result access. A private body
field is not made public by this convention. In Russian-language interfaces and
examples, `провал` names the enclosing operation's result and `описание` names
the failure module's text result; these are ordinary declared names, not reserved
words, compiler-recognized markers, or an implicit result added to every module.
Interfaces in another human language may declare the corresponding names in
that language. The failure type's complete module identity and concrete text
type belong to the documented library interface; the compiler must not infer
capabilities from a result's spelling.

The ordinary type, acquisition, and copy rules apply to the complete failure
value. Copying it preserves both its condition state and description, including
owned text lifetime; it is not a borrow of temporary diagnostic bytes. A native
payload may store its own scalar discriminator, but the description is a declared
owned Ama field/result, not an Ama handle hidden in opaque native memory under
CABI-05. Native `=` copies native data, while generated cloning copies the
description and any other declared fields. The default/literal/clone lifecycle
and preparation/commit guarantees remain unchanged.

**Why use one module.** The condition and its explanation describe one outcome.
Keeping them in one value prevents callers from accidentally copying the flag
without its explanation or pairing a new flag with an old message. It also uses
the ordinary module model: the same object can be tested as a condition, read
through its public interface, copied, stored, or passed to a reporting module.
No exception object, separate Boolean result, or new control-flow syntax is
needed.

**Example.** This is a fragment in a module where `чтение файла` and
`показать ошибку` are already declared. The reader accepts a text input `путь`
and returns the owned failure value `провал`; its public text result is
`описание`. The reporting module accepts that text as its first input. These
interfaces are illustrative, not additional fixed standard-library paths:

```text
чтение файла !
 путь < |данные.txt
 провал > провал чтения

провал чтения
?
 провал чтения
  описание >> текст ошибки
 показать ошибку !
  < текст ошибки
```

If the file is absent and the reader documents that as an expected failure,
the call completes normally with a true failure value. The `?` body reads its
stored description without executing the failure module. If reading succeeds,
the value is false and this body is skipped. The receiver and description copy
use the existing TYPE-06 inference rules. This `?` is an ordinary positive
condition, not a handler attached to the file call. A reporting module may
instead accept the whole failure value and inspect its public interface itself.

An acquisition refusal still selects only the operation's attached `:` under
CALL-04/CALL-05. It publishes no new result, so an old or default failure value
must not be mistaken for the outcome of that refused call. A handler that
continues execution must explicitly separate that refused path from processing
the current invocation's results. Intrinsic list preflight refusals remain
LIST-10 refusals; this convention adds no results to those fixed interfaces.

Native operations
report technical failures through an explicit runtime status, separate from the
module's declared application results. Returning a true application failure value
is a technically successful execution: native entry reports technical success
after constructing the declared result. Inability to construct or copy that
value, including allocation failure while creating its description, follows the
technical-failure path instead of becoming another application failure value.
PROTO-09 and ERROR-03 through ERROR-05
define the common representation and message lifetime. Destruction errors follow
PROTO-10 and do not stop cleanup of other resources.

Application descriptions are ordinary owned text and may be dynamically
constructed. ERROR-04's program-lifetime immutable-message restriction applies
to the separate native `AmaStatus` transport, not to these owned descriptions.
Neither the failure value nor its description introduces a catch mechanism for
technical errors or changes the caller's last-refusal snapshot under ERROR-06.

### End this context on technical failure

<a id="error-02"></a>

**ERROR-02.** An unhandled technical failure, including initialization, allocation,
copy, or native decoding failure, reports its cause and active Ama execution path,
then terminates the current execution context with a failed host outcome. The standard
launcher exits unsuccessfully; an embedding host receives the failed outcome
under RT-16. Attached `:` handles inability to begin an operation, including
acquisition refusal and LIST-10's documented preflight refusals. It does not
handle failures after execution begins. Error reporting does not undo prior
state changes, completed output, or external side effects. Partially constructed
owned values still require cleanup; failed replacement preserves the previous
destination until destructive replacement begins; destruction errors follow
the terminal cleanup rules in PROTO-10.

### Use one explicit status representation

<a id="error-03"></a>

**ERROR-03.** Use this common C status representation:

```c
typedef struct {
    uint32_t code;
    const char *message;
    size_t message_length;
} AmaStatus;
```

Code zero denotes success. Nonzero codes identify a refusal or technical failure
under the operation's contract. The message is optional UTF-8 data with explicit
byte length, without a required terminating NUL. A null message requires zero
length; a zero-initialized status is success with no message. Message wording
must not determine control flow: inspect the code, not translated text.

### Keep error messages valid without allocation

<a id="error-04"></a>

**ERROR-04.** Message bytes must be immutable and remain valid for the lifetime
of the generated program. The runtime borrows the message and never frees it.
String literals and other program-lifetime immutable storage satisfy this rule;
function-local buffers, temporary native state, and resources released during
cleanup do not. Reporting an error must not require heap allocation. Dynamic
instance names, source spans, and the execution trace are added by the runtime,
not packed into an allocated native message. Exact status-code allocation is a
native ABI concern, with collisions prevented across participating libraries.

### Separate invalid-index refusal from conversion failure

<a id="error-05"></a>

**ERROR-05.** Reserve a distinct runtime-recognized code for invalid index values;
it must not be reused for allocation, decoding, or other technical failures.
For indexed invocation/read, invalid index or position-bounds refusal can select
an attached `:` without executing the target. An unrelated technical conversion
failure produces a failed execution with diagnostic trace, even if `:` is present.
The runtime must apply this distinction consistently to all index-capable types.
A failed conversion's output is never consumed. No message, C call stack,
exception object, or text parsing is needed to classify the outcome.

### Remember the caller's last refusal

<a id="error-06"></a>

**ERROR-06.** Every executable instance's service context contains a last-acquisition-
refusal snapshot with the common `AmaStatus` representation, initially all-zero
success. On a refused attempted entry, resume, or retained-instance read, write the
new refusal status into that operation's caller context before entering `:` or
reporting an unhandled refusal. The refused target does not execute and none of
its output actions run. A CALL-09 link uses the initiating caller's context and
preserves earlier completed chain effects. Static constant reads do not acquire
an instance and do not write a refusal snapshot; their copy errors remain technical
failures.

Ordinary successful calls, returns, yields, and library reads do not clear or
consume the caller snapshot. Refusals occurring inside nested callees update
those callees' contexts, not their callers' saved values. A subsequent refused
operation initiated by the same instance replaces its snapshot; application code
may first copy the old status into its own variable if needed. This is the last
refusal, not a history or exception stack.

The library reader makes a read-only value copy of its immediate caller's saved
status through generated runtime access. It does not expose a pointer to metadata,
modify busy/return state, or retain a borrowed caller view. Message bytes remain
safe under ERROR-04's immutable program-lifetime contract. The copied status is
an ordinary library value, with independent code/length fields. Its nonzero code
is result data, not the reader operation's technical return status: a successful
read reports technical success even when the saved refusal code is nonzero.
Reader execution
must not overwrite the status it is retrieving simply because it is another call.
The reader does not need to allocate a diagnostic message or build a trace buffer.

This snapshot is service metadata, not declared application state: ordinary and
`%` owned clones initialize their new refusal context to success rather than
inheriting the source's last refusal. The target's permanent fields and existing
saved context are unaffected by reference assignment. A failed acquisition and a
fatal technical error may share a status representation, but only acquisition
or LIST-10 preflight refusal selects `:`. No list method reports an action/outcome
enumeration instead of handling its pre-entry refusal. Fatal
errors use the active diagnostic record rather than stale last-refusal metadata.

The snapshot is logical per-instance state. [RT-02](runtime.md#rt-02) and
[GEN-03](compiler.md#gen-03) permit a compact or implicit representation only
when every allowed reader observes exactly this same status. For example, a
proved always-success snapshot may be reconstructed as zero without allocating
all status fields in each instance. Distinct callers' nonconstant snapshots
must remain independent, and a successful call must not clear one as a storage
shortcut. The C transport representation under ERROR-03 is unchanged.

### Map execution to source locations

<a id="debug-01"></a>

**DEBUG-01.** Execution-path diagnostics are a required debugging feature, available
without exceptions or a C stack trace. The compiler emits a source map from
operation/call-site identifiers to module identities, source files, lines, and
columns. The runtime records the current operation before fallible execution and
retains the caller's call-site identifier with each outstanding return link.
Return selectors may supply this identity if their mapping is unambiguous.
CALL-09 operation identifiers also map their enclosing selector path to source
locations. Active chain frames supply selection context for diagnostics; completed
parent bodies are not fabricated as outstanding invocations. Stream this context
from existing service metadata without allocating a trace buffer.

### Capture the active path before cleanup

<a id="debug-02"></a>

**DEBUG-02.** Before releasing return links or cleaning up error state, traverse
the active instance-owned caller chain from the failing operation to the root.
Report the cause, the failing source location, and each caller's invocation
location using original names, including Unicode, rather than hashes alone.
An initialization failure before root execution must identify its source and
initialization ownership path without fabricating active invocations. Diagnostic
emission must not require allocating another instance or building a heap-sized
trace buffer: stream available entries and provide a minimal fallback if detailed
reporting fails, especially on allocation failure. Structured output format and
trace limits may be specified separately.

### Describe active calls, not execution history

<a id="debug-03"></a>

**DEBUG-03.** A trace describes active execution, not object history. Yield `%`
releases the previous caller link; subsequent resumption traces the new caller
chain. Copying a continuation does not copy historical callers. No history of
all invocations or automatic native C backtrace is promised.

### Return from native C before reporting the Ama path

<a id="debug-04"></a>

**DEBUG-04.** Native code returns normally with its failure status before the Ama
runtime reports the error. A helper may attach native context, but its internal C
calls are not automatically Ama trace entries. Platform C debuggers/backtraces
are separate facilities. No `longjmp`-style escape may skip required native cleanup.
Hardware faults or native undefined behavior are not guaranteed to produce a
managed Ama diagnostic. `#line` mappings complement runtime traces for debugging
native/generated C; neither substitutes for the other.
