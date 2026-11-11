# Typed value ABI, version 1

This is the implemented C lifecycle contract for module-defined values. It is
the lifecycle and layout part of the typed ABI. The target declaration parser,
layout generator, and runtime are connected through project export APIs and
`--generate-type-layouts`, with tests that compile and run generated C. Executable
compilation still generates the existing numeric program and native signatures. Typed native call adapters are available to C callers. Executable Ama body
validation/calls, checked references, lists, and library-defined index/condition
protocols are subsequent work.

## Execution constraint and prototype status

The normative target in `EBNF.md` is stackless. Ordinary Ama modules execute as
labeled blocks with direct jumps and explicit continuation selection inside one
C execution function. `@C` implementations must be inlined, not invoked through
an ordinary C call stack. No substitute activation stack is allowed.

The function-pointer lifecycle callbacks and C-call adapters documented below
are implemented prototypes and do not satisfy that target. Their successful tests
establish layout, ownership, copying, and failure behavior, not stackless execution.
Reuse these data contracts while replacing ordinary module calls with labels and
specializing lifecycle/native operations so required calls cannot remain.

A "frame" here is explicit parameter/result data. Prepared and retained data may
remain useful in a stackless backend; they are not C stack activation records.
Mandatory native inlining and treatment of transitive library calls still need
implementation and supported-toolchain verification. Plain `inline`/LTO is not a
guarantee. The compiler's own implementation is outside this execution constraint.
The priority replacement work is recorded in `TODO.md`.

## State belongs to its owner

An Ama type describes the state of one module instance. Its C representation can
be a small scalar, a structure of owned children, or a native wrapper owning a
resource. The code implementing operations is shared; each state is independent.

`AmaType` in [value.h](src/runtime/value.h) describes size, alignment, identity,
and lifecycle callbacks. `AmaValue` pairs a descriptor with an address supplied
by its caller. It is a temporary C view of storage, without ownership or lifetime
tracking. It must never be exposed as an Ama reference or stored as a persistent
link between modules.

Storage can be a local variable, an embedded C structure field, or heap memory.
The ABI does not require an allocation for each numeric value. It currently
accepts fundamental C alignments up to `_Alignof(max_align_t)`; a native type
requiring greater alignment must use an ordinary aligned wrapper with an owned
payload, or wait for an explicit extension.

## Type identity

Each generated module state defines one immutable canonical descriptor. Its
identity is `source_identity_hash(normalized_module_path, NULL)`; generated C
names follow the same hash naming rules as other compiler entities. The
normalized UTF-8 path remains available for diagnostics. Version 1 is an
in-process ABI for code built with the same C toolchain and target, not a
serialization format or a stable interface across arbitrary shared libraries.

Runtime copies require the same descriptor address. Equal sizes, paths, or
hashes do not independently authorize access to a foreign layout. The compiler
must reject identity collisions before generating descriptors. Descriptor and
record metadata must remain alive and unchanged while any value uses them.

## Lifetimes and callbacks

Storage has two states: uninitialized or live. The caller tracks this distinction.
Zero-filled memory does not automatically contain a live value. Fresh construction
requires sufficiently large, correctly aligned, uninitialized storage.

| Callback | Input storage | Success | Failure |
|---|---|---|---|
| `initialize` | Fresh destination | Live default value | Release partial resources; destination stays uninitialized |
| `initialize_literal` | Fresh destination and borrowed bytes/length | Live parsed value | Release partial resources; destination stays uninitialized |
| `initialize_copy` | Fresh destination and live source | Independent owned copy | Release partial resources; source unchanged |
| `move` | Fresh destination and live source | Live destination; source becomes uninitialized | Cannot fail |
| `destroy` | Live state | Releases owned resources; state becomes uninitialized | Cannot fail |

