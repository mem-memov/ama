# Typed storage, lifecycle and control-flow plans

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

The public boundary is [plan.h](../../src/planning/plan.h), implementing COMP-02,
GEN-02/GEN-03, STATE-01 through STATE-03, CALL-08 and LIFE-01 through LIFE-05.
It consumes a successful [semantic program](semantic-analysis.md), including
ordered shared constants. It does not execute Ama or generate C. The semantic
owner and its loaded project outlive the plan. Initialize/build once, inspect
only success, then destroy the plan before its borrowed owners. Every partial
allocation belongs to one arena; failure retains a structured diagnostic without
publishing `valid`. No function recurses on source/module nesting.

## Storage domains

[Storage planning](../../src/planning/storage/layout.c) first allocates every
nominal type record, then links fields, so forward/reference cycles preserve
identity without requiring C type text. A field records its declaration, module
type, role, logical slot and one of four ownership/cardinality forms. Constants
are excluded. Parameter/result roles remain distinct from persistent fields,
although all retain storage after execution. Executable types require pools and
explicit generation/busy/position/return/refusal/acquisition service storage.
Constants-only namespaces require neither a pool nor executable entry.

Each shared constant instead has immutable `SharedStorage`, a nominal data-layout
template, its actual initializer recipe, startup ordinal and reverse cleanup link.
Its service-storage mask is zero: constructing shared values uses the data domain,
including recursively shared value fields, not pool slots or executable service
metadata. Reusing a declared-data template is not instantiating its executable
module. Copying a shared value into owned storage explicitly crosses into the
instance domain using the ordinary independent-copy recipe. Native constant
purity and resource restrictions remain native contract obligations.

Slots are logical positions, not guessed byte offsets. The native C compiler
supplies `sizeof` and alignment from registered representations. The emitter
will use these already typed layouts; it must not redo name/type resolution.

## Lifecycle instruction contracts

[Lifecycle plans](../../src/planning/lifecycle/create.c) are typed recipes over
fresh/live/finalized storage, with explicit normal and failure edges:

| Procedure | Required operation order |
| --- | --- |
| Default/literal construction | Construct declared fields in order; select exactly one native `+` or `|` payload constructor; mark live |
| Destruction | Destroy the native payload first; destroy fields in reverse order; finalize; accumulate errors while continuing cleanup |
| Ordinary clone | Preflight the whole source tree; begin one operation-scoped clone map; clone owned fields/copy handles; clone native payload; fix internal references; clear positions; release map; mark live |
| Continuation clone | Same data contract and native `=` hook, preserving positions recursively instead of clearing them |

Each field instruction carries its exact storage form and initializer sources.
An owning list constructs/clones its elements; a reference list copies/releases
handles without owning targets. Noncopyable types have no clone procedure.
Shared constants and service fields are absent from clone field sequences.
`LIFE_CLONE_OWNED` walks the field's ownership subtree using that same operation's
map; it does not call a child root recipe that independently fixes references or
releases the map. Complete-map fixup occurs only after the entire boundary has
been copied, preserving sibling/ancestor internal references.

Failure edges enter `LIFE_UNWIND_PARTIAL`: a runtime ledger records the successful
field prefix and whether the native payload became live. It destroys a live
payload once before completed fields in reverse order, never invokes the failed
constructor's destructor, and preserves the primary failure while accumulating
cleanup errors. Clone failure also releases its partial map. Native constructor
partial cleanup remains native-owned. These recipes are consumed by the [runtime ledger and walkers](instance-ownership.md)
and [typed entry preparation](entry-preparation.md).

## Explicit control graph

[Lowering](../../src/lowering/control/blocks.c) consumes resolved statements and
creates stable control nodes with unique checked selectors and original spans.
Explicit work tasks lower blocks backward with a known continuation; child entry
slots are patched through stable arena addresses. Branches select explicit body
or alternative entries. A loop's true edge enters a body continuing at its test;
its false edge and nested `^` reach the loop continuation. `^` outside a loop
fails semantic checking (with a defensive planning check). `>` reaches completion. `%` records its following control node as
a saved selector and reaches yield. Declarations emit no executable instruction:
initialization belongs to storage construction, including declarations in branches.

