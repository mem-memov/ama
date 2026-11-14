# Typed append and insert execution

T078 executes `append` and `insert` through resolved intrinsic IR and the actual
list service frames. Their names select fixed typed compiler interfaces, never
runtime action constants. The contracts are
[LIST-04](../../spec/language/lists.md#list-04),
[LIST-09](../../spec/language/lists.md#list-09),
[LIST-10](../../spec/language/lists.md#list-10),
[CALL-08](../../spec/language/calls-and-results.md#call-08) and
[CALL-09](../../spec/language/calls-and-results.md#call-09).
`remove`/`clear` use [typed deletion](list-deletion.md), and
[swap/length](list-ordering.md) complete the six methods. Foreach remains guarded.

## Useful source forms

This complete module assumes a copyable native `.value` with decimal literals
and the standard-root `..integer` contract:

```text
@!
items[]
.value
 |5
position
..integer
 |0

items[]
 append !
  < &items[position]
  > appended&
 insert !
  < position
  < appended&
  > inserted&
```

The first method clones the old first element and appends the independent copy.
The second inserts another independent copy before position zero. `appended&`
and `inserted&` refer to the actual stored objects. They do not own additional
copies. A direct method result action may execute that object immediately:

```text
tasks[]
 append !
  element& !
```

Here `tasks[]` is an owning list of an already declared task module. No method
instance or intermediate value variable is constructed. A yield in the selected
task returns to the initiating caller, which finishes the method output subtree.
The task remains stored in the list and can subsequently be resumed through an
ordinary checked reference or indexed call.

## Ordered preparation

The compiler emits each supplied input's exact interface ordinal, source and
evaluation operation. Supplied sources are evaluated once in textual order.
Omissions and placeholders are absent from that table. A literal has no ordinary
source evaluation; its bytes belong to the selected constructor recipe.

`insert` then constructs its owned integer position in declaration order. An
omitted position uses `|0`, even if the native module's default constructor would
produce another value. A supplied owned integer is copied using its selected
ordinary or explicit continuation recipe. These are real typed pool roots owned
by the method frame, not C-stack native values. The source-reference input copies
its complete handle and defaults to empty. It creates no ownership credit.

Direct structural source/index/receiver helpers receive the exact acquired
parent grant. Thus `< &items[position]` does not reacquire the busy list. Ordinary
calls, their inputs and executed result bodies receive no inherited grant.
All helpers keep caller-local operands separate from a selected result owner.

The backend executes this contiguous prefix at its preflight control label.
Earlier evaluation/staging and later prepare/commit audit labels do not repeat
effects. Valid lowering has no branch, return or resume entry into the interior
of that prefix; native constructors/protocols cannot suspend Ama. The helper
records the exact resolved evaluation/installation selector before each effect,
so delayed execution of these adjacent audit nodes does not lose failure sites.
Missing or duplicate required sites stop generation instead of choosing a nearby
module field or acquisition label.

## Preflight and publication

`append` uses current stored length. `insert` converts its prepared integer once
through the declared read-only index protocol. It consumes the numeric output
only after checked success. Reserved invalid-index conversion and a position
outside `0 <= position <= length` are expected method refusals. `SIZE_MAX` is a
valid converted host value; an out-of-range value refuses before arithmetic or
array access. Other native conversion errors, including Busy, remain technical.

An owning source supplied explicitly requires clone capability even when its
handle is empty. Omission/placeholder requires only default construction. A
nonempty owning source is preflighted across its complete ownership boundary
before native clone hooks. Reference lists copy empty, stale or busy target
handles without acquiring/cloning them. This distinction is statically checked
and also retained in the typed runtime helper.

The checked storage insertion prepares order capacity, a fresh membership record
and the complete new owned tree before publication. Successful commit publishes
the element, owner credit, order and inverse positions without allocation or user
interleaving. Source ownership is retained. Ordinary owned clones clear saved
positions and remap internal references. The method's `element&` result is exactly
the new stored target, including an empty/stale handle for a reference list.

Expected preflight refusal leaves order, membership identities, ownership and
result receivers unchanged. Private capacity may grow during a later failed
preparation. The runtime sets a separate refusal flag only at bounds, reserved
index conversion and callback-free identity/availability checks. Native
allocation/construction/clone phases never set it. Tests deliberately return the
same Busy code from preflight and from native construction/clone/conversion to
prove they take different routes; message wording does not classify them.

## Output subtrees and cleanup

Method results reside in the stable caller method frame. Reference output
bindings use an authorized direct structural transfer; selected result execution
acquires that already materialized value without inventing a result field.
A root list service has no outgoing module instance to park, so its selected
result frame may explicitly retain an empty outer context. Nested services park
their actual enclosing module acquisition. Release restores exactly that context.

Successful release ends the method's result and then its owned inputs in reverse
declaration order, while keeping the parent busy for its next sibling. Refusal
unwinds the exact frame ancestry: selected targets, method credits, service grants
and enclosing module acquisitions. Only an unentered slot is skipped during
explicit abandonment. The refusing operation's own handler runs after this
cleanup; pending sibling/ancestor actions are discarded, while prior committed
mutations remain. A parent's handler cannot catch a descendant refusal.

Credits are consumed before native destruction, and the service's attached-method
link remains present until all cleanup completes. Destructor failure therefore
cannot create a retry credit or allow callback reentry. Cleanup errors are
technical; they do not enter the pending refusal handler. Accepted technical
failure uses normal context disposal and reverse successful-publication cleanup.

## Argument and costs

Source selection has one textual pass; typed input construction has one fixed
declaration-order pass. The maximum intrinsic arity is two under LIST-04. Every
owning parameter/result root has exactly one frame credit after successful
construction and zero after cleanup consumes it. Reference inputs/results have
no destroy credit. Before commit, all destination changes remain private. Commit
preserves the [membership bijection](list-membership.md) and adds exactly one
owning child credit or one nonowning entry. Frame ancestry is supplied by resolved
IR, so cleanup neither searches target handles nor reconstructs source nesting.

Appending membership metadata has amortized O(1) growth cost; inserting shifts
and corrects an O(n) suffix. Native lifecycle, source-tree preflight and exact
clone/reference-map costs are additional. Fixed frame and input-handle storage
is O(1) per lexical method. Caller/type checks retain the existing O(T) registry
lookup cost. No C activation survives dispatch to an Ama body.

## Executable evidence

`tests/unit/runtime/list-insertion` independently covers default creation,
independent owning clones, nonowning busy/stale/empty handles, copied integer
parameters, insertion at the end, negative/full-host-size positions, malformed
conversion transport, source availability, native Busy and every allocation
failure along a fresh insertion prefix. It counts real native resources and
uses actual typed index/copy hooks. `runtime/result-calls` adds isolated
materialized-value acquisition with empty and retained outer contexts.

`tests/unit/codegen/list-insertion` compiles/runs the source forms above through
the real frontend, typed IR, C toolchain and runtime. Cases cover sibling
methods, one-time direct structural indices, actual stored task execution/resume,
ordinary continuation-copy clearing, receiver preservation, skipped siblings,
local refusal cleanup, technical constructor Busy and genuinely noncopyable
default elements. Explicit empty copy sources fail static capability checking.
Its standard-root integer is entirely test-owned; completion writes only that
checked fixture path. The shared fixture now emits only declared optional native
protocols and accepts an explicit root kind, without touching maintained library
companions. Sanitizer flags instrument the actual generated executables.

`codegen/control` independently checks intrinsic literal-site identity and
writer stream failures. The ready registry maps each maintained file to these
named component/composition cases; headers compile standalone. The full strict
suite and reproducible Doxygen check the complete supported slice.
