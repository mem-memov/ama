# Native views and bounded runtime authority

The [specification](../../spec/README.md) remains authoritative. This records
T012's native boundary under [CABI-02](../../spec/implementation/native-interface.md#cabi-02),
[CABI-03](../../spec/implementation/native-interface.md#cabi-03) and
[CABI-05](../../spec/implementation/native-interface.md#cabi-05), with concrete
source in `src/runtime/program/native/` and generated binders in
`src/codegen/emit/views.c`. The [lifecycle component](native-lifecycle.md) owns
resources; this component owns only temporary access authority.

## One native operation

`AmaNativeBorrowScope` is a zero-initialized, linear runtime-only owner. The
compiler-generated entry adapter begins it for the exact current Running
receiver, which must have a published Live identity and a valid attached return
link. An active world operation refuses entry with Busy; terminal/invalid receiver
authority fails without changing the output scope or taking the gate. The context
supplies one executor; this local gate is not a thread mutex or a public context
handle. It does not create an instance, execute its body or acquire native resources.

On success the scope borrows the program/world and remembers the exact receiver.
The world operation gate prevents recursive destruction, copying, preparation or
other Ama operations while native views exist. End releases only that gate and
clears the scope. Repeated end or later resolution through the ended owner is
InvalidState. The owner must not be copied while active. Native authors receive
typed views and the copied services snapshot, never this internal owner, its
program/world pointer or pool metadata.

Generated entry performs begin, binds the interface, obtains the immediate
caller's read-only refusal snapshot, invokes the inline native entry only after
successful binding, then ends the scope. Every post-begin failure passes through
that end before the common execution-failure branch. Returned status is checked
after closure; malformed success cannot keep a view live or skip cleanup. Ama
label dispatch, host return and cooperative yield occur after the native C call
has returned and the scope ended. No setjmp/longjmp or hidden retained C frame
extends the lifetime.

## Nominal resolution and permitted access

`ama_native_borrow_cell` is a private generated-adapter service. It first requires
an active scope, its current operation gate, unchanged current receiver, nonfailed
world, a valid role and nonzero expected type. Empty references return
EmptyReference. A different nominal type returns TypeMismatch before looking up
or casting a payload. Exact pool lookup then checks publication and generation;
ordinary read requires `ama_instance_access(..., AMA_ACCESS_READ)`.

The Receiver role admits only the scope's exact Running identity under its
already-established invocation authority. It cannot be used to obtain another
Busy receiver. Ordinary Read refuses even the current receiver as Busy. This
separation lets generated interface binding use the running module's declared
input/result storage while keeping arbitrary reference reads subject to normal
availability. Each successful binder checks the nominal type before applying
that type's C storage layout. General owned-child binders recurse only through
the statically finite eager ownership layout; reference cycles are opaque views
and do not recursively expand C scopes.

Cell output changes only after all checks. Generated scope fields may be partially
filled during a failed recursive bind, but remain private and are never passed
to native entry. Parameter views are const reads of already-installed callee
storage; value binding and cloning precede entry, so the view does not alias an
owned caller source. Output views write target result storage. This does not
turn an input into a writable alias merely because both use the same native C
representation.

The private resolver accepts context-local handles supplied by checked generated
storage. It is not a public origin-authenticating host API: external identity
checks remain the [context gateway](execution-contexts.md) and T123 onward.
Current-source frontend integration remains T024/T055/T058; general list service
and cursor adapters remain their listed implementation tasks.

## Separate graph state and native resources

Persistent Ama handles, owned children, references, list membership and execution
metadata remain declared compiler-visible storage. The native payload's clone
hook copies native resources only; the runtime separately clones the ownership
tree and fixes internal declared references. No tracing callback for hidden Ama
handles is added. Opaque native payloads may own ordinary C resources under their
lifecycle contract and the specification's explicit isolated-library exception;
they may not retain ordinary Ama relationships encoded as pointers or integers.

Generated author headers expose nominal Read/Write types and incomplete reference
and list storage types. Native code cannot access `.location` or pool metadata
through those declarations. Checked reference/list operations are generated
access contracts, not a license to recover private offsets. New current-source
service families are completed by T055 and the list roadmap; this component does
not claim an entire list-mutation API. Native C can still cast or retain arbitrary
addresses; violating the bounded-borrow contract is unsupported native code,
not a C sandbox guarantee. No view, handle, scope address or caller pointer may
escape to payloads, C globals, asynchronous callbacks or worker threads.

## Argument and executable evidence

Scope acquisition has one owner and one release. Under the context's exclusive
executor, no competing world operation can destroy or replace borrowed storage;
stable pool slots preserve native addresses through allocation/growth. Nominal
comparison precedes layout application, so equal C layout cannot redirect a
foreign type into the wrong pool. Generation/publication and availability checks
precede output assignment. The Receiver rule is exact identity equality rather
than a general bypass of Busy checks. Ending erases authority before execution
continues. These properties establish bounded supported access without needing
an allocator, fallback or guessed representation.

Scope begin/end use O(1) time/space. Cell resolution uses the existing exact type
registry lookup, O(T) for T registered types, followed by O(1) pool and metadata
checks. Binding a finite eager view visits each expanded owned field once and
uses stack storage proportional to that statically expanded view. It performs
no graph walk through non-owning references and no runtime allocation. Native
resource costs and later list service adapters are separate.

- `tests/unit/runtime/native-borrows/` uses real counted/address-sensitive pools.
  It checks binding to already-installed children without allocation, exact
  receiver authority, rejected reentry, empty/stale/wrong-type/Busy references,
  unchanged outputs on failure, end expiry, repeated end and fresh later scope.
  Independent clones retain separate native resources and runtime-fixed declared
  references; changing the clone does not change the source. Cleanup leaves zero
  counted allocations.
- `tests/unit/native/scaffold/declarations/` independently compiles generated
  author headers. A positive client writes an output/read-only input correctly.
  Four negative clients require a diagnostic at the selected forbidden expression:
  input mutation, another nominal type with identical payload layout, incomplete
  reference storage and incomplete list storage. They compile with the verified
  native toolchain profile; no arbitrary C parser or spelling heuristic is used.
- `tests/unit/codegen/execution/` executes generated inline native calls. A host
  observer asserts that the gate is closed on both successful and malformed-status
  return and that terminal cleanup empties every pool. Existing independent-value,
  native error, reentry, list-copy/pool-growth and yield/resume cases compose the
  new scope with the retained backend. Passing these fixtures does not certify
  current-source behavior beyond their documented supported subset.

T012 acceptance passes the full strict `-DNDEBUG -Werror` test tree and focused
ASan/UBSan borrow, declaration and generated-execution leaves, with generated
program instrumentation enabled and leak detection disabled. Complete coverage
checks 748 maintained inputs with zero debt. Doxygen produces no warnings and
checks 577 maintained files, 2386 interfaces and all 287 canonical/page destinations;
two complete generations produce byte-identical bytes. Markdown links resolve.
