# Calling modules and using their results

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Starting and reading a particular instance](#starting-and-reading-a-particular-instance)
- [Binding inputs and outputs](#binding-inputs-and-outputs)
- [The exact order of a call](#the-exact-order-of-a-call)
- [Calling a result directly](#calling-a-result-directly)
- [When an operation cannot begin](#when-an-operation-cannot-begin)

<!-- /chapter-contents -->

A module type describes behavior and storage. A call always operates on a
particular instance: a declared owned value, the target of a checked reference,
or a selected list element. Naming a type path never creates an implicit default
instance.

There are three operations, distinguished on the target line:

| Source form | Meaning | Executes the target body? | Resets parameters and results? |
|---|---|---|---|
| `worker !` | Start at entry | Yes | Yes |
| `worker %` | Resume a saved position | Yes | No |
| `worker` with indented `>>` | Read retained results | No | No |

**Example.** Assume `worker` is declared, its first input and first output are
owned values of the same type as `source`, and `received` has not appeared before:

```text
worker !
 < source
 > received
```

The runtime acquires `worker`, installs `source` as its first input, initializes
the other interface fields, and executes the body. Completion or yield transfers
the first result into `received`. TYPE-06 declares that receiver's persistent
storage before execution; receiving the result performs an ordinary owned copy.
This copy clears saved execution positions. A later unmarked read can receive
the stored result again without executing `worker`.

**Why.** Explicit entry, resume, and read modes make state changes visible in the
source. Separate input and output lines keep interface binding distinct from
execution. Each operation validates and protects its target before using it.

## Starting and reading a particular instance

### Entry requires an explicit exclamation mark

<a id="call-01"></a>

**CALL-01.** All entry-start invocations require the suffix `!`, including
zero-argument invocations. Targets must select declared instance or collection
storage, whether explicitly declared or inferred under TYPE-06. Direct module-path
targets such as `.shop.item !`, `.shop.item %` are invalid; lookup must not create or
select a default instance.
A path followed by `>>` is a separate constant read under CONST-03, not an
invocation or retained-instance read. Parameters and requested results appear on separate
indented lines. Inputs precede all output-phase actions. An operation's ordinary
bindings use named or positional mode consistently; CALL-09 selectors are separate
actions. Positional binding order follows interface declaration order.

```text
worker !
 input < source
 output > destination
```

### Inputs, defaults, and complete value results

<a id="call-02"></a>

**CALL-02.** Input values use ordinary type-defined value semantics; reference
inputs copy checked handles. Omitted inputs use their declared defaults for
entry-start execution. Requested results are transferred on completion or yield;
unrequested results remain stored. Repeated identical inputs still execute the
body repeatedly. Reactivity and deduplication are application decisions.

For a template target, ordinary typed inputs and already declared typed output
receivers provide compile-time evidence under [TPL-03](templates.md#tpl-03),
together with typed defaults, declaration initializers, assignments and
reference/alias connections. A receiver with unresolved generic arguments
contributes equations rather than an independent concrete anchor. A new receiver
under TYPE-06 does not choose a type merely by existing. Input bindings also
support the explicit constructor patterns of [TPL-07](templates.md#tpl-07).
Static constructor
formals have no runtime input position; their named type applications follow
[TPL-08](templates.md#tpl-08), rather than adding a new ordinary call argument
form. The selected instance already has one fixed concrete specialization before
execution. Omission/defaults and result transfers retain the semantics here;
they do not partially bind a static template or choose a new runtime type.

#### What receiving a result copies

Every requested result, including one received into a TYPE-06 inferred field,
transfers its complete value state by TYPE-05 and COPY-01 through COPY-07. An
owned scalar copies any native payload, parameters, retained results, persistent
fields, and recursively owned children/lists; compiler-visible internal references
follow the copied ownership tree. A whole owning list copies all elements in one
copy boundary. A reference scalar copies only its checked identity, and a reference
list copies its handle entries, without cloning their targets. The source remains
unchanged and no module body executes as part of copying. Native resources require
independent clone capability; copying is not raw-byte duplication or shared ownership.

#### Receiving a value does not preserve its continuation

This is ordinary value copying: subject to COPY-06's exact self-assignment no-op,
COPY-03 clears saved positions throughout a copied owned tree and never copies
active invocations, caller links, busy flags, or source pool identities. A reference
copy leaves its target's execution state unchanged.
The new field has no special continuation-copy behavior; preserving saved positions
requires a separate explicit `% source` operation allowed by CONT-04, not a new
output-receiver form. Inference changes how storage is declared, not result-transfer
semantics or the existing self-assignment rules.
Selecting a result for a direct operation under CALL-09 is not a result transfer
and performs no implicit clone.

### Reading retained results without executing the target

<a id="call-03"></a>

**CALL-03.** Retained reads omit `!` and `%` on their target line, accept no inputs,
and require at least one output-phase action: a `>>` transfer or a selected-result
operation under CALL-09. Every ordinary transfer to a receiver uses `>>`.
The enclosing target's body does not execute; selected results may execute their
own bodies under CALL-09. Reading does not execute a computed getter or freeze
objects identified by reference results.

```text
sensor
 measurement >> latest
 error >> last_error
```

A bare unmarked target is invalid. Combining `!` or `%` with `>>`, mixing `>` and
`>>`, or supplying `<` to an unmarked read is invalid within one operation level.
Nested operations have their own modes. Validate the complete operation tree
before resetting storage or executing any target code.

#### Declaring a dependency does not execute it

A dependency is declared once with a local name and then invoked by that name:

```text
@!
item
.shop.item

item !
```

The declaration creates owned state once when its enclosing instance is created;
it does not execute the child body. `item !` executes that particular child.
Acquiring `&item` likewise refers to the declared object, not its module type.

## Binding inputs and outputs

**Explanation.** Positional bindings follow interface declaration order. Named bindings select a declaration by its interface label. An operation uses one mode consistently across its ordinary input and output bindings; selected-result operations are separate actions.

### Leaving positional inputs or outputs unused

<a id="call-06"></a>

**CALL-06.** A positional input line may contain only `<`, with no source;
a positional output line may contain only `>`, with no receiver. Each consumes
one position in its respective parameter or result sequence. A skipped entry
input keeps its declared default after the normal `!` reset; it does not retain
a previous invocation's input. A skipped output causes no result copy or hidden
receiver allocation, while the target still computes and retains that result.
Input and output position counters are independent and follow interface order.

For an interface with three parameters and three results:

```text
worker !
 < first
 <
 < third
 >
 > selected
 >
```

This binds parameters one and three, leaves parameter two at its default, and
receives only result two. Omitted trailing input/output lines remain allowed;
explicit trailing placeholders are also valid within the declared interface size.
Every placeholder counts toward argument arity, so an extra empty line beyond
the interface is a compile-time error. Input lines, including empty ones, must
precede output lines. Spaces and an optional trailing comment after a marker do
not supply an operand.

These forms belong to positional mode only. Named bindings still require their
source/receiver; mixing a positional placeholder with named arguments violates
the existing consistent-mode rule. `%` resumption may use output placeholders,
but no input lines, including an empty `<`. Each ordinary retained-result transfer
still uses `>>` with an explicit receiver; CALL-09 separately permits selected-result
operations without a receiver. Neither rule admits `>` or an empty `>>` in read mode.
An empty `>` at call-argument indentation skips a result; standalone `>` at body
statement indentation remains early return. Validate the whole argument list
before acquiring/resetting/mutating the target.

### Constructing an input from literal bytes

<a id="call-07"></a>

**CALL-07.** An entry-start input accepts a line literal in positional or named
form. The receiving declaration determines its type; the caller does not specify
another type or create a hidden Ama variable/instance.

```text
writer !
 < |Hello # this is data

writer !
 message < |Hello
```

There must be exactly one ASCII space between `<` and `|`, and between a named
label and `<`. Everything after `|` through the physical line ending is payload,
including leading/trailing spaces, `#`, quotes and further bars. TAB is rejected
globally even in this payload. No trailing
comment is recognized on such a line. LF/CRLF and optional final newline have the
same meaning as for declaration literals. `< |` supplies an empty literal;
`<` alone remains an omitted positional input selecting its default. Each literal
binding consumes exactly one input position. Existing input-before-output and
named/positional consistency rules apply.

#### Destination type and supported forms

Resolve the bound parameter first. It must be a single owned value, not reference
storage or a whole-list parameter, and its type must support native literal
construction through `|`. This operation does not initialize a list element,
splice a list, create a referenced temporary, or forge a handle. Pass reference
and collection variables normally. Named literals for unknown parameters, excess
positional literals, incompatible storage and missing literal capability are
source-located compile-time errors.

#### Preparing a literal parameter

Validate the complete call before acquiring/resetting its target. After successful
acquisition, prepare the supplied literal value using the receiving parameter's
type and its ordinary owned-field construction rules. Select the literal payload
constructor over fresh native storage exactly once, without first constructing
that parameter's default payload. Replace the previous parameter value under the
normal preparation/commit/cleanup protocol. Unbound parameters and results retain
their specified reset/default behavior. Compiler storage used for preparation is
implementation bookkeeping, not an implicit source-level object or numeric value.
Do not execute the target body until all inputs are successfully installed.

#### Literal failure is a technical failure

Construct on every entry call, even for identical payload bytes. Literal failure
is a technical failure with the call site, binding and parameter declaration/type
context; `:` does not handle it. Release acquisitions and partial resources under
the existing lifecycle rules. There is no new rollback guarantee for an entire
interface reset; each replacement obeys the existing preparation/commit contract.
Native constructors are not executed by the compiler to guess validity. `%`
resumption and `>>` reads accept no input bindings, including literals. Literal
sources remain forbidden in assignment, conditions, indices and output bindings.

## The exact order of a call

**Explanation.** Evaluation is sequential. An earlier output can change an index used by a later output, and a later failure does not undo completed work. The following algorithm defines that order for every call form.

<a id="call-08"></a>

**CALL-08.** Invocation has a fixed evaluation order, independent of module loading
and of whether bindings are named or positional:

1. Statically resolve and validate the entire operation: target mode, binding
   labels, input/output order, arity, storage forms, types and capabilities.
   Classify result receivers lexically under TYPE-06 and solve their type terms
   with the other declared connections before runtime. Typed defaults constrain
   specialization even when this invocation supplies their input, without
   executing those defaults. All receiver fields already exist when the
   operation begins.
   An ordinary named input/output binding label may occur at most once in its
   respective sequence. Different bindings may name the same caller-side receiver.
2. For an instance/list operation, evaluate the target selection once and acquire
   it. A static constant read instead resolves its namespace without acquisition.
   Refusal to acquire this selected target (including invalid indexed target
   or absent resume position) selects the attached `:`. LIST-10 additionally
   defines pre-entry validation refusal for intrinsic list methods after their
   required inputs have been prepared. A technical index-conversion failure
   remains fatal. Do not reset data or assign requested results on refusal.
3. For entry-start, resolve supplied ordinary input sources and their indexed
   selections in binding text order in the caller context, before resetting
   target storage. Protect any state needed for copying until it is consumed;
   merely copying a reference handle does not acquire its referent. Invalid
   source selection or conflicting replacement is a technical failure, not `:`.
   A supplied standalone `&` therefore identifies the initiating caller, including
   inside a CALL-09 selected operation; it does not select the input receiver.
4. For entry-start only, install parameters in declaration order. For each, choose
   exactly one construction path: copy the supplied source, construct the supplied literal,
   or use its declared initializer/default when omitted. Construct over fresh
   replacement storage under LIFE-01 through LIFE-04. Do not construct the default
   and then overwrite a supplied value. Later defaults see already installed
   earlier parameters. Then reinitialize results in declaration order, so result
   defaults also see the installed parameters. No target body runs before success.
   Evaluate declared defaults in the callee's instance context; their standalone
   `&` identifies the callee's existing stable identity, not replacement storage.
5. For entry-start/resume only, record the return connection and execute. Resume
   skips steps 3 and 4 and jumps to its saved position. Retained and constant
   reads skip steps 3 through 5 and proceed to output transfer without an
   outstanding invocation, body execution or reset.
   Intrinsic list methods stage their inputs and perform LIST-10 preflight
   before committing their interface, mutating the collection, or producing
   results. Refusal disposes staged inputs and follows the method's own `:`.
6. On completion/yield, or during a nonexecuting read, process output-phase actions
   in source order. An ordinary binding transfers its requested result: resolve
   its receiver, including its current index, when that binding transfers.
   A previous action may therefore update an index used by a later action or a
   receiver written again later. Each transfer uses TYPE-05 and the replacement
   protocol; it is not simultaneous assignment. An omitted `>` consumes a position
   without resolving a receiver. A CALL-09 action instead operates directly on a
   selected result and finishes its complete subtree before the next action.
7. Release protected access after transfers/cleanup and continue the caller.

#### Which effects are preserved after a failure

This order does not permit a source to be invalidated by reset: reject conflicting
ownership/acquisition before destructive commit rather than silently snapshotting
it. Interface preparation and output transfers are individually failure-safe
replacements, not a transaction over the whole call. Earlier committed transfers,
target execution and external effects are not rolled back after a later failure.
An acquisition refusal leaves receivers unchanged; a later technical failure
need not. In a chain this applies to the refused link's unexecuted bindings;
previously completed actions remain committed under CALL-09. Standalone assignment
uses the order in REF-03.

## Calling a result directly

**Explanation.** A returned method is an executable module instance stored as a result. It can be selected and called in place. The syntax below removes an intermediate receiver while preserving the identity and state of the returned instance.

<a id="call-09"></a>

**CALL-09.** An output-phase action may directly execute, resume, or read a named
`@>` result of its enclosing operation without transferring it into caller
storage. LIST-04 extends the same structural selection to intrinsic list members
and their typed ephemeral outputs, with its explicit mode/lifetime restrictions:

```text
mod1
 mod2 !
  > mod3
```

Here `mod1` is a caller-side instance target. Its body does not execute. The
indented `mod2 !` selects the declared result `mod2` of `mod1` and executes that
existing instance from entry. `> mod3` transfers the selected instance's result
to caller storage, including a result-inferred field under TYPE-06 when applicable.
There is no intermediate caller variable, anonymous instance, ownership transfer,
new target construction, or implicit copy of `mod2`. Its persistent state changes
remain in `mod1`'s result. The explicit alternative below still calls an independent
ordinary copy when `mod2` is an owned value:

```text
mod1
 mod2 >> local_method
local_method !
 > mod3
```

Selection itself requires no clone capability. Ordinary input/output transfers
still require their normal capabilities, and `!` still prepares the selected
instance's interface under CALL-08.

#### A selector names the parent’s declared result

The selector uses the existing scalar/reference/list/indexed target spellings,
but resolves its base name exclusively in the enclosing target's declared `@>`
interface or LIST-04's intrinsic service/method interface. Require the
corresponding storage form; indexed selectors use the
same owning-list/reference-list distinction as ordinary indexed targets.
There is no fallback to a caller variable, module path, parameter, constant,
private body field, or native symbol. Selecting an owned scalar executes/reads
that instance; selecting a reference operates on its current checked referent;
selecting a whole list opens LIST-04's typed service rather than invoking every
element. A whole-list selector is an unmarked parent with selected intrinsic
method calls; whole-list entry `!`, resume `%`, and ordinary result-copy bindings
are invalid. Its fixed member names and each method's typed results are resolved
under LIST-04/LIST-08, with no action constant or first-class method value.
Shared-constant reads retain their ordinary `>>` bindings and do not
permit direct operations on immutable `@=` values.

#### Calling a returned reference

For a reference result, retain `&` in the selector and invoke its referent:

```text
mod1
 mod2& !
  > mod3
```

Here the current checked reference stored in `mod1`'s result `mod2&` selects the
module instance to execute. Validate and acquire that instance before entry,
exactly as for an ordinary reference-target call. State changes affect the
referenced instance and are visible through other references to it; the result
remains a reference and no owned copy of its target is constructed. An empty,
stale, deleted, or busy target causes this link's ordinary acquisition refusal.
`mod2& %` likewise resumes the referent, and an unmarked `mod2&` reads its retained
results. A selected reference-list entry follows the same reference-target rules.
Explicitly transferring a reference through `>>` also copies only the handle;
calling that receiver still selects the same referent, unlike copying an owned
result into an independent instance.

#### Resuming a returned continuation

A result containing a saved execution position can be resumed directly:

```text
producer
 continuation %
  > value
```

The continuation is an ordinary executable instance with a saved position, not
a separate declaration storage form. The selected `continuation %` resumes that
existing result without copying or resetting it; use `continuation& %` for a
reference result. No input bindings are permitted. Absence of a saved position
causes the selected link's normal acquisition refusal. Using `!` instead starts
from entry and discards the saved position.
To populate an owned result by copying a continuation, use explicit `% source`
under CONT-04. An ordinary owned assignment or `>`/`>>` transfer clears saved
positions under COPY-03; a later direct resume cannot restore them. Reference
transfers preserve their referent's execution state. See CONT-01 through CONT-06.

#### Sibling actions and nested modes

An action may use `!`, `%`, or an unmarked retained read; the latter requires its
own output-phase actions. These forms can nest to any finite source depth.
An enclosing `!` or `%` processes them after its own body completes or yields;
an unmarked enclosing read processes them without executing its body. All input
bindings of an operation precede its first output-phase action. Each operation
retains its own existing mode rules. A selector is an operation target, not a
named output binding: it neither consumes a parent positional result nor changes
the parent's named/positional binding mode. Repeated selections of the same result
are permitted and observe the same storage, including earlier state changes.
Ordinary duplicate named binding labels remain invalid.

#### Ordinary operands keep the caller’s lexical environment

The entire source tree has one initiating caller. Nested argument sources,
indices, ordinary result receivers, and `:` bodies resolve in that caller's
module namespace, not in any selected module. Calls written inside a `:` body
start ordinary root operations; result-selector context does not extend into
that body. A selector's base name is not a local-variable occurrence for TYPE-06,
while its index operands and ordinary
receivers are. Selecting `mod2` does not prevent a later first local occurrence
of `mod2` at an ordinary output receiver from declaring a caller field.

#### Exact depth-first execution and protection

Apply this exact evaluation and lifetime algorithm:

1. Parse all levels structurally, then statically resolve the complete operation
   tree from the root type through the selected result interfaces. Check modes,
   binding order, types, capabilities, selectors, and result-inferred declarations
   before any acquisition, reset, or body execution. Result selection uses the
   declared interfaces; TPL-03's scoped static constraints, including ordinary
   typed Ama body connections, may refine their nominal type terms. This never
   searches a body for an undeclared matching member, guesses a type from native
   code or runtime contents, executes a module body for inference, or introduces
   a hidden receiver.
2. Acquire and prepare/execute the root under CALL-08. After a body completes or
   yields, retire that body's current return connection before running a nested
   selected operation. Retain the acquired target and its pending output actions
   in explicit chain service metadata belonging to the initiating caller.
3. Visit output-phase actions in source order. For an ordinary binding, use
   CALL-08's current receiver/index and transfer rules. For a selected operation,
   resolve its result field and any current index once when reached, acquire the
   resulting target, apply its own CALL-08 operation, and finish its entire
   output-action subtree before continuing with the next sibling. This is a
   depth-first traversal; earlier actions may affect later selections or indices.
4. Keep each enclosing target and the storage used to reach its result alive
   until that subtree finishes. Authorized selection of an owned result under
   this protection is not a conflicting reentry into the owner. It does not
   bypass the selected target's own acquisition or the ordinary overlap/lifecycle
   rules: a reference to a running, waiting, or already incompatibly acquired
   owner/ancestor still refuses. Indexed selection invokes the selected stable
   element identity, not an address in a movable list buffer. Ordinary list and
   destruction contracts prevent invalidation of protected selected storage.
   Protection preserves lifetime and storage identity; the selected operation
   may modify its own state. Reference targets require their own acquisition.
   Whole-list mutations still obey their existing LIST contracts.
5. On successful completion/yield and output processing of a selected operation,
   release its protection and continue the next enclosing action. Release each
   enclosing protection after its final action. A selected module's yield returns
   its outputs normally and does not suspend the initiating caller's chain.
   The yielded module keeps only its own saved position; its later `%` creates
   a new current return connection, never one into an old chain.

#### Runtime invariant and bounded service metadata

For every active traversal prefix, the caller's chain service frames identify
exactly this chain's acquired targets and retained containers on its unfinished
source path, plus the next action at each level. Ordinary copy preparation retains
its own lifecycle bookkeeping. A completed subtree has no remaining chain frame or
return connection. The finite syntax depth bounds the frames needed per initiating
instance. Store them in stable runtime service metadata and lower continuations
to labels; never rely on C automatic locals or a C call stack across Ama transfers.
These frames and protections are not application fields, default instances,
working-state mirrors, or saved continuation data. Neither ordinary copying nor
explicit `% source` copying inherits them. Retiring a current return connection
does not clear a module's own saved resume position.

#### A refusal abandons the unfinished chain

Acquisition refusal of any link ends the entire initiating syntactic chain.
Record the refusal in the initiating caller under ERROR-06; discard all pending
actions and release only this chain's protections and current return connections.
Then execute the refused link's own attached `:` body, if present, in the initiating
caller. An ancestor's `:` does not catch a descendant's refusal. Normal handler
completion continues after the outermost chain statement; no remaining sibling
or ancestor output actions restart. Without that link's handler, use the existing
unhandled-refusal failure path. Previously completed transfers and method effects
remain committed. Technical failures remain fatal and use LIFE-05 cleanup.

Cleanup precedes the refusal handler, so its return, loop exit, or suspension uses
ordinary control flow with no retained chain protections. Resuming a handler after
`%` continues its body and then the statement after the outermost chain, without
restoring discarded actions. Releasing a chain does not clear modules' own saved
positions or unwind unrelated dynamic callers. Operations written inside a selected
method's body are separate initiating operations; a refusal handled there does not
itself mean that the method's acquisition was refused.

#### Example: two methods and their alternatives

**Example.** Assume `object` has owned method results `first` and `second`, each
with one compatible owned result, and `fallback` is a caller-side child instance:

```text
object
 first !
  > first result
 :
  fallback !
 second !
  > second result
```

On success, `first` runs and transfers its result before `second` begins. The
colon has the same indentation as the method it handles. If `first` refuses,
the runtime releases this chain's protections, calls `fallback` in the caller's
environment, and continues after the entire `object` operation. It skips
`second`. If `second` refuses without its own handler, the refusal is unhandled;
the completed first call and `first result` transfer remain committed. The
first method's alternative does not catch the second method's refusal.

#### Returning executable methods

A constructor may therefore return owned method modules containing independently
copied state, then expose those modules for direct invocation. Each method retains
its own copy; the chain does not merge these values into shared mutable state.
Shared state, when wanted, uses declared Ama references and the existing ownership
and internal-reference-remapping rules rather than opaque native payloads or
method names encoded as constants.

#### Relationship to condition loops

FLOW-04 separately permits a linear condition chain as a loop statement. Its
links use these direct-selection and acquisition rules, but its loop body is
ordinary caller control flow, not an output-phase action held under chain
protection. Ordinary CALL-09 operation trees do not acquire arbitrary control
bodies or a repeat boundary from a nested marker.

## When an operation cannot begin

**Explanation.** A checked reference may be empty or stale, and a live target may already be busy. An attached alternative describes what the caller should do when that operation cannot begin. Technical failures after entry use the separate failure protocol.

### Refusing acquisition or a documented list preflight

<a id="call-04"></a>

**CALL-04.** Entry, resume, and retained-read operations acquire their target
instance or list service. Missing, deleted, stale, busy, or invalid indexed targets are refused;
resume without a saved position is also refused. Access remains protected through
all requested result transfers. Failed acquisition must not assign results.
For CALL-09, protection includes nested output actions, and a refused link executes
none of its own output actions; earlier completed chain actions are not rolled back.
LIST-10's intrinsic-method preflight may also refuse before beginning its
mutation/body or result processing. No absent indexed target or removed
associated foreach membership is invoked merely because a former position or
external referent still exists. LIST-13 specifies list busy-state protection
and exact delegation to its selected method.

```text
target& !
 input < source
:
 unavailable_handler !
```

### A local alternative with a precise failure boundary

<a id="call-05"></a>

**CALL-05.** Attached `:` handles inability to begin the operation. It is not a
catch-all handler for errors after execution begins, decoder errors, or arbitrary
assignment failures. An unhandled acquisition refusal reports a diagnostic with
an active Ama execution trace and ends execution with a failed host outcome.
Before dispatching the alternative, the runtime saves the refusal's `AmaStatus`
in the caller's service context under ERROR-06. Static type incompatibility
remains a compile-time error.
For intrinsic list methods, inability to begin includes LIST-10's documented
preflight failures after input preparation; it does not include allocation,
constructor, clone, or destructor failures. No method mutation or output action
has begun when that method's `:` is selected.

#### An empty alternative still handles a refusal

An explicitly written empty `:` is a present handler: it handles the refusal
without executing statements, after saving status and performing the same
cleanup as a nonempty handler. It then continues after the operation, or after
the initiating outermost chain under CALL-09. It never retries the operation or
restores skipped output actions. Its empty-body warning under SYN-01 does not
turn the handled refusal into an unhandled failure.

#### Reading and reporting the saved refusal status

A library reader module obtains that saved status through the existing native
runtime-access contract and returns it as an ordinary declared result. No special
binding syntax, magic variable, additional error ABI, or exception object is
introduced. For example:

```text
@!
target&
.shop.item

failure
.sys.error.status

read_failure
.sys.error

report_failure
.sys.diagnostic

target& !
:
 read_failure !
  > failure

 report_failure !
  < failure
```

The example deliberately leaves `target&` empty, so acquisition fails before
execution. `.sys.error`, `.sys.error.status` and `.sys.diagnostic` are illustrative
library paths, not reserved words or claims of existing implementation. The reader
uses the context of its immediate calling Ama instance, not its own previous
failure slot. `failure` receives a value snapshot that later operations cannot
silently replace. The reporter accepts this value through normal parameter binding.