Assignments select the receiver, then source, preflight replacement, prepare,
commit, and release. The transfer enum selects ordinary/continuation clone or reference copying.
Every assignment retains the full replacement sequence. Ordinary exact-identity
no-op is a checked runtime outcome, not a source-spelling optimization or a
static exemption from copy capability. Preparation failure preserves old storage; destruction failure
after commit is terminal. Acquisition/overlap/source-tree checks must occur before
mutation, following the resolved storage form and copy mode.

[Calls](../../src/lowering/calls/operation.c) have these distinct edges/phases:

1. Acquire the selected instance/list once. Only refusal reaches the attached
   alternative; technical index/helper failures take the technical failure edge.
2. For entry calls, evaluate/protect supplied ordinary inputs in caller text order.
   Install parameters in callee declaration order, choosing the supplied copy,
   typed literal or declared default once. Later defaults read already installed
   callee fields. Initialize results afterward. Persistent fields are untouched.
3. Save an explicit instance return connection and output continuation selector;
   enter the callee's shared entry graph or its dynamic saved position. Static
   type links are patched after all entries exist, so same-type calls do not
   recurse during compilation. Native entries are distinct inline-body markers,
   not a temporary C-call backend.
4. On completion/yield, transfer outputs in binding text order. Evaluate each
   caller receiver/index at that step, then release target access. Multiple
   transfers to one receiver remain ordered rather than simultaneous.

