# Module-qualified native C symbols

Boundary: `src/native/namespace/` and the native-namespace category in
`src/source/identity/`. Implements NAME-05's naming facility and PROTO-01/11's
module-local binding interpretation. It emits naming contexts and qualified
symbol uses; it is not a complete project C backend or native dependency loader.

## Inputs, ownership and effects

Register a complete logical module identity in an `IdentityRegistry` using
`IDENTITY_NATIVE_NAMESPACE`, with no member components. Registration uses the
existing versioned encoding, category byte 7, SHA-256 and full-key collision
rejection. The registry owns the resulting `ama_n_<64 hexadecimal digits>` prefix.
Identity source spans and local-function spans borrow their source managers.

`native_namespace_begin` emits a module context defining `AMA_NATIVE(local_name)`
as that prefix followed by `_` and the local C identifier. A preprocessor error
rejects an already-active context. The generator brackets each companion inclusion
and its associated typed uses with begin/end; the same module receives the same
prefix for its header and implementation. `native_namespace_end` undefines the
temporary macro. Macro arguments participate directly in token pasting, retaining
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
