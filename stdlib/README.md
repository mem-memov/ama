# Ama standard library

[TECHNICAL_SPECIFICATION.md](../TECHNICAL_SPECIFICATION.md) is the source of truth.
The [grammar](../EBNF.md) defines the language and the
[native value ABI](../VALUE_ABI.md) defines native capabilities and lifetimes.
Implementation work is tracked in [TODO.md](../TODO.md).

## Library boundary

Scalar types, literal interpretation, enumerations, conditions and indices are
library capabilities. The compiler uses the fixed declared contracts instead of
assuming a universal integer payload or recognizing special English field names.
Public paths and action/outcome constants must be published by the library.
Example `.std.*` paths in language documentation illustrate these roles; they do
not reserve a canonical package layout.

Shared `@=` constants are immutable, copyable values initialized once before the
root. Library enumeration types interpret their literals and expose qualified
constant names. Distinct module types remain incompatible even if their native
tags have equal values. Domain functionality extends through modules, preserving
the core syntax and semantics.

## Modules and companions

Interfaces and handwritten C sources live under `stdlib/modules/`.
The current `.std.status` type stores an immutable-message `AmaStatus` value.
`.std.refusal` has one `.std.status` result containing the invoking instance’s
last acquisition refusal; the reader itself succeeds even when that saved code
is nonzero. Add this directory with `--root /path/to/stdlib/modules`. These are
public library paths, not language keywords. An executable
module has an extensionless Ama interface and may select `@C` with companion `.h`,
`.c` and `.abi` files. The fixed ABI requires default construction and destruction;
literal construction, cloning, condition and index conversion are explicit optional
capabilities. Never infer a protocol from helper names or guess a representation.

Qualify module-owned C types, functions, tags and objects with
`AMA_NATIVE(local_name)`. ABI records contain the local identifier; the generated
module context supplies its full-identity prefix. Use unique include guards and
contain other C macros. The [native namespace contract](../docs/implementation/native-namespaces.md)
documents the implemented naming facility and composed probes.

Submodules of `name` reside in `name+/`, while their logical identities omit `+`.
Dependency resolution loads transitive Ama modules once, checks their contracts
and includes required native dependencies in the generated project assembly.
Native entry bodies inline; internal native helpers may use C calls and the stack.

## Ownership and failures

Opaque native payloads own native resources only. Persistent Ama relationships
must be declared as Ama fields so generated copying and cleanup can see them.
Borrowed views cannot escape their operation. Follow the common status contract,
partial-construction cleanup, independent cloning and at-most-once destruction.

Lists use the fixed four-input/three-output interface and six actions specified
in the technical specification. Action and outcome names are typed library
constants, not language keywords. Conditions and indices use registered conversion
capabilities; list positions begin at zero. Acquisition refusal, ordinary list
refusal and technical failure retain their distinct contracts.

## Contributing and verification

Follow [CONTRIBUTORS.md](../CONTRIBUTORS.md). Define narrow interfaces, decompose
implementation into a readable tree of small operations, and test both individual
components and their real composition. Use exact algorithms with explicit domains,
invariants and mathematical/algebraic justification; avoid heuristic special cases.

Test literal boundaries, invalid conversion, ownership/copy laws, resource failure,
cleanup ordering and generated native signatures. Verify the module through the
same generated C assembly used by projects. Build with `make`, run tests with
`make test`, and clean test outputs with `make test-clean` from the repository root.
Acceptance evidence establishes implemented support; documenting a contract alone
does not establish it.
