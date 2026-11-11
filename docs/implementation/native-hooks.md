# Executable native ABI harness

This closes the six-binding/void-entry harness task in [TODO.md](../../TODO.md).
The production boundary is the [native ABI](native-abi.md), status headers and
read-only `AmaNativeServices` snapshot. The executable companion is the handwritten
buffer module under `tests/unit/native/abi/hooks/project/`, with all six bindings
listed in its `.abi` file. That file is a fixture, not a claim that a frontend
binding parser already exists.

The companion owns only a native allocation and native scalar fields. Literal
construction accepts an immutable bounded byte view and allocates independent
storage; default construction is empty. The input view must be valid over its
length. Length beyond PTRDIFF_MAX is explicitly unsupported by this fixture's
native array representation, never an unchecked allocation/copy. Allocation failure
returns status and owns no resource. Cloning constructs fresh independent storage,
and destruction frees it even when reporting an injected native cleanup error.
Fault-injection fields are native fixture controls, not inferred language behavior.

Condition/index conversion takes a const payload, writes an output only on success
and never executes the native entry. The fixture index representation is decimal
bytes: empty, negative, nondigit or overflowing values return InvalidIndex;
explicit technical fault injection returns its own nonzero code. For each next
digit d, checking `value <= (SIZE_MAX - d) / 10` proves `value * 10 + d` is
representable. Induction on prefix length proves exact conversion of every valid
representation; failure leaves the output untouched. Conversion is O(n) time,
O(1) auxiliary space. This is a library fixture interpretation, not a privileged
integer or literal category in Ama.

Constructors and clone are O(n) time/owned bytes and constant auxiliary space;
destruction releases one resource. Each terminates conditional on allocator/native
I/O behavior. Typed assignments check all six helper signatures. The native entry
uses the selected direct inline shape, borrows already-bound input, writes typed
results and separate technical status, and receives a value copy of caller refusal.
Its checked native execution count cannot overflow. Nonzero refusal result data
is not a failed native entry. It retains no Ama identity or borrowed view.

Run `make -C tests/unit/native/abi/hooks`. The five cases verify helper types,
independent data, explicit byte lengths, invalid/technical conversion distinction,
SIZE_MAX/overflow boundaries, and real typed native entry composition. Binding
input is cloned explicitly in the harness to model the contract; generated
interface binding, acquisition and frontend capability checking remain later work.
