# Module-qualified native C symbols

Boundary: `src/native/namespace/` and the native-namespace category in
`src/source/identity/`. Implements NAME-05's naming facility and PROTO-01/11's
module-local binding interpretation. It emits naming contexts and qualified
symbol uses; it is not a complete project C backend or native dependency loader.

## Inputs, ownership and effects

Register a complete logical module identity in an `IdentityRegistry` using
`IDENTITY_NATIVE_NAMESPACE`, with no member components. Registration uses version-two complete encodings with explicit root kind,
category byte 7, SHA-256 and full-key collision rejection. T017 now sends the
frozen project/standard multiword Unicode vectors through the current producer
and compares their exact bytes and full prefixes.
[Lexical layout](layout.md) and [module paths](module-paths.md) supply
separate parser/catalog integration evidence.
The registry owns the resulting `ama_n_<64 hexadecimal digits>` prefix.
Identity source spans and local-function spans borrow their source managers.

`native_namespace_begin` emits a module context defining `AMA_NATIVE(local_name)`
as that prefix followed by `_` and the local C identifier. A preprocessor error
rejects an already-active context. The generator brackets each companion inclusion
and its associated typed uses with begin/end; the same module receives the same
prefix for its header and implementation. `native_namespace_end` undefines the
temporary naming and error macros. Macro arguments participate directly in token pasting, retaining
the local identifier rather than expanding an unrelated object-like macro first.

`native_namespace_symbol` emits the corresponding qualified identifier when no
module macro is active, for example for a later typed call. Its local span must
already be a C identifier accepted by the ABI/C frontend. This emitter does not
parse C, infer capabilities or rewrite arbitrary declarations. The C compiler
checks typed uses and Unicode/UCN identifier equivalence. Headers must use unique
include guards; other native macros remain ordinary C authoring obligations.

Emission borrows inputs and allocates nothing. Invalid arguments/category return
false before output; output errors may leave an unusable partial artifact. The
owner must check final flush/close and discard failed generated output. Source
or identity allocation errors remain the source/registry contracts, without
publishing a valid-looking prefix on failed registration.

## Correctness and bounds

Let P(M) be a successfully registered module prefix. For distinct accepted module
identities, prefixes differ: unequal full encodings with equal digests are rejected,
never merged. Prefixes have a fixed width and a terminal delimiter. Therefore
P(M) + `_` + local-name determines its module prefix and remaining C identifier
unambiguously. Two companions may use identical local function/type/helper names
without cross-binding. C-equivalent spellings such as a Unicode scalar and its UCN
denote the same intended local C identifier, not different Ama module identities.

Begin/end contexts contain no instance state and do not affect Ama ownership,
copying or execution semantics. Ordinary C lexical scopes and external public
symbols retain their C meaning. Prefix generation is independent of filesystem
root, discovery order, and source language alphabet.

Registry bounds are in [identities.md](identities.md). Context output is constant
work; symbol output is O(local identifier bytes), with constant auxiliary space.
Both terminate subject to the output stream's I/O behavior.

## Acceptance evidence

- `tests/unit/native/namespace/`: category separation, Unicode module prefixes,
  context/symbol agreement, invalid member-bearing namespace keys, and direct
  output failure on the supported Linux target.
- `tests/unit/native/abi/hooks/`: the resource companion uses qualified types,
  hooks and inline entry in one C translation unit. Its fixed test context is
  the category-7 identity `.fixtures.buffer`; it does not claim an Ama backend.
- `tests/integration/parser/grammar/`: real catalog and ABI records feed two
  companions with identical local names/types into one C translation unit. All
  six typed hook uses compile and run; Unicode, UCN and dollar spellings survive,
  and the macro does not leak after either module context.

General project emission, semantic capability checks and mandatory companion
validation stay in their planned stages. A passing naming probe does not certify
arbitrary handwritten C resource or lifetime behavior.

Verification: the full release/Werror test tree and compiler build passed.
Focused lexer/parser, namespace, resource-hook and frontend integration suites
passed ASan/UBSan with `ASAN_OPTIONS=detect_leaks=0` and non-PIE builds; the
environment's ptrace restriction still prevents LeakSanitizer certification.
Affected public headers compile independently, and changed document links pass.

