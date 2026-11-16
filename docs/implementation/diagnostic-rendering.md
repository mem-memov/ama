# Source excerpts and C line mappings

Boundary: `src/diagnostics/render/` and `src/source/mapping/directive.h`.
Implements COMP-04/05 and NAME-03 using the independent structured record and
immutable source components. No generated-C diagnostic needs reparsing.

Rendering validates a complete diagnostic before output. It emits physical
file:line:column, a stable code, the message, optional original subject, every
source line intersecting the half-open span, byte offsets, cause/expected/help,
then related names/locations. Empty/EOF spans still show their containing line.
A span ending at a new line's start does not spuriously include that next line.
CRLF line terminators are omitted from excerpts without modifying stored bytes.

Columns and ranges count UTF-8 bytes. Excerpts deliberately use numbered lines
and numeric ranges instead of pretending terminal cell widths are byte widths.
Chinese characters and combining sequences therefore need no locale-dependent
width guesses. Well-formed Unicode remains recognizable; malformed bytes and
C0/C1 controls are escaped bytewise as `\xHH`. Structured source views retain the
original bytes even where printed excerpts require escaping. The text format is
for people; future editors should consume codes/spans directly, not parse it.

For each selected line the renderer writes precisely its source content interval;
advancing beyond its LF strictly increases the line index. Induction from first
to last selected line proves coverage and termination. UTF-8 output either writes
one valid scalar or escapes one byte, so its offset always advances. No compiler
heap allocation is needed, including error reporting after allocation failure.
Host stdio may itself buffer/allocate; supported stream failures return I/O error.
The stream is borrowed and flushed, never closed; a mid-write failure can leave
partial diagnostic text but cannot be reported as successful rendering.

Work is O(message/path bytes + excerpt bytes + related notes), plus line-table
searches O(selected lines * log line count). Auxiliary storage is constant, aside
from host stream buffers. No heuristic excerpt truncation discards source context.

C mapping emits `#line` with a positive C11 line <= 2147483647 and the original
physical UTF-8 path supported by the verified target. It validates coordinates/encoding before writing. Quotes and
backslashes are escaped; supported controls use fixed three-digit octal escapes to avoid
following-digit ambiguity. Question marks use `\?`, preventing C11 trigraph
rewriting. Unicode bytes remain unchanged. Callers check final flush/close errors;
the writer checks immediate failures. Output generation itself remains a later
compiler stage; this is its source-mapping primitive.

Evidence:

- `make -C tests/unit/diagnostics/render` checks exact Unicode output, related
  locations, multiline/EOF/empty ranges, malformed bytes, CRLF, controls, rejected
  invalid records with no output, `/dev/full`, and escaped C directives.
- `make -C tests/integration/source/diagnostics` composes real source discovery,
  identity registration, retained spans and rendering. Actual ambiguity and invalid
  UTF-8 errors match handwritten expectations. A separate context record exercises
  the identity-to-diagnostic handoff; it is not a syntax checker.
- The integration suite writes a C mapping probe, compiles it with the configured
  compiler and explicit UTF-8 character sets, and runs it. `__FILE__` is compared
  to an independent numeric byte array and `__LINE__` to the expected source line.
  Its path includes Chinese text, quotes, backslash, tab, LF and a trigraph sequence.
- Source, diagnostic and composed suites passed ASan/UBSan with
  `ASAN_OPTIONS=detect_leaks=0` and non-PIE sanitizer builds. LeakSanitizer is not
  certified in this ptrace-supervised environment.
- The full `make -j4 test` passed with C11, pedantic, Werror and NDEBUG; test flags
  restore assertions with UNDEBUG. That recorded source/diagnostic stage had 33 explicit
  scenarios, including exhaustive Unicode domain and allocation-failure loops.

Source/rendering tests isolate diagnostic behavior. Current CLI checking and
[project emission](project-emission.md) compose it with the parser and dependency
closure. [Editor commands](source-tool-commands.md),
[execution diagnostics](stackless-execution.md) and the
[debugger client](debugger-client.md) supply distinct current consumers. The
initial [audit](audit/README.md) remains historical evidence; current per-file
contracts and named tests are in [ready.json](documentation/ready.json).

## T020 target round trips and allocation-denied reporting

The exact C mapping proof separates directive spelling from target preprocessing.
Quotes, backslashes and question marks receive a protective escape; supported
controls receive exactly three octal digits, so a following digit cannot extend
the escape. Every input byte is emitted once in its prescribed representation.
The compiled probe checks the resulting `__FILE__` bytes against an independently
emitted numeric array, and checks `__LINE__` at every byte boundary in an LF/CRLF
source including EOF. It enables C11 trigraph processing explicitly and includes
all non-NUL ASCII bytes supported by the target, control-following digits,
Unicode and both potentially active trigraph prefixes.

GCC 15.2.0 has one demonstrated exception: a CR in the mapped physical filename
becomes a raw CR in its `__FILE__` macro expansion, causing invalid C tokens even
when the directive used the correct octal escape. Under SRC-03's unsupported-path
rule, physical source roots containing CR now fail catalog construction with
`MODULE_UNSUPPORTED_PATH`, retaining an owned copy of the original path. Ama
module components already exclude CR. The mapping writer independently rejects
such filenames before writing, so standalone consumers cannot accidentally emit
a mapping unsafe for the verified target. No path is renamed or normalized.

The allocation-denial test constructs source ownership with a counted allocator,
then forbids all further compiler allocation. A static out-of-memory record, an
original Unicode excerpt and an EOF mapping still render, with the exact allocation
request count unchanged. Teardown returns the live allocation count to zero.
This proves the compiler-side fallback property; host stdio buffering remains a
separate platform responsibility. Invalid UTF-8 paths, unsupported CR paths and
out-of-range offsets write no bytes; an unbuffered `/dev/full` reports the mapping
writer's immediate I/O failure rather than deferring it to the caller's flush.

Evidence: `make -C tests/unit/diagnostics/render`,
`make -C tests/unit/source/module` and
`make -C tests/integration/source/diagnostics`. Their additional cases are mapped
to maintained files in the documentation coverage registry. The existing exact
human-output expectations, related notes and malformed-byte escaping remain part
of these independently runnable suites.
