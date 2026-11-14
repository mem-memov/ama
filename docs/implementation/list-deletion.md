# Typed remove and clear execution

T079 implements the two destructive list methods under the existing
[service grant](list-service-frames.md). The contracts are
[LIST-06](../../spec/language/lists.md#list-06),
[LIST-08](../../spec/language/lists.md#list-08),
[LIST-10](../../spec/language/lists.md#list-10) and
[LIFE-05](../../spec/implementation/native-interface.md#life-05).
The remaining `swap`/`length` methods and foreach retain explicit execution guards.

## Their source forms

Assuming `items[]` and an ordinary `..integer` variable `position` are declared:

```text
items[]
 remove !
  < position
 clear !
 clear !
```

The first method removes exactly the current position. The first clear removes
the entire remaining sequence; the empty second clear succeeds. A method's own
`:` handles a preflight refusal, after releasing the complete initiating service
tree. An owning destructor error is technical and never enters that handler.

## Shared typed parameters without irrelevant values

The former insertion-only preparation is now factored under
[`runtime/program/lists/inputs`](../../src/runtime/program/lists/inputs/inputs.h).
All supplied sources are evaluated once in textual order, followed by fixed
declaration-order installation. A method's explicit parameter forms are scalar
owned integers or element references. These are type descriptors, not action
selectors; each method still has a separate entry point and fixed signature.

`remove` constructs one owned integer input. Its omission/placeholder uses `|0`,
not the native integer module's default constructor. Conversion occurs once
through that type's declared read-only protocol, consuming output only on checked
success. Reserved invalid-index and out-of-range positions are expected method
refusals. Other native errors stay technical. The integer root remains owned by
the method frame until cleanup in reverse declaration order.

`clear` has zero parameters. It evaluates no source, creates no integer root and
calls no index converter. The emitter omits its installation array entirely,
rather than emitting an empty ISO C array or inventing a position-zero argument.
No result object or retained outcome is constructed for either method.

## Whole-forest preflight

The checked storage boundary validates `position <= length` followed by
`count <= length - position`, so range arithmetic cannot wrap. `remove` requests
one entry; `clear` requests the complete current sequence. Bounds refusal precedes
destruction and changes no result receiver or collection state.

For an owning list, every selected root and owned descendant must be available
before the first native destructor. Preflight protections are temporary. If a
later subtree is busy, earlier protected nodes are restored, all memberships and
targets remain unchanged, and no destroy credit is consumed. Removing one entry
does not require unrelated entries to be available. Clone capability is never
needed for removal, even when the element type is noncopyable.

For a reference list, removal retires only the selected occurrences. It does not
look up, acquire, clone or destroy their target handles. Duplicate references,
empty handles, stale handles and a handle to the running caller remain ordinary
reference data. Independent references to a surviving target are not revoked.

## Destruction and publication

After complete preflight, owning cleanup visits removed roots in reverse order.
Each native credit is consumed before its destructor, and cleanup continues
through other removed roots after failure. The parent ownership ledger is
reduced by exactly the removed roots. Membership retirement and suffix position
correction then remove those occurrences. Owning target generations invalidate
escaped old references; surviving target identities and memberships are preserved.
Clear retains the collection incarnation: it is a structural edit, not whole-list
replacement. Empty clear changes no target or membership.

No user code observes the partially published order. Cursor correction will be
connected in the traversal task before foreach is enabled. Destruction failure
is already irreversible and terminal; the runtime promises cleanup and preserved
primary failure, rather than rollback to destroyed values.

Bounds and checked subtree-availability checks set a separate refusal flag.
Later destructor results never enter that classifier. In particular, Busy from
a native destructor is a technical failure, despite sharing its numeric code
with admission Busy. Prepared inputs/results and the exact service grant are
released through the existing frame ancestry rules; native cleanup cannot start
a sibling or end the parent prematurely.

## Invariants and argument

Before destruction, preflight either protects the entire selected forest or
restores the protected prefix. Since it invokes no native hooks, a failed
preflight consumes zero lifecycle credits. Successful preflight establishes that
destruction can traverse the complete initialized owned forest without deleting
a running/waiting subtree. Each cleanup operation consumes its own recorded
credits once; terminal errors cannot cause a retry or resurrection.

Removing exactly an order range and updating the remaining inverse positions
preserves the [membership bijection](list-membership.md). Reference removal has
zero target credits; owning removal has one parent child credit per removed root.
Thus ledger changes and membership changes describe the same removed set. All
identities retained outside the list are validated through ordinary owner/record/
target generations rather than a search for equal handles.

Preflight/destruction cost is proportional to the selected ownership forest,
including the retained registry/scratch-map costs. Suffix compaction is O(n).
Clear has no surviving suffix; empty clear performs no element work. Method
frame storage remains O(1) per lexical site, and caller/type lookup retains O(T)
with the current linear registry.

## Evidence

`tests/unit/runtime/list-deletion` uses actual stable pools and address-sensitive
native resources. Five cases cover a busy child in the second owned entry before
any effect, rollback of earlier protection, removal independent of another busy
entry, exact membership retirement/survivor positions, busy duplicate reference
targets, empty clear without integer construction/conversion, terminal destructor
failure and every preflight-allocation failure. Failing native initialize/clone
hooks demonstrate that deletion invokes neither operation. Allocation credits
and extra pools are checked at teardown.

`tests/unit/codegen/list-deletion` compiles/runs four projects. They check exact
survivor order and stale owned aliases, full versus empty clear, reference entries
pointing twice at the running root, reverse native destruction continuing through
technical Busy without a handler or retry, and genuinely noncopyable native
elements. The C toolchain checks zero-argument descriptors and actual control
adapters. The existing append/insert suite is rerun against shared input staging.

Per-file coverage, standalone headers, runtime and generated-program ASan/UBSan,
the full strict suite and reproducible Doxygen validate the implemented boundary.
