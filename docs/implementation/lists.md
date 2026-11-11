# Ordered lists

## Storage law

`runtime/lists/capacity.c` grows a contiguous identifier buffer geometrically,
checking the handle-count and byte-size bounds before allocation. Growth allocates
and copies before replacing the old buffer, so allocation failure preserves both
order and capacity. Only identifiers move; pool payloads retain their addresses.

Capacity is at least count. Initial construction and whole-list copying establish
this invariant; disposal clears both. Doubling gives a geometric sum of copied
identifiers bounded by twice the final capacity, hence amortized constant-time
append excluding lifecycle costs. Positional indexing remains zero-based.

The runtime list suite checks ordered growth, allocation failure and arithmetic
limits. Existing ownership/clone/transfer suites check composition with stable
payloads and reference remapping.

## Mutation boundary

`runtime/lists/mutation/` implements Inspect, Append, Insert, Remove, Clear and
Swap over the ordered handle buffer. It accepts an already decoded request;
module payload representation is outside this component's domain.

Bounds checks precede ownership work. Owned-source copy preflight covers the
whole source tree; Clear preflights all affected ownership trees before the first
destructor. Availability refusals update completion/outcome metadata but preserve
contents and state. Allocation, constructor and destructor failures remain
technical errors. Insertion publishes a fully built ownership tree before adding
its handle. Deletion completes cleanup and updates the parent's child ledger,
even when a destructor reports terminal failure. Reference entries never acquire
or destroy their targets, including empty and busy targets.

Preflight and preparation are separate from commit. Thus expected refusals have
no destructive prefix. Insert/remove shift precisely the suffix outside the
selected range, preserving relative order; Swap exchanges two identities only.
Lifetime child counts change by exactly the committed owned-element count.
The tests cover each action, Clear with a busy descendant, all allocation failures
in default insertion, terminal destruction, stale references and empty pool-block
reclamation. ASan/UBSan run with LeakSanitizer disabled in the traced environment.
