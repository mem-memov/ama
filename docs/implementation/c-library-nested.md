# A native parent owning a compiled Ama child

The maintained example in
[adapter.c](../../tests/unit/library/nested/project/packages/test/fixture/v1.0.0/src/adapter.c) implements
[EMBED-07](../../spec/implementation/c-library.md#embed-07) using the ordinary
native lifecycle and the checked C-library boundary. Its companion
[adapter.h](../../tests/unit/library/nested/project/packages/test/fixture/v1.0.0/src/adapter.h) contains one
pointer to an exclusively owned foreign resource. It contains no checked Ama
reference, parent field address, borrowed input or borrowed result.

## Ownership and construction

The private heap holder contains only the child's public context/instance
controls and its last execution outcome. The adapter always creates a fresh
context, allowing child constants to initialize, and then constructs its selected
exported root with defaults. Neither factory executes an Ama body. There is no
operation that adopts an existing context, so successful constructors extend an
acyclic forest of exclusive owners.

The fixture child declares shared text and condition constants, a text parameter,
two retained results, and private state. Before the first step, its default
response is `child factory` and its previous-state flag is false. Two adapter
instances own distinct child contexts and preserve their own state independently.

Construction has three resource acquisition steps: allocate the holder, create
the context, create the root. Native initialization failure receives no native
destructor credit, so the initializer itself closes any acquired context before
freeing the holder. Context disposal already owns its child root and constants;
there is no second root destructor owner.

The destructor disposes the available child context and frees the holder. It
never resumes a saved child position. Exclusive ownership and the parent's
checked admission gate establish that cleanup cannot run while its native entry
or a child call is still active. Accepted child cleanup may consume its context
even when returning a technical status; the adapter checks the consumed control
and releases the returned diagnostic in that case too. Cleanup failure does not
replace an earlier construction/execution failure.

The [native ABI](../../tests/unit/library/nested/project/packages/test/fixture/v1.0.0/src/adapter.abi) deliberately
declares only `+` and `-`. It has no `=`. Both ordinary copying and `%` copying
are therefore rejected before C compilation. Copying an opaque C pointer or a
context control does not create another resource owner or an independent clone.

## One explicit step

The entry borrows the parent's typed text input synchronously. On fresh child
entry it passes a counted public C text borrow; the child builds its own input
state before executing. A yielded child is resumed without passing new inputs.
The child's previous input remains in effect, even though a new parent entry
has reset the parent's own parameter interface.

Each child call returns normally through C. The adapter records its outcome,
copies the owned child text snapshot into the parent's declared text result,
sets separate previous-state/waiting results, and releases both child snapshot
owners on every path. Copying uses explicit byte counts, including embedded zero
and multibyte UTF-8. No child view survives its releasing owner.

Here the parent export is the native adapter itself. Its call **completes** even
when the child yielded. `waiting` is an ordinary declared condition result;
there is no automatic parent `%`, scheduler enrollment or busy retry loop.
A subsequent explicit parent entry performs the next child step. T130 adds the
separate outer Ama continuation and its supervision policies.

This adapter propagates child execution failure as a parent technical failure.
The returned `AmaStatus` uses immutable text in the parent's own code image,
after releasing the child's API result. It never forwards a diagnostic pointer
whose child code could unload. Retaining/copying a detailed child report and
keeping a supervising parent healthy are separate T130 policies.

## Exact binding to physical fields

This composition exposed a transport bug for exports declaring shared constants:
a semantic declaration ordinal includes constants, while an instance layout
excludes them. For example, two constants precede a parameter whose declaration
ordinal is two but whose physical slot is zero. Using the declaration ordinal
could read the wrong field or fail a valid retained read.

The codec now resolves each exact `SemanticSymbol` through its matching
`ScaffoldMember`/`StorageField` and uses that field's planned `slot`. It validates
every interface binding before emitting code. Constants have no physical slot
and return the explicit missing-slot sentinel. The same mapping drives prepared
inputs, retained result selection, result operation locations and the failure
description adapter. Public role order and C transport signatures are unchanged.

The invariant is independent of the number/names of constants: a declared
instance field has exactly one matching planned storage field, whose slot is
the descriptor-array index used by ordinary Ama dispatch. No subtraction,
name matching, source-order heuristic or runtime fallback repairs a mismatch.
Existing zero-constant transport suites and the nested two-constant child cover
both sides of this distinction through actual compiled entry/read calls.

## Evidence and boundaries

```sh
make -C tests/unit/library/nested
```

The producer builds and verifies the child as a genuine isolated static/shared
component. Its separate parent translation unit includes the real project
assembly, lifecycle and transport implementations. A public-header-only C host
links that parent with the child archive. Generated authoring aliases bind exact
private parent view names; the parent sees only the child's public header.
Test allocator/observer hooks are C-host instrumentation, not part of the product
ABI or a persistent Ama-state channel.

The suite checks:

- initialized child constants/defaults before bodies, two independent child
  contexts, first/repeated entry and explicit child resume;
- child yield completing the native parent call, ignored new inputs during
  resume, counted UTF-8/zero bytes and snapshots surviving both domains;
- abandonment of yielded children without another body call, and exactly-once
  destruction after explicit root or whole-context disposal;
- active-parent access through another root, active-parent disposal, wrong
  component context/instance controls, and stale child controls after cleanup;
- real child constructor failure after context creation, checked factory refusals,
  immutable parent diagnostics after child cleanup, and every measured parent
  constructor/entry allocation failure;
- statically rejected ordinary/continuation copying of the noncopyable adapter
  and rejected attempts to export an Ama reference through an owned value schema.

After each case the child code registry has no live contexts or value owners,
all observed child acquisitions have matching disposal, and the parent's counted
allocations and code pins return to zero. Retired load identity receipts remain
the process-lifetime identity history documented in
[component packaging](c-library-packaging.md#reloads-cannot-revive-old-numeric-controls).
ASan/UBSan instrumentation also covers the generated child, parent and C host.

This task does not introduce a universal foreign-reference transport, implicit
context sharing, hidden cloning or a scheduler. Full CLI-oriented example
publication and the standalone/nested EMBED conformance matrix are T131 work.
