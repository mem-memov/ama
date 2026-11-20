# Ama terminology

[Specification index](README.md)

This glossary defines how terms are used in this specification. Detailed
requirements in the referenced sections govern their behavior; the glossary
does not add language features or substitute for the detailed contracts.

## Modules, declarations, and values

| Term | Meaning in Ama |
|---|---|
| Module | A source unit identified by its full project/library path. An ordinary executable module defines instance state and shared code; a constants-only module provides immutable shared declarations. A static `@...` file declares a marker when empty, or a Type/Template equation producer when nonempty, without instances. |
| Module path | An absolute, relative or aliased reference resolved to a root kind and components: `/example/demo/v1.0.0/src/shop/item` selects a versioned ordinary package; `//integer` selects the standard library. It names a source definition, never an executable object; ordinary definitions supply Types/constants, empty static definitions supply markers, and nonempty static producers supply named exports. |
| Package version | One vendor/package/component sequence and version such as `v2.4.14-memory`, with package.ini, src and tests under [PKG-01](project/packages.md#pkg-01). A local packages copy overrides its whole dependencies copy under PKG-02. |
| Path alias | A scoped `~name` in package.ini or module.ini that abbreviates a logical path under [PKG-04](project/packages.md#pkg-04). It preserves the resolved identity and is distinct from a Type/Template name in a static graph. |
| Type | One nominal static identity denoting an ordinary module Type or a static marker under [TPL-07](language/templates.md#tpl-07). Only ordinary runtime-capable Types describe instances; exact identity determines compatibility. |
| Static type marker | A closed nominal Type declared by a file containing only `@...` and valid layout under [TPL-14](language/static-results.md#tpl-14). Its full logical module identity distinguishes it from other markers. It has no formals, named exports or runtime storage. |
| Declaration type expression | A structural named application, nested argument or named static export selection yielding one closed Type per concrete use under [TPL-15](language/templates.md#tpl-15). It introduces no standalone static graph. |
| Type placeholder | One atomic descriptive name beginning with `.`, with no slash or filesystem lookup. Free operand placeholders in ordinary input/output declaration type expressions introduce Type parameters under [TPL-01/TPL-02](language/templates.md#tpl-01); foreign labels do not. Static-module names follow the graph scope of [TPL-12](language/static-results.md#tpl-12). |
| Module template | An Ama module definition with static parameters, specialized and checked before executable state is constructed. Its distribution and concrete native boundary follow [TPL-13](language/templates.md#tpl-13); an open template is not a runtime object or a generic C entry. |
| Template source package | Ama source definitions and their required dependencies distributed for ordinary module resolution and later specialization under [TPL-13](language/templates.md#tpl-13). Concrete native dependencies retain their ordinary companion contracts; this is not a new binary template format. |
| Concrete specialization | The original template identity together with normalized complete bindings under [TPL-05](language/templates.md#tpl-05). Equal specializations share a type, while objects retain independent state. |
| Specialization creation path | A chain of constructor applications arising from instantiated definitions under [TPL-11](language/templates.md#tpl-11). It records why specializations are required, rather than structural containment of already supplied types. Returning to the same closed canonical vertex can close a finite graph; reaching a different specialization of the same source template is forbidden. |
| Template constructor | A compile-time constructor retaining its actual nominal origin, fixed bindings, captures and residual signature. Static modules receive, apply and forward constructors under [TPL-07/TPL-08](language/templates.md#tpl-07); usage requirements need not enumerate every residual parameter. It is not a runtime instance. |
| Partial substitution | Named binding of some residual constructor parameters while retaining the others and all captures under [TPL-08](language/templates.md#tpl-08). A returned partial Template differs from a missing producer input required to resolve before publishing its outputs. Ordinary runtime argument omission still means the declared default. |
| Static formal | An external Type or Template name used but not defined in an `@...` graph. Whole-graph binder collection determines its input role; usages impose compatible category and named-member requirements under [TPL-12](language/static-results.md#tpl-12). It occupies no runtime argument position. |
| Static result | A public immutable Type or Template name defined by an alias pair or a named producer-output receiver in an `@...` graph under [TPL-12](language/static-results.md#tpl-12). It occupies no runtime result field. |
| Static result alias | A public graph result name denoting an existing Type or Template without creating a nominal type. Its module-wide graph scope and repeated-projection equality follow [TPL-12](language/static-results.md#tpl-12). It is not a runtime variable. |
| Instance / object | One independently identified state of an executable module type. Many instances share the module's code but have separate fields and runtime metadata. |
| Value | The contents represented by typed storage. Owned-value copying constructs independent owned data; reference-value copying copies a checked target identity. |
| Declaration | An explicit name/type-expression pair under [TYPE-01](language/declarations.md#type-01), a receiver-inferred runtime field under TYPE-06, or a foreach-local reference under FLOW-05. Ordinary declarations have storage forms and optional initializers. Each explicit type expression must yield one closed admissible Type under TPL-15. |
| Receiver-inferred declaration | A simple assignment or output receiver that is the first textual occurrence of its local base name. Its field identity and explicit storage form are fixed lexically; its scoped type term may be refined by later declared connections before layout under TYPE-06. |
| Assignment-inferred declaration | A receiver-inferred declaration whose type comes from its assignment source; the source transfer executes whenever the assignment is reached. |
| Result-inferred declaration | A simple output receiver that is the first textual occurrence of its local base name; it declares persistent caller storage with the selected result's module type and matching explicit storage suffixes. |
| Foreach binding | A scalar reference receiving the current list-entry handle on each iteration. A compatible visible variable is reused; otherwise FLOW-05 declares a reference local to the loop body. |
| Base name | One complete, possibly multiword identifier, excluding storage markers and surrounding grammar separators. `item`, `item&`, `item[]`, and `item&[]` share one base name and cannot coexist as module-field declarations; FLOW-05 permits disjoint local bindings. |
| Scope | The lexical source region in which a declaration can bind uses of its name under NAME-06. Scope does not determine when storage is initialized or how long its value lives. |
| Visibility | Availability of a declaration to lexical name lookup at a source occurrence, subject to its scope and source-order rules. A visible declaration can still be invalid for the requested access, type, storage form, or initialization phase. |
| Shadowing | Introducing a distinct inner declaration that hides an already visible declaration with the same complete base name. Ama forbids shadowing; updating or reusing the existing variable is not shadowing. |
| Name word | One nonempty Ama identifier word under [SRC-03](language/source-and-names.md#src-03), using pinned Unicode identifier characters and the permitted internal ASCII hyphen spelling. Single U+0020 spaces join words into one Ama name; the words do not introduce separate symbols. |
| Storage form | Whether a declaration is an owned value, reference, owned list, or reference list, as marked by the absence/presence of `&` and `[]` on its name. |
| Runtime parameter | Runtime interface storage declared in ordinary `@<`, reinitialized at entry-start from its default or supplied typed input; literal inputs use CALL-07. Static-module inputs occupy no runtime positions. |
| Runtime result | A runtime value in an instance's ordinary `@>` interface, retained after execution, or LIST-04's ephemeral intrinsic output. Execution transfers use `>`; retained reads use `>>`; selected actions use CALL-09. Separate static graph exports allocate no retained value. |
| Result chain | A finite source tree of output-phase operations selecting named results of their enclosing operations. Each selected operation uses existing storage and shares the initiating caller's lexical context. |
| Persistent state | Runtime body fields initialized once at creation, including declarations nested in branches/loops, except FLOW-05's scoped foreach bindings. Persistent runtime fields survive ordinary calls and resumption. |
| Shared constant | An immutable declaration in the first optional `@=` section, shared once per concrete specialization and execution context, and excluded from instance ownership, reset, and copying. Its type may reuse interface parameters under [CONST-06](language/declarations.md#const-06). |
| Constant namespace | The independently selected concrete ordinary Type whose own `@=` members are read under [CONST-03](language/declarations.md#const-03). A path, determined scoped Type or selected concrete static export can supply it; selection never imports names or executes an instance. |
| Enumeration | A library-defined type with a finite set of accepted values, exposed through named `@=` constants. Different enumeration module types are incompatible; no special enum declaration operator is required. |
| Literal | Raw UTF-8 payload after `\|` in a declaration initializer or entry-call input, interpreted by the destination's declared type. It has no quoting, escaping, or multiline continuation. |
| Qualified constant source | A logical module path followed by the constant name, such as `//palette/red`, used directly as a typed value source without a local alias or executable module call. |
| Initializer | A declaration's optional source or literal specifying initial contents. It is distinct from executable assignment. |
| Draft | A UTF-8 source file with none of `@=`, `@!`, `@C` or `@...`. It is excluded from compilation and may contain unfinished declarations; it cannot be used as a type or entry. |
| Constants-only module | A complete file containing only the `@=` section and layout. It has no executable implementation, instance pool, or callable singleton. |
| Static module | A complete file beginning with `@...`. An empty graph declares a static marker under [TPL-14](language/static-results.md#tpl-14); a nonempty graph is a producer under [TPL-12](language/static-results.md#tpl-12), whose defined results are public and free names supply static inputs. Neither form has runtime instances or ABI. Static composition generates concrete modules under [CORE-06](introduction.md#core-06). |

See [source files and names](language/source-and-names.md) and
[declarations and scope](language/declarations.md) for their complete contracts.

## Ownership, references, and collections

| Term | Meaning in Ama |
|---|---|
| Ownership | Responsibility for the lifetime and destruction of an instance or native resource. Owning a child differs from merely holding a reference to it. |
| Ownership tree | An instance and its recursively owned descendants, including owned list elements. Non-owning references may form a separate graph and do not extend this tree. |
| Reference / handle | A non-owning, checked identity of a target instance: its slot and generation, with the target type statically known or explicitly carried. It is not a raw C address. |
| Empty reference | A handle with generation zero, including an all-zero handle, that selects no instance. |
| Stale reference | A previously issued handle whose target has been deleted or whose generation no longer matches. It must never resolve to a later replacement object. |
| Generation | A nonzero incarnation number for a live pool slot. Reuse advances it without wraparound; exhaustion permanently retires the slot. |
| List | An ordered, dynamically sized collection represented by a contiguous array of membership identifiers and checked membership records. Each record identifies its target; equal target handles can belong to different entries. Instance payloads remain in their module pools. |
| Owning list | A list that owns its element instances. Removal destroys an element subject to access/lifecycle checks. |
| Reference list | A list owning only its handle entries, not their targets. Removing an entry does not destroy the referenced instance. |
| Position / index | A zero-based place in current list order. It may change after insertion/removal; it is not an object's stable identity or pool-slot number. |
| Length | The number of current list entries. It differs from allocated list capacity and from the number of live objects in an element-type pool. |
| Capacity | Allocated storage available before growth is required. Reserved capacity does not itself create live elements or instances. |
| Membership / list entry | One independently identified occurrence in a list. Removing and reinserting the same target creates a different occurrence. See LIST-03 and LIST-13. |
| Traversal boundary | A position `q` separating entries excluded from a traversal's immediate future from the current suffix still to consider. Edits correct this position under FLOW-05; it is not attached to the last selected object. |
| Ordinary copy | Independent owned-data copying with internal-reference remapping and no copied resume positions. Reference-only assignment copies identity instead. |
| Continuation copy | Explicit `% source` copying of owned data and saved positions throughout the copy boundary, without busy flags or outstanding return links. |
| Copy boundary | The source ownership tree, or all owned elements/descendants in a whole owning-list copy. References to objects inside it follow the clones; external targets remain unchanged. |
| Clone context | Temporary runtime source-to-destination identity mapping used to construct a copy and redirect compiler-visible internal references. Native payload hooks do not inspect this graph. |

See [values and references](language/values-and-references.md) and
[lists](language/lists.md) for copying, identity, and the collection interface.

## Execution and runtime storage

| Term | Meaning in Ama |
|---|---|
| Runtime | Generated program machinery for pools, current-instance selection, access checks, lifecycle, control transfers, host outcomes, and diagnostics. It is distinct from the compiler. |
| Execution context | An isolated runtime lifetime with its own pools, constants, current-instance selection, diagnostics, and one Ama executor. Checked Ama identities are local to it. |
| Compiled component | An explicitly exported Ama dependency closure compiled as one generated C unit and packaged with a public C interface and target/ABI metadata under EMBED-01. A larger C application may link several components without merging their Ama state. |
| Concrete export facade | An ordinary module with no own free static parameters whose implementation uses closed specializations and presents a concrete export interface under [EMBED-01](implementation/c-library.md#embed-01). A facade is ordinary Ama source, not a new generic export selector. |
| Host control handle | A component-specific checked control token for a library context or exported instance. It is not a transferable Ama reference or access to private pool layout; EMBED-02 through EMBED-04 govern its origin, lifetime, and permitted operations. |
| Foreign-context resource | An opaque owned library-context resource used by an Ama native wrapper under EMBED-07. Its isolated context is managed through the foreign public API; the wrapper cannot hide references into its own or an ancestor context in that payload. |
| Executor | The single execution owner allowed to run Ama operations and change Ama state in one execution context. A native worker or external callback is not that executor. |
| Task | An explicitly scheduled computation whose state lives in an instance. Its library protocol distinguishes runnable work, event waiting, and completion; it does not change ordinary call semantics. |
| Reactor | Native event-waiting machinery that records native operation completions and makes them available to the context's executor without retaining borrowed Ama pointers. |
| Registration | A visible relationship between a task and one expected event, with an identity that distinguishes cancellation, restart, and late completions. |
| Pool | Per-executable-module-type heap storage for instances, allocated in blocks with stable live-slot addresses. It does not encode user list order or imply default objects. |
| Pool slot | Storage/identity location for one possible instance. Slot identity, liveness, and generation are distinct from the payload's contents. |
| Free list | Runtime chain of unused pool slots, linked through metadata in free slots for reuse before allocating more blocks. It is not a user-level list. |
| Refusal context | Empty initial status; own refusal saved before `:`; successful/reader calls do not clear it; nested-instance refusal does not overwrite caller; next own refusal replaces it; copied status remains unchanged; owned clones start with empty service context |
| Acquisition | Protected runtime access to a selected instance/collection for an operation, preventing conflicting access or destruction until release. Forming a reference with prefix `&` does not hold such protection. |
| Busy | An instance is running or waiting for an outstanding nested invocation; conflicting entry/resume/read is refused. |
| Waiting instance | A still-busy caller whose callee has not returned. Its data stays in its heap slot and its pending return chain remains active. |
| Yielded instance | An available instance with a saved position after `%` and no outstanding caller connection from the yielded invocation. It may be explicitly resumed. |
| Entry-start | An `instance !` operation that reinitializes the interface under STATE-02/CALL-07 and runs from body entry while preserving permanent state. |
| Resumption | An `instance %` operation that executes from the saved position without input bindings or an interface reset. |
| Retained-result read | An unmarked instance target with output-phase actions. It copies stored results through `>>` or selects them for CALL-09 operations or a FLOW-04 condition chain without executing/resetting the enclosing target. |
| Return link | Service metadata identifying the outstanding invocation's caller instance and return continuation. It is not an owning reference or a saved C activation. |
| Return continuation | A compiler-generated control location to which a callee returns, selected using explicit runtime metadata and labels. |
| Resume position | The next execution/control location saved by `%`, including information needed to continue inside branches/loops. It is separate from an outstanding return link. |
| Root instance | The one executable entry instance created by the host startup rule. It has no caller in Ama. |
| Host / launcher | The C program entering generated execution and receiving completed, yielded, or failed outcomes. The standard standalone launcher is one such host. |
| Stackless execution | Ama module invocation/return through labels and explicit instance metadata rather than C activations per module call. It does not prohibit stack use inside native C. |
| Positional placeholder | An empty argument line `<` or `>` that consumes an input/output position while keeping its input default or skipping result transfer. |

See [calls](language/calls-and-results.md), [continuations](language/continuations.md),
and [runtime storage](implementation/runtime.md) for these behaviors.

## Native code, lifecycle, and errors

| Term | Meaning in Ama |
|---|---|
| Native module | An Ama interface selecting `@C`, with a handwritten companion implementation and checked generated C entry. It has no own free Type/Template formals under [TPL-13](language/templates.md#tpl-13), while concrete Ama fields and declared references retain their ordinary contracts. |
| Native payload | Opaque per-instance C state owning native resources only. It must not hide persistent Ama references, instance addresses, or runtime control links. |
| Borrowed view | Temporary typed access to generated storage during an authorized native operation. It does not transfer ownership and cannot escape into retained native state or callbacks. |
| ABI | Application binary interface: the contract joining generated code and native implementations, including representations, signatures, lifetimes, and status handling. |
| `.abi` companion | A fixed UTF-8 binding table mapping the six symbols `+`, `-`, `\|`, `=`, `?`, and `[]` to C functions. It is not executable Ama source. |
| Native protocol | A fixed native capability/lifecycle contract: default creation, destruction, literal construction, cloning, condition conversion, or index conversion. |
| Constructor | A selected native `+`, `\|`, or `=` operation over fresh payload storage. Ama has no constructor section or implicit execution of child bodies. |
| Destruction / finalization | Ending resource ownership and lifetime through native `-` and generated child cleanup. It is distinct from normal completion of a module invocation. |
| Copy preparation | Construction and validation of an unpublished replacement and its reference fixups, before destroying the old destination. Failure preserves the old destination. |
| Replacement commit | The phase beginning with destruction of the old destination. Destruction errors are terminal and do not roll back a partly destroyed value. |
| Native status | The common C code/message/byte-length record. It reports technical results separately from declared Ama application results; code zero means success. |
| Acquisition refusal | Inability to start an entry, resume, or retained read, such as a missing/busy target. An attached `:` may handle it. |
| List-method preflight refusal | Inability to begin an intrinsic list method after preparing its inputs, such as a missing position or busy owned deletion subtree. It selects that method's `:` before mutation or results under LIST-10. |
| Technical failure | A failure such as allocation, native construction/cloning, or cleanup failure that ends current Ama execution with diagnostics and a failed host outcome, without exceptions. |
| Execution trace | The active Ama operation and caller-site chain rendered using source names/locations before return links are cleaned up. It is not object history or an automatic C backtrace. |

See [native interfaces](implementation/native-interface.md) and
[diagnostics](implementation/diagnostics.md) for their detailed contracts.

## Compiler and verification

| Term | Meaning in Ama |
|---|---|
| AST | Abstract syntax tree preserving parsed source constructs and spans before execution/storage lowering. |
| Typed IR | Intermediate representation with resolved types and explicit lifecycle, acquisition, result-transfer, continuation, and control-flow operations. |
| Source span | A source identity and location/range retained for precise diagnostics and mappings from generated operations to original code. |
| Backend | The compiler stage that emits checked C types, interfaces, storage, labels, dispatch, and debugging mappings from validated plans. |
| Qualified identity | A full module/member/category identity used for deterministic collision-checked hashing rather than a basename alone. |
| Acceptance criterion | Required observable behavior or invariant with evidence from implementation checks/tests. A documented target feature is not automatically implemented. |

See [compiler architecture](implementation/compiler.md),
[tools and conformance](implementation/tools-and-conformance.md), and the
informative [implementation roadmap](implementation/plan.md).
