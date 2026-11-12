# Source storage

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

Boundary: `src/source/text/source.h`, `text/` and `io/`. Implements SRC-01,
SRC-03 byte preservation and NAME-03 source coordinates, not tokenization.

A manager owns separately allocated immutable source records, path copies, bytes
and line tables. A span is a borrowed record pointer and half-open byte offsets;
all survive additions and expire on manager destruction. Input views need only
survive the add call. Allocator callbacks must have malloc/free semantics,
including accepting NULL on release, and remain usable for the manager lifetime.
The extra zero byte is convenience storage, never part of the source length.
Embedded NULs remain data; later lexical validation decides their admissibility.

Allocation, size and I/O failures publish nothing and preserve the output value.
The file reader closes its stream on every path, preserves an earlier failure,
and releases temporary storage. Concurrent modification of an input file is an
external I/O condition: the source is precisely the bytes successfully read,
not a promised filesystem snapshot. Loading itself does not validate UTF-8.

Construction allocates a zero-initialized private record, then path, bytes and
line table. Each failure releases this private ownership set. Only success links
the complete record. Induction over additions proves that existing records and
views never move. Destruction removes one record per iteration and releases its
four allocations. Checked arithmetic rejects unrepresentable storage sizes.

Line starts are 0 and the offsets immediately after LF, strictly increasing.
Binary search maintains the interval containing the greatest start <= offset.
The interval decreases until its lower endpoint is that predecessor; line and
byte column are respectively its index + 1 and offset - start + 1. EOF is valid,
including the empty final line after LF. Excerpts exclude LF and its preceding
CR; a bare CR remains source data. Columns count bytes, not terminal cells.

Adding n bytes and p path bytes costs O(n+p) time and storage; locating one offset
costs O(log l) for l lines. Geometric read buffers cost O(n) total copying and
storage. Record teardown costs O(number of sources). Manager operations require
exclusive access; there is no shared mutable compiler singleton.

Evidence: `make -C tests/unit/source/text` passed with C11, pedantic and Werror.
Tests preserve Unicode/NUL/CRLF bytes across 100 additions, compare every offset
against a linear line model, inject every allocation failure for add/load, and
exercise missing files. This component itself owns source storage, not a compiler project.
