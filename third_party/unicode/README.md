# Pinned Unicode input

Unicode 17.0.0 DerivedCoreProperties:
https://www.unicode.org/Public/17.0.0/ucd/DerivedCoreProperties.txt

SHA-256: `24c7fed1195c482faaefd5c1e7eb821c5ee1fb6de07ecdbaa64b56a99da22c08`.
Unicode License V3 is included in `LICENSE.txt` (upstream
https://www.unicode.org/license.txt). Property data retains its original notice.

Offline regeneration from the repository root:

```sh
gcc -std=c11 -D_POSIX_C_SOURCE=200809L tools/unicode/generate.c -o /tmp/ama-unicode-generate
/tmp/ama-unicode-generate third_party/unicode/17.0.0/DerivedCoreProperties.txt > /tmp/ama-xid-tables.inc
cmp src/source/unicode/xid_tables.inc /tmp/ama-xid-tables.inc
```

Copy the regenerated output only when deliberately updating pinned input. Builds
use the committed table and require neither network nor a host Unicode library.
