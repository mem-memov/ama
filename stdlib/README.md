# Standard-library implementation

The [product specification](../spec/README.md) is authoritative, including the
[grammar](../spec/grammar.md), [library boundary](../spec/language/library.md) and
[native interface](../spec/implementation/native-interface.md). This document
records retained source and executable evidence; it does not define another
library API. [TODO.md](../TODO.md) tracks conformance work.

## Required library boundary

Standard-library paths begin with `..`; ordinary project roots begin with `.`.
Literal interpretation, copying, conditions and indices are declared module
capabilities. The compiler uses checked protocol identities and signatures,
without assuming a native numeric payload or inferring protocols from C names.
The canonical `..integer` type and its exact `from_size` constructor are required
by [LIST-04](../spec/language/lists.md#list-04) and
[PROTO-03](../spec/implementation/native-interface.md#proto-03). T085 validates
that library, including its full host-size range.

Lists expose six typed intrinsic methods under LIST-04, not action constants or
retained completion/outcome fields. Their method frames, membership records and
mutable traversal are still T075–T084. Expected application errors use the one
condition-capable failure value with its own description under
[ERROR-01](../spec/implementation/diagnostics.md#error-01), implemented in T086.
Technical native status and acquisition refusal remain separate outcomes.

## Retained sources and probes

Interfaces and handwritten C companions live under `modules/`. The current
frontend still uses the older root/path model recorded in the
[audit](../docs/implementation/audit/README.md); its scalar fixtures are inputs
to the forthcoming migration, not a published substitute for standard-root
identity or the canonical integer/failure APIs.

| Retained physical source | Existing probe behavior |
|---|---|
| `modules/std/index` | Checked nonnegative `size_t` literal, clone, index and condition hooks |
| `modules/std/condition` | Boolean literal, clone, condition and index hooks |
| `modules/std/status` | Common immutable-message `AmaStatus` representation |
| `modules/std/refusal` | Read the immediate caller's saved acquisition-refusal status |
| `modules/std/index+/next`, `add`, `before` | Checked successor, addition and comparison compositions |

The compiled scalar suite uses these fixtures through the old frontend's explicit
`--root` option. It verifies nominal type checking, malformed/overflowing literal
rejection, declared field ordering, named inputs independent of text order and
omitted inputs receiving defaults. The arithmetic probes return their sole result
first and report overflow as technical status. Their acceptance does not establish
`..integer`, failure-value conventions or current grammar support.

## Native construction and ownership

An extensionless Ama interface may select `@C` and have `.h`, `.c` and `.abi`
companions. The six ABI capabilities and their requirements are defined in the
canonical native-interface chapter. [The implementation ABI guide](../VALUE_ABI.md)
and [namespace contract](../docs/implementation/native-namespaces.md) explain the
checked C boundaries and their tests.

Qualify module-owned C names with `AMA_NATIVE(local_name)`; ABI records contain
local identifiers and the generated context supplies their complete identity.
Use unique include guards and contain temporary C macros. `name+/` is physical
submodule storage, while logical identity omits `+`. Generated projects load
transitive dependencies, inline native entry bodies and permit normal C calls
inside native helper code.

Opaque native payloads own native resources. Ordinary persistent Ama relationships
must occupy declared Ama fields, visible to copying and cleanup. Borrowed views
cannot outlive their operation. The separate compiled-component resource exception
is specified by [EMBED-07/08](../spec/implementation/c-library.md#embed-07); that
exception and compiled-library embedding remain open implementation tasks.

## Verification

Run `make -C tests/unit/codegen/library` for compiled scalar compositions and
`make test` for the recursive component/integration tree. Follow
[CONTRIBUTORS.md](../CONTRIBUTORS.md) for decomposition, Doxygen rule links,
per-file tests and failure/lifecycle checks. Use `make docs` for generated file
reference pages. Current acceptance limits remain explicit in the audit and TODO;
no library representation is inferred from a translated diagnostic message.
