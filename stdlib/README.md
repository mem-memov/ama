# Standard-library implementation

The [product specification](../spec/README.md) is authoritative, including the
[grammar](../spec/grammar.md), [library boundary](../spec/language/library.md) and
[native interface](../spec/implementation/native-interface.md). This document
records retained source and executable evidence; it does not define another
library API. [TODO.md](../TODO.md) tracks conformance work.

## Canonical scalar value

Standard-library paths begin with `..`; ordinary project roots begin with `.`.
The maintained [`..integer`](modules/integer) type is signed arbitrary precision.
Its six declared protocols provide default zero, independent clone, destruction,
counted decimal literals, mathematical nonzero truth and checked host-size index
conversion. Shared `zero` and `one` constants have that same nominal type.

Its additional header-declared `from_size` constructor covers every value from
zero through `SIZE_MAX`, without decimal formatting or default-then-overwrite.
[List length](../spec/language/lists.md#list-04) returns this ordinary owned type;
negative/above-host-range integers remain valid values but refuse index conversion.
The [scalar implementation contract](../docs/implementation/scalar-library.md)
explains literal forms, representation, allocation failures, proof and tests.

Configure the library explicitly:

```sh
./ama --check project .main --standard-root stdlib/modules
./ama --emit project .main --standard-root stdlib/modules --out output --sdk src --compile
```

The compiler recognizes validated nominal identities and protocol signatures.
A project `.integer` remains a different type and gains no fixed size bridge.
There is no implicit numeric payload or conversion for unrelated modules.

## Other scalar types

| Public type | Literal range | Truth | Checked index |
|---|---|---|---|
| `..integer` | Optional minus and decimal magnitude, limited by storage | Mathematical nonzero | Exactly `0..SIZE_MAX`; other values refuse |
| `..std.index` | Decimal `0..SIZE_MAX`; overflow is a constructor failure | Nonzero | Stored nonnegative value |
| `..std.condition` | Decimal zero or one | Stored Boolean value | Zero or one |

The bounded types also own shared `zero`/`one` or `false`/`true` constants and
independent scalar clones. They are distinct nominal types. The modules
`..std.index.next`, `..std.index.add` and `..std.index.before` provide checked
successor/addition/comparison probes with ordinary typed interfaces. Their
expected-failure result convention is aligned in T087 after the failure-value
foundation; current overflow remains a technical status in these probes.

## Collections and remaining library work

Lists expose six typed intrinsic methods, fixed membership identities and live
mutable traversal. [Execution and continuation copying](../docs/implementation/list-traversal-execution.md)
are implemented and independently tested. Intrinsic methods use typed inputs,
not constants selecting an action or persistent completion fields.

## Application failures and text resources

[`..failure`](modules/failure) is one copyable condition-capable value with a
public owned `description` result of type `..text`. Default construction is
false/empty. Condition conversion reads its discriminator and retained reads
preserve its description. `..failure.make` accepts one text `message` and returns
one present `failure`; it has no separate success flag. Calling a failure value
with `!` explicitly clears it to false/empty.

[`..text`](modules/text) owns counted UTF-8, including embedded NUL. Native
cloning acquires one nonwrapping share of immutable backing; replacement prepares
new bytes before releasing old storage. Different values retain distinct Ama
identities. Empty text allocates no backing, and read-only truth means nonempty.
Allocation/share exhaustion remains a technical error, never application data.

`..std.refusal` returns its immediate caller's last refusal as `..std.status`.
This is read-only ordinary data with the common C status representation; a
nonzero copied status does not make the reader fail. These immutable diagnostic
message views remain separate from the failure value's owned description.
The [failure/resource implementation contract](../docs/implementation/failure-values.md)
explains states, native producer helpers, sharing invariants and failure evidence.
T087 aligns arithmetic/composition results with this outcome convention.

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

Run `make -C tests/unit/native/abi/integer` for native integer invariants,
`make -C tests/unit/codegen/scalar-library` and `make -C tests/unit/codegen/library`
for actual standard-root scalar execution,
`make -C tests/integration/standard-integer` for integer CLI behavior,
`make -C tests/unit/runtime/text-values` for text ownership,
`make -C tests/unit/codegen/failure-values` for complete outcomes,
`make -C tests/integration/failure-values` for their CLI example, and
`make test` for the recursive component/integration tree. Follow
[CONTRIBUTORS.md](../CONTRIBUTORS.md) for decomposition, Doxygen rule links,
per-file tests and failure/lifecycle checks. Use `make docs` for generated file
reference pages. Current acceptance limits remain explicit in the audit and TODO;
no library representation is inferred from a translated diagnostic message.

The [retained library/test/build contract](../docs/implementation/library-tests-and-build.md)
records file-level ownership, independent payload-fragment checks and byte-exact
fixture exceptions. Doxygen generates the actual C interface/file reference.
