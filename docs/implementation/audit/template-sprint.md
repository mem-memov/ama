# Template sprint implementation audit

T168 establishes the implementation boundary for the
[ordered sprint](../../../TODO.md#sprint-static-module-templates).
Input revision: 88f5867, clean before the task; audit started 2026-11-18.
This is implementation evidence and migration planning, not language authority.
The canonical contracts remain in [spec](../../../spec/README.md).

## Scope, method and current checkpoint

Inspect maintained compiler/native/test/build inputs only. Do not inspect the
compiler binary, object/build trees or test generated artifacts. Executing the
compiler and supported tests is permitted; generated behavior is observed through
those interfaces. Preserve the immutable initial audit and old release evidence.

| Step | Intended observation | State |
|---|---|---|
| 1. Trace actual models and consumers | Identify concrete-only assumptions and reusable invariants at real source interfaces | Complete |
| 2. Assign every gap to the sprint | Actual production/test homes, narrow handoff contracts and safe migration boundaries | Complete |
| 3. Characterize baseline behavior | Focused existing suites and independent minimal positive/gap probes | Complete |
| 4. Reconcile documents and accept | Current prose, routing, links and documentation gates agree; archive and commit T168 | Complete |

Completed 2026-11-18. The compiler build and all fourteen selected leaves passed
(sessions 53083 and 9678; /tmp/ama-t168-build.log and individual
/tmp/ama-t168-*.log files). Documentation gates also passed (session 95911).
The archive/queue edit is followed by final documentation regeneration before
commit; T169 is next. No compiler/test jobs remain active.

## Findings from source interfaces

1. [The frontend](../../../src/compiler/frontend.c) owns loaded sources before
   semantic state and plans, records initialization before fallible work and
   tears those owners down in reverse order. Preserve that lifetime contract.
   [Semantic orchestration](../../../src/semantic/analyze.c) currently runs
   names, native contracts, source-order inference, resolution, relations and
   constants. There is no intervening static specialization stage.
2. [AST declarations](../../../src/ast/ast.h) carry one type SourceSpan;
   [declaration parsing](../../../src/parser/declarations/declaration.c) requires
   parser_path on the next line. [Path recognition](../../../src/parser/access/names.c)
   admits one/two leading dots before an ordinary name. It cannot represent a
   placeholder, constructor signature, static result or nested type application.
   Add structural expressions in T170; do not encode them in path strings.
3. [Complete-name validation](../../../src/source/unicode/name.c) splits words
   and calls unicode_identifier; [token scanning](../../../src/lexer/tokens/name.c)
   independently scans XID scalar predicates. Neither implements internal hyphens.
   T169 must share a bounded word recognizer between validation and scanning.
   Catalog/logical-path validation, literal-boundary recognition, rename and
   relocation discovery consume these existing interfaces and need composition
   evidence. Raw scalar XID predicates retain their pinned Unicode meaning.
4. [Semantic models](../../../src/semantic/semantic.h) attach exactly one
   concrete SemanticType to a loaded ProjectModule and concrete types to symbols,
   sources and bindings. [Type lookup](../../../src/semantic/resolve/types.c)
   compares source logical paths. These are ordinary nominal types, not a
   representation of scoped static formals or multiple specializations.
   Preserve ordinary identity; add scoped terms and a separate specialization
   identity rather than forging ProjectModule files.
5. [Receiver inference](../../../src/semantic/inference/resolve/receiver.c)
   classifies fresh receivers and immediately assigns a source's concrete type.
   [Its driver](../../../src/semantic/inference/infer.c) links explicit types
   before one ordered event pass. T182 must separate lexical binding identity
   from globally refinable type terms. Keep existing foreach/source-order laws.
6. [Dependency extraction](../../../src/project/dependencies/discover/ast.c)
   walks explicit declaration paths, qualified constants and namespaces in
   source order. Keep original requesting spans and exact configured roots;
   T171 must additionally traverse structural type expressions without treating
   atomic placeholder components as filesystem dependencies.
7. [Native admission](../../../src/semantic/native/contracts.c) checks companions
   and ABI after ordinary inventories; it has no own-free-static-formal inventory.
   T174 adds that early check after T172. Existing companion/lifecycle checking
   is reusable; native modules are concrete, not a generic specialization ABI.
8. [Relation vertices](../../../src/semantic/relations/eager.c) use concrete type
   or member identity and source module names. Keep eager/constant relations
   separate from new equality, static-definition and creation-path graphs.
   [Constant planning](../../../src/semantic/constants/plan.c) already retains
   forward/reverse ordered links; T186 supplies specialization-aware vertices.
9. [Planning](../../../src/planning/plan.h) borrows resolved concrete semantics.
   [Generated constant keys](../../../src/codegen/model/identity.c) currently
   include the source module path/member. T183/T187/T188 must carry full
   specialization identity through these consumers, with no runtime template
   records or shared mutable instance state.
10. [Export lookup](../../../src/library/resolve/exports.c) selects an ordinary
    available executable type. Retain that public boundary; T194 exports concrete
    facades rather than adding specialization selectors.
    [Editor analysis](../../../src/editor/analysis/build.c) shares the frontend
    and [relocation reference analysis](../../../src/tools/relocation/references/build.c)
    consumes checked project graphs. T192/T193 adapt those consumers to static
    binders and actual module-path leaves, not source-text replacement.

No inspected implementation component is obsolete solely because it lacks
templates. Retain its ordinary behavior, splitting concrete-only assumptions
where their actual new consumer requires it. Obsolete current prose about
already implemented pre-template features must be reconciled here; immutable
historical task/acceptance records remain historical.

## Production and acceptance ownership

Existing paths below are retained responsibility homes. Paths marked new are
planned cohesive children, not directories created merely to satisfy a tree.
Each implementation task defines its actual narrow C interface and argument
before code; it must register every changed file and independent/consumer case.

| Task | Production owner and handoff | Independent and real consumer evidence home |
|---|---|---|
| T169 | source/unicode bounded words; lexer/tokens, source/module/path and literal boundary consumers | tests/unit/source/unicode, lexer/tokens, source/module; parser/access and integration/parser/grammar |
| T170 | ast plus parser static type-expression/interface children (new); immutable source spans out | tests/unit/parser static-expression child (new); parser/tree/declarations/calls and compiler/check |
| T171 | project/dependencies/discover and source/module; exact parsed definitions/origins out | tests/unit/project/closure; integration/project/closure |
| T172 | semantic static inventory/scope/signature children (new); bound static identities and unresolved obligations out | tests/unit/semantic/static/scope and signatures (new); parser-to-inventory frontend composition |
| T173 | semantic static interface/binding normalization (new); four disjoint ordinal sequences out | tests/unit/semantic/static/bindings (new); actual parsed interface applications |
| T174 | semantic/native admission and compiler/authoring; completed own-formal inventory in | tests/unit/semantic/native; native/scaffold and compiler authoring consumers |
| T175 | semantic/static/terms (new); immutable scoped normalized records out, no expansion | tests/unit/semantic/static/terms (new); bound inventory handoff |
| T176 | semantic/static/equality (new); finite equations in, solved/conflict/unresolved out | tests/unit/semantic/static/equality (new); normalized term composition |
| T177 | semantic/static/application and patterns (new); exact normalized records/recovery out | tests/unit/semantic/static/application (new); real syntax/signature/equality consumers |
| T178 | semantic/static/dependencies (new); guarded symbolic ancestry/provenance out | tests/unit/semantic/static/dependencies (new); nested applications before expansion |
| T179 | semantic/static/results/projection (new); guarded definition equations out | tests/unit/semantic/static/results (new); parsed nested/static namespace applications |
| T180 | semantic/static/results/closure (new); closed producer plus immutable alias result out | tests/unit/semantic/static/closure (new); scoped alias and returned partial constructor consumers |
| T181 | semantic/constants plus static namespace resolution (new); declared type obligations out | tests/unit/semantic/constants and static namespaces (new); actual constant-source connections |
| T182 | semantic/inference lexical events plus static constraint collection (new); bindings fixed before solving | tests/unit/semantic/inference and static inference (new); semantic/composition |
| T183 | semantic/static/specializations (new), source/identity; closed candidate keys/dependency cache out | tests/unit/semantic/static/specializations (new); alternate incoming-ancestry consumer |
| T184 | compiler/frontend and semantic static orchestration (new); complete closed candidates out | tests/unit/compiler/check and composition; integration/check |
| T185 | semantic/values, copy, control, relations; validated concrete candidates out | tests/unit/semantic/types/relations/native; compiler/check and semantic/composition |
| T186 | semantic/constants, planning/storage and runtime/program constants; ordered startup out | tests/unit/semantic/constants and runtime/program; codegen/assembly later in T188 |
| T187 | planning storage/lifecycle/control; concrete resolved plans out | tests/unit/planning/ir/validation; codegen model composition |
| T188 | codegen model/project/emit; assembly and runtime descriptors out | tests/unit/codegen/assembly/execution/continuations; integration/execution |
| T189 | diagnostics, semantic/report, source/mapping and codegen source tables | tests/unit/diagnostics; integration/source/diagnostics and compiled trace consumers |
| T190 | native/scaffold, native/toolchain and concrete semantic views | tests/unit/native/scaffold/declarations; integration/scaffold/native and generated execution |
| T191 | project inputs/dependencies, source/module and external native closure | tests/unit/project/inputs/closure; integration/project/closure/rebuild |
| T192 | editor analysis/index/query/rename; common frontend identities in | tests/unit/editor; integration/tools |
| T193 | tools/relocation references/domain/edits and existing transaction/recovery | tests/unit/tools/relocation; integration/tools and rebuilt moved programs |
| T194 | library resolution/header/transport/package/execution | tests/unit/library; integration/library-build/library-exports/c-library-examples |
| T195 | executable template examples over existing list methods | integration/examples template child (new); actual compiled type variants and invalid applications |
| T196 | tools/conformance and reviewed evidence registry | tests/unit/conformance; all current canonical acceptance owners |
| T197 | build/test/release/toolchain orchestration | focused resource/toolchain consumers and clean make/test release |
| T198 | maintained comments/contracts and tools/documentation | docs-complete, docs, docs-reproducible; final conformance freshness |

The [37-area routing](../../../TODO.md#acceptance-routing) supplies the exact
canonical area-to-task map. It is not executable evidence yet. Three hyphen
areas, storage/dispatch, editor/relocation and concrete C facade/export-exclusion
areas have explicit additional owners there. The deferred list foundation is
not an unowned gap: T195 exercises ordinary list algorithms; no provider bridge
or raw storage contract is selected.

## Verification

All commands run from the repository root on 88f5867 plus T168 narrative changes;
no production/test/build input has changed.

| Check | Command / input | Actual result and limit |
|---|---|---|
| Compiler | make; default Makefile GCC/C11/O2 flags | Exit 0; binary executed, never inspected |
| Pinned names, token/ordinary structure | make -C tests/unit/source/unicode; lexer/tokens; parser/declarations; parser/calls; parser/results; parser/control, each with its full tests/unit prefix | All six exit 0; retained ordinary behavior, not template acceptance |
| Semantic/frontend closure | make -C tests/unit/semantic/inference; semantic/composition; project/closure; compiler/check, each with its full tests/unit prefix | All four exit 0; current concrete model and allocation/ownership paths |
| Emission and exports | make -C tests/unit/codegen/assembly; make -C tests/unit/library/resolution | Both exit 0; retained compiled assembly and ordinary exports |
| Real CLI/parser composition | make -C tests/integration/parser/grammar; make -C tests/integration/check | Both exit 0; actual CLI and ordinary grammar handoffs |
| Reviewed conformance | make conformance-check | Exit 2: specification changed; conformance review required. Expected stale pre-template evidence, not a reason to refresh hashes |

### Minimal independent source probes

Temporary source/result storage: /tmp/ama-t168-probes-r85u2uzf, with captured
inputs/stdout/stderr in results.json. These durable definitions reproduce the
probes without requiring that temporary directory. For each case place an
extensionless main file and the listed value/identity files in a fresh directory
P; execute ./ama --check P .main. The control value file contains only an empty
Ama implementation section:

```text
@!
```

The identity template file contains:

```text
@<
input
...element
@>
output
...element
@!
output
 input
```

| Case | Main source or change | Expected from current rules | Observed |
|---|---|---|---|
| Ordinary control | Empty body section followed by two lines: value and .value | Accept | Exit 0 |
| Multiword control | Same, with receiver name value name | Accept | Exit 0 |
| Hyphen | Same, with receiver name value-name | Accept under SRC-03 | Exit 1, invalid code character at the hyphen; T169 owns the gap |
| Static result | Result section containing ...provided then .value, followed by an empty body section | Accept under TPL-12; no free owner formal | Exit 1, compact access opener requires a name; T170 owns initial recognition |
| Concrete template call | Main below, with value and identity as above | Accept: infer element from a concrete ordinary input | Exit 1 at identity's placeholder type; recognition blocks testing later template semantics |

Concrete template-call main:

```text
@!
sample
.value
worker
.identity
worker !
 input < sample
 output > result
```

The initial template probe accidentally paired a named input with a positional
output. Its rejection was correct and supplied no template-gap evidence.
Correcting the fixture to the named output shown above reached the placeholder
failure; the original and corrected observations are both retained in results.json.

These probes establish particular visible gaps, not a proof that every later
template feature is independently absent. The actual AST/model/stage interfaces
explain the prerequisite changes. No full-suite conformance, sanitizer coverage
or runtime template support is claimed.

## Acceptance

Source/API findings, all thirty downstream ownership boundaries and the existing
37-area canonical routing agree. The documentation-only audit changes no language,
compiler/native/test code, reviewed fingerprints or immutable historical audits.
The current AST guide now distinguishes implemented ordinary constructs from
missing static expression forms. No independently observed behavior is inferred
from a historical checkbox.

python3 /tmp/ama-t168-doc-check.py checks all tracked/unignored Markdown links,
the 214-rule index bijection, 108 byte-unchanged aligned EBNF productions and
37 template areas. It passed over 205 Markdown documents. git diff --check passed.
make docs-complete passed: 2335 ready maintained inputs, zero explicit debts.
make docs passed: Doxygen 1.11.0, Python 3.14.4, markdown-it-py 3.0.0; 1749
maintained file pages, 6703 interfaces, 417 destinations and 5528 generated pages.
The final generation rechecks coverage and the final archive/queue inputs.
The full source/probe definitions and the failed initial fixture are preserved
above; temporary logs are supplementary. This audit satisfies T168's bounded
acceptance and does not claim full implementation conformance.