`initialize`, `move`, and `destroy` are required. A missing copy callback reports
`AMA_STATUS_COPY_UNSUPPORTED`; a missing literal callback reports
`AMA_STATUS_LITERAL_UNSUPPORTED`. Resource-owning types must never substitute a
pointer copy for an unsupported independent clone. Self-replacement succeeds
without invoking a clone callback, even for a noncopyable resource.

Literal callbacks receive immutable raw bytes plus an explicit length. The
buffer is borrowed only for the callback; store a copy if needed. The length,
including embedded zero bytes, is authoritative. Empty data may have a null
pointer. Numbers, text encoding checks, range, and literal errors belong to the
type implementation. A rejected literal returns `AMA_STATUS_LITERAL_INVALID`.
The runtime does not interpret numeric text. Mapping status to an Ama source
location remains future work. Generated initialization calls carry `#line`
mappings, but returning a status does not yet attach a runtime source location.

Move transfers ownership without allocation or failure. It must repair pointers
into its own state when the address changes. A native object that cannot itself
move can reside in an owned allocation behind a movable wrapper. Callback code
must not publish temporary state addresses or retain another module's borrowed
storage. Native code is trusted to respect these rules; the ABI does not make
arbitrary C casts safe.

`ama_value_initialize_copy` and `ama_value_move` reject overlapping states,
including identical source/destination addresses. `ama_value_destroy` requires
a valid live value and must be called exactly once per lifetime. It has no
failure return; it does not check whether the caller already destroyed a state.

## Replacement preserves the old value on failure

`ama_value_replace`, `ama_value_replace_literal`, and `ama_value_reset` operate on
live destinations:

1. Validate descriptor, alignment, and operation support.
2. Allocate temporary storage and construct the proposed value there.
3. If construction fails, free the temporary storage and return its status.
   The callback has already released its partial resources; the destination
   remains live and unchanged.
4. Destroy the previous destination and move the prepared state into its place.
5. Free temporary storage without destroying the moved-from state.

After successful preparation, commit has no fallible operation. This also lets
a literal replacement read bytes owned by its old destination: the bytes are
consumed before that destination is destroyed. Ordinary nonoverlapping copy
sources remain unchanged. Self-copy replacement is a no-op.

This guarantee covers owned state and resources under the callback contract.
It does not roll back arbitrary native I/O or mutation of unrelated objects.
Finalization cannot fail; explicit operations such as flushing a file must
report failures before final cleanup. Multi-result call transactions and
reference invalidation are not yet defined by these single-value operations.

## Composition

[record.h](src/runtime/record.h) supplies common adapters for owned structure
fields. An `AmaRecord` describes fields in declaration order using `offsetof`
and each child's type descriptor. Generated code must use actual C layouts,
including their padding; the runtime does not invent a second layout system.
Empty states use a nonzero C representation and a zero-field record.

The adapters validate field sizes, offsets, alignment, and overlap before
constructing a record. They initialize and clone children in declaration order.
On failure they destroy only successfully constructed children, in reverse
order. Normal destruction also runs in reverse order. Move delegates to each
child's move callback. Padding has no value semantics and is not copied.

Record layouts must be acyclic; the compiler layout planner rejects recursive
inline ownership. Descriptors are trusted compiler/native metadata,
not arbitrary untrusted input. The adapters do not detect recursive metadata
or validate whether a supplied C address really has the advertised capacity.

These adapters establish default initialization and owned composition. Generated
declaration-specific initialization applies literal and named defaults. Reference
handles and lists need their own adapters and lifetime
contracts; they must not be represented as raw owning pointer copies.

## Generating structures and descriptors

`emit_type_layouts` in [types.h](src/codegen/types.h) accepts a batch of
`TypeModule` entries and writes a header plus one C implementation. Each entry
provides a normalized module identity and either an AST whose declarations have
passed validation, or a native header supplying its state typedef and canonical
descriptor. The generator does not invent a layout for an opaque native type.
Native provider headers declare `ama_state_<hash>` and `ama_type_<hash>` using
the same module hash as generated types; their companion C defines the descriptor.

