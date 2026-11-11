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
physical UTF-8 path. It validates coordinates/encoding before writing. Quotes and
backslashes are escaped; controls use fixed three-digit octal escapes to avoid
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
  restore assertions with UNDEBUG. New section-3 boundaries have 33 explicit
  scenarios, including exhaustive Unicode domain and allocation-failure loops.

The CLI still exposes the implementation foundation only. Complete grammar
parsing is section 4; dependency closure and draft semantics are section 5;
shared check/build command orchestration is later in the plan. The technical
specification was not changed to fit these implementation boundaries.
