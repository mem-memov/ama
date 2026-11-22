# Building applications with CMake

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Keep dependency instructions with their native module](#keep-dependency-instructions-with-their-native-module)
- [Generate an instructional native companion](#generate-an-instructional-native-companion)
- [Build or run the selected module with one command](#build-or-run-the-selected-module-with-one-command)
- [Make the target and build inputs explicit](#make-the-target-and-build-inputs-explicit)
- [Explain which stage failed and where to find help](#explain-which-stage-failed-and-where-to-find-help)

<!-- /chapter-contents -->

The Ama compiler knows which modules form a program. CMake knows how to arrange
C compilation and link dependencies declared by their authors. These jobs meet
at a generated build description. They do not require the Ama compiler to
implement a package manager for system libraries or to infer link flags from
C source text.

A module's optional CMake companion declares its native requirements. A separate
project-tool command emits the Ama component, asks CMake to build it, verifies
the properties required by Ama and optionally runs the result. A programmer can
also request generated C and integrate it into an existing C project. The
[launch and source-inclusion contract](launch-and-embedding.md) specifies the
entry wrapper, arguments and generated interface for those two output modes.

**Explanation: three different kinds of information.** The native declarations
and `.abi` file say how Ama and a native implementation interact. A `.cmake`
companion says how to connect that implementation to installed C dependencies.
A package README explains how to prepare the machine. The README is useful to a
person or an agent, but it is not a compiler input or an executable installation
script. Neither the presence of a README nor successful C linking establishes
that the native behavior is correct; module tests still supply that evidence.

## Keep dependency instructions with their native module

<a id="build-01"></a>

**BUILD-01.** A concrete native `@C` module may have an authored companion named
`MODULE.cmake` beside its Ama source and other native companions. The name uses
the exact physical module basename, including its spelling and case. The file
is optional: absence contributes no module-specific CMake instructions and is
not itself an error. It belongs to the module's delivery unit under
[PKG-07](../project/packages.md#pkg-07). Its contents are CMake code, not Ama
syntax, INI configuration or a new `.abi` protocol.

The compiler emits a machine-readable ordered list of participating native
modules and their effective companions from its actual resolved component
closure. This list includes native dependencies of selected concrete static
specializations and excludes unused modules, ignored package copies and
unselected tests. Deduplicate a module by its full resolved identity within one
component. Select its companion from the same effective package copy as its
source; never fill an absent local companion from an overridden dependency.

The CMake adapter applies each participating companion exactly once to each
independently compiled Ama component target. Two different test components each
receive the instructions they require, even if they share a native dependency.
Multiple runtime instances do not cause repeated application. The adapter uses
an explicit `include()` of the selected file; it does not concatenate companion
texts, scan directories for other build scripts or interpret CMake itself.
Within the include, `AMA_COMPONENT_TARGET` names an already created concrete
CMake target for this component. The adapter invokes each include inside a fresh
CMake function scope so temporary ordinary variables do not become another
companion's inputs. Relative authored resources must be based on
`CMAKE_CURRENT_LIST_DIR`, or on an explicitly supplied resource path, rather
than the invoker's current directory.

A companion must attach requirements to `AMA_COMPONENT_TARGET` through
appropriate target operations. Prefer imported dependency targets that carry
headers, definitions and link requirements together. It must not change global
compiler/linker flags, the toolchain, the selected generator, unrelated targets
or another companion's private variables. It must not compile the module's
native `.c` separately: that body already belongs to the one Ama assembly under
[GEN-01](compiler.md#gen-01). External libraries may have their own normal build
or imported targets; those targets do not split internal Ama execution into C
calls. Required Ama ABI, language-mode and native-inlining settings cannot be
removed or overridden by a companion.
Module-owned helper scripts/resources must travel in the same declared delivery
unit, such as same-basename companions or the `MODULE+/` subtree, or be supplied
as explicitly identified external inputs under PKG-07. Selective delivery does
not execute CMake to discover extra package-root files. Missing undeclared
resources are not repaired by downloading neighboring files heuristically.

A global `include_guard()` or a process-wide "already applied" flag must not
skip application to another component. Reusable imported CMake dependency
targets may be shared where their declared version, platform and options agree.
A companion must validate an already present shared target against its declared
requirements, or use an independently qualified dependency target. An existing
target's mere name is not evidence of compatibility. Declared incompatible
requirements produce an error; traversal/include order must not choose a
winner. CMake's normal diagnostics and author-supplied requirements do not
constitute universal detection of undeclared external-library ABI conflicts.

CMake execution occurs only in an explicitly requested configuration/build
operation. Compilation in generation-only mode, source analysis, acquisition,
relocation and scaffolding do not run companions. A companion may locate
prepared installed libraries through ordinary CMake mechanisms. The supported
contract does not implicitly download sources, run an OS package manager,
install software or execute an installation command from a README. Preparation
of unavailable dependencies remains a separately requested operation.

**Example: an optional Zlib dependency.** Assume a native module with the
physical files below, all in one effective package version:

```text
src/
├── compress
├── compress.c
├── compress.h
├── compress.abi
└── compress.cmake
```

The authored `compress.cmake` can contain:

```cmake
find_package(ZLIB REQUIRED)
target_link_libraries("${AMA_COMPONENT_TARGET}" PRIVATE ZLIB::ZLIB)
```

The generated orchestration has this shape; exact private target/function names
are not public ABI:

```cmake
function(ama_apply_native component instruction)
    set(AMA_COMPONENT_TARGET "${component}")
    include("${instruction}")
endfunction()

add_executable(program "${CMAKE_CURRENT_LIST_DIR}/program.c")
ama_apply_native(program "/prepared/package/src/compress.cmake")
```

The Zlib target supplies its usage requirements. This is an informative use of
[CMake's FindZLIB module](https://cmake.org/cmake/help/latest/module/FindZLIB.html),
[include command](https://cmake.org/cmake/help/latest/command/include.html) and
[function scope](https://cmake.org/cmake/help/latest/command/function.html).
The supported adapter documents the actual CMake versions it accepts; these
links do not require adopting the newest available CMake version implicitly.

## Generate an instructional native companion

<a id="build-02"></a>

**BUILD-02.** An explicit native-scaffolding request for an `@C` module under
[CABI-06](native-interface.md#cabi-06) also creates `MODULE.cmake` when that file
does not already exist. Its initial contents are comments and blank lines only,
with no active CMake command. The scaffold is immediately usable for a native
module that needs no additional C libraries. It is an authored companion, not
an output that normal compilation may regenerate.

The comments explain the file's purpose and all BUILD-01 obligations. Include
a fully commented `find_package`/`target_link_libraries` example, the meaning of
`AMA_COMPONENT_TARGET`, use of `CMAKE_CURRENT_LIST_DIR`, target-only settings,
the one-assembly rule, function-local temporary variables, application once per
component, the prohibition on a global once-only guard, compatible sharing of
imported targets, and the separation of dependency installation from building.
State that compiler/toolchain/ABI settings remain controlled by the selected
build profile and that an optional version README carries preparation guidance.
Link the canonical BUILD-01 rule in the generated comments through the supported
documentation location; do not embed the generator developer's checkout path.

An existing companion is preserved byte for byte, including an empty file.
A repeated scaffold request reports it as existing and never replaces it with
new instructions. Validate output ownership and collisions before publishing
new scaffold artifacts. A preparation or publication failure must not truncate
an existing authored file. Normal header/configuration refresh under CABI-06
does not confer permission to rewrite this companion. No CMake code executes
during scaffolding.

**Example: part of a newly generated companion.** The real template also
contains the other explanations required above:

```cmake
# Optional native dependency instructions for this module.
# AMA_COMPONENT_TARGET is the existing component target being configured.
# Example only: uncomment and adapt when this module requires Zlib.
# find_package(ZLIB REQUIRED)
# target_link_libraries("${AMA_COMPONENT_TARGET}" PRIVATE ZLIB::ZLIB)
# Native entry code already belongs to the Ama assembly: do not add its .c again.
# Apply settings to the supplied target; do not change global compiler flags.
# Do not use a global include guard: another test component needs these settings.
# Resolve authored resources relative to CMAKE_CURRENT_LIST_DIR.
# Dependency installation belongs to explicit environment preparation.
```

## Build or run the selected module with one command

<a id="build-03"></a>

**BUILD-03.** Provide these executable-mode operations:

```sh
ama-project build MODULE --output DIRECTORY
ama-project run MODULE --output DIRECTORY -- PROGRAM_ARGUMENTS
```

`--output DIRECTORY` is required and selects the operation-owned output root.
Options belong before the standalone `--`; reject unknown options and duplicate
conflicting settings. `run` requires the separator even when the runtime argument
sequence is empty. These operations select one fully resolved concrete executable
entry. The target build profile is an explicit invocation/configuration input.
`MODULE` uses the ordinary module resolution contract, not a fixed project-wide
main filename. Open template/static roots and unresolved interface
types are rejected according to the launch contract. The runtime
argument sequence of `run` follows the standalone `--`; tokens after it are passed
to the completed program without interpretation as project-tool or CMake options.
`build` performs no program execution. The entry wrapper's argument syntax and
semantics are defined only in the
[launch chapter](launch-and-embedding.md).

The high-level operation executes these stages in order:

1. Resolve the selected entry and prepared local source/configuration/native
   closure; validate the request and claim operation-owned output paths.
2. Ask the compiler to validate the program and emit the selected component's
   C assembly, launch/public declarations, source maps, dependency instructions
   and required build/verification metadata. Stop on any compiler failure.
3. Generate an out-of-source CMake project with the selected target/profile,
   required Ama options and exactly the participating companion includes.
4. Run CMake configuration, then build the selected component target using the
   requested build configuration. Stop on configuration, compilation or link
   failure. No later stage may use an older successful executable instead.
5. Verify the actual built artifact and recorded configuration against the
   supported ABI and mandatory native-entry/control-flow checks under
   [CABI-06](native-interface.md#cabi-06) and
   [STACK-01 through STACK-04](compiler.md#mandatory-inlining-and-verification).
   Successful C compilation or the mere presence of an `inline` spelling is not
   this verification. Reject unsupported toolchain/profile combinations.
6. Publish the artifact and its matching metadata only after all required checks
   pass. For `run`, launch that published artifact with the runtime argument
   sequence. Report the child's actual completion, execution failure or signal;
   starting a child process is not evidence of its success.

The adapter uses the ordinary separate CMake configure and build operations,
schematically `cmake -S GENERATED -B BUILD` and
`cmake --build BUILD --target TARGET`; target names and paths are concrete
outputs of the plan, not shell-interpolated untrusted strings. The supported
adapter records any generator-specific configuration selection, including the
configuration selected for a multi-configuration generator. See the
[CMake command reference](https://cmake.org/cmake/help/latest/manual/cmake.1.html).

The compiler also supports generation without CMake execution or a C compiler
invocation. The explicit output-mode configuration selects `executable` or
`source-inclusion`; `build`/`run` always select `executable`. Generate the requested
main-wrapped or source-inclusion component and its required integration metadata. Generation success establishes valid
emission, not a verified runnable artifact. The source-inclusion output provides
no `main`; its entry/public naming, include ownership and lifecycle follow the
launch chapter. Its generated CMake integration applies the same dependency
contract to the host target that compiles the included implementation, without
adding that implementation as another source or creating a second executable.
A manual non-CMake integration remains possible, but must reproduce the recorded
requirements and invoke the mandatory verifier before claiming conformance.

Preparation and compilation stay separate under
[PROJECT-01](project-tools.md#project-01). `build`/`run` may invoke explicit CMake
and C toolchain processes; they must not fetch packages, mutate source trees,
install missing libraries, generate authored companions or run unrelated tests.
Test orchestration uses the same component/build contract for each independently
selected test under [PROJECT-03](project-tools.md#project-03).

**Explanation: repeated execution.** Program arguments are runtime inputs.
Changing `--par=value` does not change the generated program or require CMake
reconfiguration. A cached verified build may be reused only after BUILD-04
establishes that its complete declared build identity remains current. Running
a previously built artifact directly remains possible without invoking CMake.

## Make the target and build inputs explicit

<a id="build-04"></a>

**BUILD-04.** The selected profile identifies the target platform/architecture,
C compiler and toolchain, CMake generator/configuration, supported C language and
ABI contract editions, required Ama verification settings, and explicitly chosen
native dependency search/configuration inputs. A documented default profile may
select the native host platform, but its resolved values are recorded; silently
substituting host libraries during cross-compilation is prohibited. Cross-build
`run` requires an explicitly selected supported execution environment; without
one, refuse execution while allowing a successful build to be delivered.

Collect native requirements from the actual resolved component closure, never
from every package installed on the machine. Apply options in a deterministic
documented order derived from full resolved module identities. That order
reproduces the build description; it does not resolve incompatible requirements.
Preserve the semantics and required order of library/link options as specified
by CMake dependency targets and the selected linker, including repetitions
needed by that contract. Do not alphabetically sort or set-deduplicate link
inputs after their meaning has been established.

All native companions in one Ama assembly share its C translation environment.
Function-local CMake variables do not isolate macros, packing, type layouts or
C compiler options within that assembly. Diagnose declared incompatible
requirements and detected target/ABI/type mismatches. Modules requiring
incompatible translation environments need an explicit external compiled
component boundary, not a per-module compilation exception. The native authors
remain responsible for complete accurate requirements; no guarantee of detecting
all undocumented third-party incompatibilities is introduced.

Retain a machine-readable build record: selected entry/output mode and component
identity; resolved Ama/package/configuration/alias/native input identities and
content hashes; participating companion paths and contents; target/profile and
tool versions; effective compile/link settings and dependency identities to the
extent exposed by their declared contracts; commands, verification and results.
Record authored resource/build inputs declared by CMake and the selected dependency
providers. An external provider with no stable artifact identity is reported as
such; do not pretend that its target name or version string is a content hash.
Unsupported or unrecordable external effects cannot justify cache reuse.

Changes to an entry, resolved closure, alias/package selection, companion,
declared external dependency, supported target/toolchain or ABI-affecting option
invalidate the affected artifacts and verification. Reuse only with matching
input records and the build system's actual dependency tracking. An output file's
existence or timestamp alone is insufficient. Untracked user environment/input
changes require explicit invalidation rather than inferred equivalence.

Generate outputs only in explicitly selected operation-owned locations outside
the Ama source namespace. Follow [COMP-04](compiler.md#comp-04): never overwrite
an authored companion or unrelated host project/build tree. Parallel jobs own
independent generated/build paths, or use a checked ownership protocol that
prevents conflicting writers. Stage a new candidate, retain failed diagnostics
separately, and publish a coherent successful artifact/metadata set only after
verification. On failure report that the requested build failed; any previously
published artifact remains identified as a previous result and is neither run
nor presented as the new result. Cleanup removes only operation-owned temporary
outputs and reports retained paths on cleanup failure. These obligations do not
promise rollback of arbitrary effects inside trusted authored CMake code.

## Explain which stage failed and where to find help

<a id="build-05"></a>

**BUILD-05.** A failed build/run reports the selected application or test identity,
the failed stage, subprocess command/tool and status when applicable, and the
original diagnostic output. Distinguish Ama validation/emission, CMake
configuration, C compilation, linking, mandatory verification, process startup
and already running program failure. Preserve the primary failure when reporting
cleanup or report-writing errors. A successfully linked binary is not a passed
test, and a missing header is not an algorithmic test failure.

Attach the originating module and physical source/companion span when the
provenance establishes them, using compiler source maps and the generated
companion-to-module relation. A failed include can identify the included
companion, while a C source diagnostic may identify its owning native module.
When a linker or arbitrary CMake diagnostic cannot establish a unique module,
report the component/test and relevant participating instructions instead of
guessing a culprit from an unresolved symbol. Do not infer an OS package name
or installation command from diagnostic wording.

For each identified relevant package version, report the effective version
root and its optional `README.md`. If present, supply its usable physical path;
if absent, state that the file is absent and still report the version directory.
Use the same effective package selection as compilation. A package README is
optional, is not parsed by the compiler or CMake adapter and cannot be required
for compilation. If the exact README path exists but is not a readable regular
file, generation and building continue: retain its path and the observed
I/O/shape issue as unavailable-guidance metadata, rather than treating it as
absent or selecting another copy. Failure to inspect optional guidance does not
replace a primary build failure. This does not relax errors for required source,
native companion or declared resource inputs. Actual source delivery still
checks the README under [PKG-07](../project/packages.md#pkg-07) and refuses a
supplied unreadable/non-file entry.

Native dependency requirements and their preparation checks belong in the
module's CMake companion under [BUILD-01](#build-01). README text is optional
human guidance, never a substitute for those declarations. The system must not
fabricate installation guidance for a README that is missing or unavailable.
BUILD-01's separation of explicit dependency preparation from configuration/build
continues to apply; this rule does not authorize automatic software installation.

Package authors should describe required external libraries, supported platforms,
installation of runtime and development/header packages, configuration options,
a preparation check and the command to repeat the affected test. A diagnostic
that a header or library was not found can mean either missing software or an
incorrect search/configuration path. A person or agent uses the instructions
to inspect the selected target environment before deciding which correction is
appropriate; successful testing establishes the resulting scoped evidence.

**Example: a missing native header while building one test.**

```text
Failed test build: /acme/db/v1.0.0/tests/open/read
Stage: C compilation
Native module: /acme/db/v1.0.0/src/open
Source: dependencies/acme/db/v1.0.0/src/open.c:4
C compiler: fatal error: sqlite3.h: No such file or directory
Preparation instructions: dependencies/acme/db/v1.0.0/README.md
```

This is informative diagnostic text. The facts are required; exact wording and
machine-readable diagnostic encoding remain implementation interfaces.

Acceptance includes actual CMake configure/build/run and generation-only probes;
no-dependency and shared native dependencies; two independent test targets using
one companion; required include/source order; optional/missing companions and
README files, including nonfatal unreadable/non-file guidance during generation
and building versus delivery refusal; untouched authored scaffolds; declared dependency conflicts;
unsupported toolchains and target/host distinction; missing headers/libraries;
CMake, compiler, linker, verifier and startup failures; source-inclusion host
integration; stale cache rejection; parallel output ownership; publication and
cleanup failure; and provenance that matches the artifact actually executed.
The source-inclusion/launch chapter adds its entry and argument acceptance cases.
