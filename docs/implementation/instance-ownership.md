# Stable instances and ownership operations

## Pool identity algebra

A per-executable-type pool issues `(type, slot, generation)` handles. Generation
zero is empty. Reservation is private construction storage; publication requires
a live lifetime ledger. Identity validation and activity checks are separate.
No pool or spare capacity creates an implicit instance.

Blocks contain fixed-stride, max_align_t-aligned metadata/payload slots. Directory
growth copies block pointers, never payloads. Each free nonretired slot occurs once
in the intrusive free chain. Release requires invalidated, fresh/finalized storage,
advances generation without wrapping, and retires an exhausted slot. A configurable
nonzero generation limit permits exhaustive reduced-width tests. The supported
payload layout requires alignment no greater than max_align_t; generated layout
consumers must check their concrete layout against this contract.

Allocation failure publishes no reservation or new block. A successfully enlarged
directory may remain as spare capacity after payload allocation failure. Pool
storage disposal is a low-level storage operation: the ownership layer must first
finalize resources. Handles never survive disposal/reinitialization of a runtime.

For B historical blocks and S slots per block, reserve/release/lookup are O(1)
except amortized directory growth and O(S) block initialization. Directory memory
is O(B); payload memory is proportional to allocated blocks. Arithmetic is checked
before allocation, and SIZE_MAX is reserved as the free-chain sentinel.

[Pool tests](../../tests/unit/runtime/pool/main.c) cover stable addresses over many
growth steps, every issued historical handle with a three-generation limit, empty
and mismatched handles, and both growth allocation failures.

## Reclamation

Reclamation finds the last occupied block, rebuilds the free chain using only
retained blocks, then releases free blocks after that boundary. Empty historical
directory entries remain null permanently. Growth appends new slot identities;
it never fills historical directory holes. This permanently retires the released
identities, including unissued slots, and therefore preserves stale-reference
rejection without allocating a per-slot tombstone table. The small directory
history remains O(B) in lifetime block count. Reclamation is O(B*S), allocates
nothing, and cannot move a live payload or reclaim a partly occupied block.
Tests repeat reclaim/regrow with reduced generation limits and verify all old
handles as well as the address of a retained instance.