Retained reads omit steps 2/3. Static constant reads also omit acquisition/release
and use the resolved shared storage. There are no list-action or list-input
installation opcodes. Semantic checking rejects whole-list execution and retained
list results before a plan is accepted. Typed intrinsic methods and their result
frames are T075–T084, under [LIST-04](../../spec/language/lists.md#list-04).

A refusal handler first snapshots the status into caller service storage.
The AST explicitly records handler presence: an empty `:` body handles refusal as
an empty branch, while no handler fails. It must not be guessed from whether the
body contains nodes. Technical failure always captures the active trace before
cleanup and host failure; it never enters a refusal branch. Completion and yield
use runtime return metadata, with host outcomes at the root, rather than C stack
frames. The later execution backend implements these typed operations with labels
and runtime services under the specification's existing contracts.

## Correctness, bounds and evidence

Induction over declaration order gives a bijection between nonconstant fields and
logical slots; shared storage is a separate bijection with constant plans.
Induction over lifecycle sequences gives ordered construction and exact reverse
cleanup. The initialized-prefix/live-payload ledger is the invariant required of
later runtime execution, including cleanup after failure.

For control lowering, the block invariant is: its produced entry executes the
written statements in order, then reaches the supplied continuation unless a
specified return/yield/failure/break transfers elsewhere. Backward composition
preserves this invariant for ordinary statements. Scheduling branch/loop child
blocks with their defined continuations proves the control cases by structural
induction. The work queue enumerates the finite AST without following runtime
back edges, so compilation terminates even for nonterminating user loops and
same-type recursive call graphs. Distinct selectors are never recycled/wrapped.

With `N` syntax/binding nodes, `D` fields and `V` types, linear statement lookup
and interface lookup give at most quadratic planning time in input size, plus
path-independent type/field lookups. Recipes, field/statement reversal records,
pending block tasks and control nodes require linear owned storage. Shared
lifecycle children are referenced as typed templates, not recursively expanded
into unbounded source. Allocation failure never publishes a usable partial plan.

[Planning tests](../../tests/unit/planning/ir/main.c) compose actual source loading,
ABI parsing, semantic checking, relation sorting and planning. They check storage
domains, constructor/destructor order, clone modes, explicit call input/output
ordering, no-reset reads/resume, empty/nonempty refusal routing, trace-before-cleanup,
loop/break/yield edges, same-type calls, typed list defaults, 128 nested branches
and every plan allocation failure. Recursive Make leaves remain independently
runnable. Release/Werror and focused sanitizer checks validate this boundary;
no generated C execution or working runtime is claimed.


T147 documents each storage/lifecycle/control allocation and its exact source
identity. All procedure/node/view storage is plan-owned and borrows the complete
semantic/project lifetime. Call entry targets are patched only after all entries
exist, so benign call cycles do not recurse during compilation. Graph/AST work
uses finite explicit records; source nesting does not consume recursive C frames.

Current plan tests prove the retained node families and order. They do not certify
complete current context/result/foreach/loop semantics: T043–T045 and their runtime
consumers remain open. Parsed-empty ordinary loops already leave after one condition conversion;
header and traversal lowering retain explicit unsupported guards until their fixed
restart and lexical frame operations exist.

## T041: publication checks for the typed IR

The existing `StorageField`, `SharedStorage`, `LifeNode` and `ControlNode` families
are the compiler's typed IR. They borrow exact canonical semantic identities;
source spans remain borrowed views of the loaded immutable source. Logical slots,
selectors, transfer kinds and native ABI members are operands rather than C text.
Borrowing typed semantic access/source records does not repeat name resolution.
The AST identity retained alongside a statement identifies its original source
construct; it is not an invitation to resolve names during emission.

[Publication validation](../../src/planning/validation/ir/program.c) runs after
forward entry links and shared startup/cleanup endpoints are complete, immediately
before `ProgramPlan.valid` is set. It checks:

- One planned nominal record for each resolved type, in exact inventory order.
- Declared nonconstant fields in order, exact nominal types, storage forms and
  logical slots; shared storage follows the resolved constant order and has no
  instance service state.
- Required lifecycle procedures, registered native hooks, field operands,
  original source maps, procedure endpoints and same-owner execution/failure edges.
- Control endpoints, unique nonzero selectors, statement and binding ownership,
  mandatory operation operands and edges. Ordinary edges remain in their owner;
  only explicit entry transfers cross to the exact callee entry. Refusal,
  technical failure and resume edges are checked separately.

Lifecycle instructions now have a separate allocation inventory, like control
instructions. Validators scan these finite owner lists instead of following
executable links. Thus recursive calls and loop backedges do not cause recursive
compiler calls or require reachability guesses. The inventories are trusted
allocator bookkeeping: this API diagnoses malformed typed edges/operands, not
arbitrary corruption of allocator memory or owner-list links.

The invariant is that each accepted edge names an instruction in the permitted
owner inventory, every operand names an already resolved declaration or native
binding, and each selector has exactly one owner. Enumerating the inventories and
comparing exact pointers proves membership; pairwise selector comparison proves
uniqueness. No hashing, spelling equality, C execution or allocation is needed.
For `N` total records, validation uses at most quadratic time and constant scratch
space. Existing construction and lowering proofs establish instruction order;
this structural validator does not attempt to prove arbitrary program termination.

A failure clears publication, retains a structured diagnostic, and leaves every
partial arena record destructible. Revalidating a published intact plan preserves
its publication state. A successful check alone cannot publish an unbuilt plan.
[Mutation tests](../../tests/unit/planning/validation/main.c) independently damage
storage, control and lifecycle operands/edges, source sites, selectors, hooks and
shared endpoints, and verify rejection without new allocations. Existing planning
cases exercise valid instruction families and every construction allocation
failure. Current selected-result, header-loop, foreach and self-reference runtime
guards remain: T042 onward adds their storage and execution support without
silently accepting incomplete lowering.

T042 adds independently validated [context, instance and fixed lexical storage](storage-planning.md).
The full-plan publication gate also checks this complete storage description.