For each generated module it emits:

- `ama_state_<hash>` containing only permanent body declarations, including
  declarations nested in control blocks. Their initialization belongs to instance
  creation and does not depend on entering those executable blocks.
- `ama_frame_<hash>` containing parameter/result declarations in interface order.
  The frame is separate storage. Native C invocation uses the protocol below;
  executable Ama body calls and their result delivery still require integration.
- External `ama_type_<hash>` and `ama_frame_type_<hash>` descriptors with C
  `sizeof`, `_Alignof`, and `offsetof` expressions, plus internal field metadata.
- Initialization adapters that construct each field from its default, raw literal
  bytes, or an independent copy of an earlier field in the same structure.
  Failure unwinds only successfully initialized fields in reverse order.

State types are emitted after their owned dependencies. All state layouts are
complete before frames are emitted, so a module may accept or return its own
state type without creating an inline ownership cycle. Direct and indirect
cycles between permanent owned fields are rejected before either output stream
is written. Validation also checks type providers and generated name collisions.
Reference and list declarations are explicitly rejected until their ABI exists.

The descriptor for a state uses the module identity hash. Frame descriptors use
`source_identity_hash(module_path, "$frame")`; `$frame` is an internal reserved
identity, not a permitted source variable name. All generated identifiers follow
the hash naming scheme. Unicode source filenames and lines remain in `#line`
directives for fields and initialization calls. Literal payloads are emitted as
byte arrays with explicit lengths, without C string escaping or termination rules.
Empty structures have one reserved byte and zero metadata fields.

Initializers crossing the frame/state boundary are currently rejected. Choosing
when a temporary parameter could establish permanent initial state needs a
separate language decision. Copying a state copies its permanent owned fields;
the layout API does not include a saved call frame in that copy.

The header has an include guard and external descriptor declarations. Define
these descriptors in exactly one generated implementation per program; do not
include the implementation as a header or generate overlapping batches into
multiple translation units. Native providers must likewise expose a single
canonical descriptor. Planning and validation occur before output; an I/O failure
can leave partial output streams, which the caller must discard.

## Typed native calls

Native entries with a validated interface AST now also receive frame layouts,
frame descriptors, a native prototype, a call schema, and an inline invocation
adapter. Layout-only native providers passed to the C API without an AST still
provide only a state type. Project export supplies interfaces for all implemented
native modules, including those without parameters or results.

The generated signature has this shape (hash placeholders are illustrative):

```text
AmaStatus ama_typed_native_<module>(
    ama_state_<module>* state,
    const ama_state_<input-type>* input,
    ama_state_<output-type>* output);
```

Parameters follow interface order, then results. A zero-argument function still
receives its persistent state pointer. The technical `AmaStatus` is an ABI error
channel, not an Ama result value. Native code includes `ama_types.h` so the C
compiler checks its definition against the generated declaration. This namespace
is separate from the existing numeric `ama_native_<hash>` functions.

Inputs denote independently copied, live parameter values in prepared frame
storage. The `const` pointer forbids replacing or destroying the parameter
wrapper through that view; it is not deep immutability of a native payload.
Results denote live slots initialized with declared defaults. Replace a result
through its type's lifecycle operations; never overwrite an owning pointer with
a borrowed one. Every slot must remain live on both success and failure, because
the adapter owns frame cleanup. Native code must not destroy its supplied state,
retain frame addresses, or free resources owned by other values.

[call.h](src/runtime/call.h) describes the runtime protocol:

1. `ama_call_execute` verifies descriptor identities, nonoverlapping state/frame
   storage, input positions, and duplicate bindings before preparation.
2. Initialize a fresh frame, including parameter and result defaults. Named
   result defaults derived from parameters therefore see declared parameter
   defaults at this stage, before explicit inputs replace them.
