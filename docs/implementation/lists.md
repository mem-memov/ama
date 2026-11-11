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
