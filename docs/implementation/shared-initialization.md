# Immutable construction and startup plans

T034 completes the static constant boundary under
[CONST-01](../../spec/language/declarations.md#const-01),
[CONST-02](../../spec/language/declarations.md#const-02) and
[CONST-05](../../spec/language/declarations.md#const-05).
The canonical specification defines behavior; this document explains the exact
implementation and its evidence. No native constructor executes during analysis.

## Resolve values before ordering them

Qualified sources identify a module's exact shared member. They cannot select its
parameters, results or body fields. Scalar/list spelling must match that member,
and qualification cannot bypass same-module initializer order. Unqualified shared
sources retain their full access and index chains. Native literal/clone support
and the transfer matrix are checked before graph construction; a constants-only
module supplies a namespace, not an instance type.

The [constant graph](../../src/semantic/constants/graph.c) has one vertex per
required constant declaration. Every declaration depends on its preceding local
constant. Each actual construction read adds an edge to its constant prerequisite,
including shared index operands. Source module dependencies alone add no value
edges; unused namespaces do not initialize themselves.

[Read collection](../../src/semantic/constants/reads.c) distinguishes direct
qualified reads from full named access chains. A named shared root is visited
through its access once, rather than twice through both the constant metadata and
the access metadata. Its index sources remain separate actual reads. Declaration
order and a value read can legitimately produce two edges to the same prerequisite
at different sites; these are retained as distinct reasons.

## Follow actual construction, not future execution

[Fresh-construction closure](../../src/semantic/constants/construction.c) follows
owned field defaults and literal construction, including each initialized owning
list's element construction. Empty lists create no elements. A supplied complete
value copy reads its source and does not rerun that type's default initializers.
The work list retains exact semantic type identities: the same freshly constructed
type always contributes the same declared defaults, so visiting it once per root
constant preserves the union of actual prerequisites. This terminates for every
finite resolved type graph without recursive C calls or depth heuristics.

An inferred field has its type and default storage but no declaration initializer.
Its later body assignment is not a constant-construction read. For example, if
`.worker` contains a first assignment from `.main.future`, constructing a shared
`.worker` does not execute that assignment or depend on `future`. Actual declared
field initializers would contribute their reads even inside an unexecuted branch.

Immutable constants cannot contain mutable-object handles. The closure rejects
explicit `&owned` and standalone `&` in any freshly constructed descendant's
initializer, at the original source site. The latter is important now that self
has valid static typing: a nested default's lexical self is still a mutable
instance identity. Empty reference cells have no target, and an assignment in a
module body is not executed during constant construction. Native payload purity
and absence of hidden resource identities remain the documented native author's
contract; there is no guessed C-body analysis or new capability marker.

## Deterministic plans and safe publication

The [relation service](type-relations.md) rejects actual value-read/order cycles
and emits the least dependency-ready full module-path/declaration key. Shared
storage of a module's own executable type is not itself an eager owned field and
does not create a cycle just by mentioning that type.

[Plan publication](../../src/semantic/constants/plan.c) follows this order and
assigns consecutive shared ordinals. Every entry retains its typed initializer
sequence and both forward and reverse links. After k successful insertions, the
forward list is exactly the first k ordered declarations, and reverse links are
its inverse. A failed allocation preserves owned partial records and leaves
`constants_valid` false; consumers require the completed stage.

The static plan describes full startup and reverse cleanup. Runtime lifecycle
code separately records completed construction and cleans only acquired credits;
a failed startup never starts the root. This task does not claim that native I/O
purity can be proved from a companion's C source.

For C constants, V reachable fresh types and F reachable field/default operations,
exact work-list membership costs O(CFV) plus exact constant-vertex lookup and graph
ordering. Work records use O(CV) storage; prerequisite records retain O(E) edges.
The graph service's order/cycle bounds and allocation checks apply unchanged.

## Evidence

Run `make -C tests/unit/semantic/constants`. Real loaded projects cover:

- qualified member/form errors and qualified/unqualified local forward reads;
- standalone self in nested parameter/result/state and reference-list initializers;
- empty reference defaults and unexecuted body assignments;
- indexed shared roots and index operands with exact prerequisite multiplicity;
- nested default/literal construction and complete-value copies;
- inferred-field defaults without executable-source dependencies;
- constants-only namespaces, unused namespaces, actual cross-module value cycles;
- consecutive startup ordinals, unique publication and exact reverse plan links;
- every allocation in nested closure, graph/cycle diagnostics and plan publication.

The relation and code-generation suites independently test type/eager separation
and execute shared indexed transfers. Counted allocation owners must return to
zero on every injected failure. Full strict tests and sanitizer checks supplement
the local assertions; compiler-time tests do not execute authored native code.