3. Copy explicitly supplied `AmaCallInput` values into their parameter slots.
   Inputs may be borrowed from the old retained frame; copies finish before it
   is replaced. Missing bindings keep their defaults.
4. Invoke native code with the persistent state and prepared slots.
5. On failure destroy the prepared frame. On success destroy the previous
   retained frame and move the prepared frame into its place without further
   fallible operations.

The caller initializes state and the retained frame before the first call and
owns their final destruction. The generated `ama_invoke_<hash>` wrapper selects
canonical descriptors and the `ama_call_schema_<hash>()` metadata. These inline
schemas may have separate addresses in different translation units; state and
frame type descriptors remain canonical external objects.

`ama_call_read` copies one saved result into a live destination without resetting
or executing. Before the first call it reads the initialized result default.
A failed read preserves its destination. It rejects a destination inside the
retained frame, so reading cannot accidentally mutate saved slots. Multiple
result receivers are not yet one transaction: a future Ama call adapter must
stage them together if the language requires all-or-nothing delivery.

A failed call preserves its old retained frame, but mutations of persistent
state and native I/O are not rolled back. This is synchronous C invocation, not
a checked object-reference, busy-state, or concurrency protocol. Trusted native
code must obey the lifetime contract. Runtime statuses do not yet include a
source-located failure report; `#line` mappings on prototypes and initialization
help C diagnostics but do not substitute for runtime error propagation.

## Exporting layouts from a project

`./ama --generate-type-layouts <root> <output-directory>` connects scanning,
section splitting, typed parsing, module-type resolution, and layout generation.
It creates `ama_types.h` and `ama_types.c`, including typed native prototypes
and inline call adapters. The export itself does not execute or semantically
validate executable bodies beyond the declaration and initialization checks
needed for layouts. Body syntax is parsed, but unresolved executable calls and
assignments are outside this export operation.

The current project section selectors are still `@Ama` and `@C`; target `@!`
and `@=` selectors remain separate migration tasks. Declared variables use the
target type-line and indented literal/named initializer syntax. Draft modules
are skipped without parsing their declarations, and references to draft types
are rejected. All implemented module types are exported, in deterministic
module-path order subject to owned layout dependencies.

A native provider lives in the handwritten sibling `<module>.h`, alongside
`<module>.c` and the extensionless `@C` interface. Both companion files must
exist. The header supplies the canonical state typedef and descriptor described
above. Absolute include paths keep providers resolvable when the generated
header is in another directory; source/provider files must remain available
when compiling the export. This command never creates or replaces native code.

`generate_type_layouts` is the project-only C API. Its companion
`generate_type_layouts_with_library` accepts an explicit typed module directory.
The CLI export is project-only and does not use `AMA_STDLIB`, because the
currently installed numeric library is not a typed library. Duplicate paths
across a project and an explicit library remain errors.

All compiler-owned syntax, type views, and temporary outputs are released after
each export, including failure. Parsing, declaration validation, provider checks,
and layout generation happen before publishing output. Existing files survive
these errors. Output publication rejects aliases of module sources, companions,
and the other output file. Publication is not a two-file transaction against
I/O failures; discard both outputs if a write fails. Use a generated directory
outside the project source tree.

## Building and testing

The implementation lives under `src/runtime/`. `make` includes it in
`build/stdlib/libama_std.a` and publishes headers under
`build/stdlib/include/runtime/`. Native consumers include `runtime/value.h`
or `runtime/record.h` and link the archive. Neither header depends on compiler
AST, symbol, or diagnostic types.

Run `make -C tests/unit/runtime` and `make -C tests/unit/codegen/types`. Tests exercise inline native scalars, raw
literal lengths, nested resource ownership, address repair on move, unsupported
clone operations, partial failure cleanup, unchanged destinations on failure,
and descriptor/layout rejection. The C fixtures illustrate the contract; they
are not new language-level standard library modules.