## Deterministic native error providers

T010 adds `src/native/status/providers/`, an isolated compiler planning owner.
Its inventory borrows native-namespace records from one live collision-checked
identity registry. That owner and all full key bytes outlive the plan. The plan
allocates only its private row table; it never changes registry records or caller
inventory. Build is a one-shot transaction. Invalid keys, duplicate complete keys,
allocation failure and namespace exhaustion leave `ready` false with no published
table; destruction releases private scratch even after failure. Duplicate failure
retains both conflicting records for compiler diagnostics.

Sort the full encodings lexicographically, using unsigned byte order and then
length. Assign namespace `index + 1` in that exact order. The capacity is 65535;
an excessive inventory returns LimitExceeded before dereferencing inventory or
allocating. Empty inventory is valid and allocates nothing. Lookup uses the same
full-key comparison, changes its output only on a match, and returns TypeMismatch
for an absent provider. Hashes, pointer addresses, translated display names and
filesystem discovery order do not determine numeric IDs. IDs are private to one
compiled component; this is not a stable cross-component/public ABI numbering
promise. The [compiled-library boundary](c-library-exports.md) supplies its own
checked component identity, export model and ABI compatibility key.

After establishing `AMA_NATIVE`, `native_namespace_status` emits a planned
nonzero namespace and `AMA_NATIVE_ERROR(reason, message, length)`. The native
author owns unique nonzero 16-bit local reason IDs and immutable program-lifetime
message bytes. The macro passes each argument once to `ama_library_status`, which
checks the pair and structural message view without allocation. For example,
the unchanged fixture companion writes:

```c
return AMA_NATIVE_ERROR(AMA_NATIVE(reason), "project", 7);
```

The generator rejects missing or duplicated error contexts through explicit C
preprocessor errors. Begin rejects either leaked temporary macro; end removes
both. Header and body inclusions use the same complete identity and planned ID,
even when their inclusion orders differ. The generator does not inspect native
C to infer reason IDs, parse local declarations, or rewrite arbitrary source.
[T055 generated provider emission](native-contract-composition.md) now calls this
planner before emitting the dependency closure.

## Ordering argument and additional acceptance

Heap construction establishes that each parent is at least its children. Sift
follows one strictly descending path, restoring that invariant after replacing a
root. Removing the maximum grows an ascending fixed suffix by one element; the
remaining prefix stays a heap. Finite shrinking heap size proves termination.
`root < count / 2` bounds child arithmetic before multiplication. Sorting costs
O(N log N * M) byte comparisons for N providers and maximum full key length M,
with O(1) sorting scratch beyond the O(N) row table. Binary lookup costs
O(log N * M). No recursive call stack, heuristic sort or hash injectivity is
assumed. Consecutive ordinals prove injectivity within the nonzero 16-bit domain;
combining one with a local reason leaves the core-code range disjoint.

`tests/unit/native/status-providers/` checks all six discovery orders of three
frozen root-aware Unicode/multiword vectors, equivalent full keys at different
addresses, absent/invalid lookup, duplicates, malformed inventory, allocation
failure, empty inventory, exact capacity arithmetic and an injected digest
collision before planning. Its separate sorting case exhausts every permutation
for sizes zero through eight; the invariant argument above covers arbitrary size.

`tests/unit/native/namespace/` now compiles and executes unchanged project-root
and standard-root companions with identical paths and local names for types,
hooks, helpers, constants and reason enumerators. Header order is reversed from
body order. Both clones preserve independent payloads, both errors retain their
messages and distinct provider codes, each macro argument is evaluated once, and
neither temporary macro leaks. Negative compilations require the precise missing,
duplicated or unclean-context diagnostic. Output validation covers zero IDs,
NULL streams and an actual failing stream. SDK status tests also cover maximum
pairs, zero fields and malformed message views.

T010 acceptance passes the full strict `-DNDEBUG -Werror` suite and focused
ASan/UBSan status, provider and naming leaves (leak detection disabled). Complete
coverage checks 739 maintained inputs with zero debt. Warning-free Doxygen checks
569 maintained file pages, 2350 interfaces and 286 required destinations; two
complete generations produce identical bytes. Counts record this task's evidence.
