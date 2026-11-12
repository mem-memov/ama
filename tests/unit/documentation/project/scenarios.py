## @file
# @brief Supply isolated inputs and independent observations to the C/assert leaf.
#
# These fixtures call real parsing, coverage, Doxygen and rendered-link boundaries.
# Errors escape to the C parent as child failure; no alternate test runner is used.
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(sys.argv[1])
sys.path.insert(0, str(ROOT / 'tools/documentation'))
from linking.markdown import compact_heading_levels, links
from linking.registry import Registry
from linking.source import comments
from linking.view import transform
from rendered import Page, interfaces, validate
from build import generate
import coverage


def require(condition, description):
    """Assert one exact scenario observation by raising ValueError; this fixture helper does not reinterpret a failing subprocess as success."""
    if not condition:
        raise ValueError(description)


def rejected(operation):
    """Require the supplied invalid operation to raise its declared validation/compiler failure; normal return is a regression."""
    try:
        operation()
    except (ValueError, subprocess.CalledProcessError):
        return
    raise ValueError('invalid input was accepted')


def prepare(name):
    """Create one isolated generated scenario root with minimal canonical requirements; remove only that prior test-owned fixture if present."""
    root = Path('generated') / name
    if root.exists():
        shutil.rmtree(root)
    (root / 'spec').mkdir(parents=True)
    (root / 'spec/requirements.md').write_text('# Rules\n\n[RT-13](runtime.md#rt-13)\n')
    (root / 'spec/runtime.md').write_text('# Runtime\n\n<a id="rt-13"></a>\n\n**RT-13.** Preserve identity.\n')
    return root.resolve()


def source():
    """Check physical Markdown/C source offsets, prose-only canonical rewrites and untouched code examples/literal bytes."""
    headings = '# Root\n\n#### Child\n\n```md\n###### Literal\n```\n\n#### Sibling\n\n##### Grandchild\n'
    compacted = headings
    for begin, end, replacement in reversed(compact_heading_levels(headings)):
        compacted = compacted[:begin] + replacement + compacted[end:]
    require(compacted == '# Root\n\n## Child\n\n```md\n###### Literal\n```\n\n## Sibling\n\n### Grandchild\n',
            'heading compaction changed the parent tree or a fenced example')
    require(compacted.count('\n') == headings.count('\n'), 'heading compaction changed source line count')
    quoted = '# Root\n\n> #### Quoted\n\n#### Child\n'
    require(compact_heading_levels(quoted) == [(quoted.index('#### Child'), quoted.index('#### Child') + 4, '##')],
            'heading compaction interpreted a nested container as top-level')
    separated = '# Root\u2028suffix\n\n#### Child\n'
    require(compact_heading_levels(separated) == [(separated.index('#### Child'), separated.index('#### Child') + 4, '##')],
            'Unicode prose separator changed CommonMark LF offsets')
    setext = 'Root\n====\n\n#### Child\n'
    require(compact_heading_levels(setext) == [(setext.index('#### Child'), setext.index('#### Child') + 4, '##')],
            'setext parent rank was not preserved')
    registry = Registry(ROOT)
    path = 'src/runtime/pool/deep/fixture.c'
    href = '../../../../spec/implementation/runtime.md#rt-13'
    prose = '[RT-13](' + href + ')'
    text = '/** @file\n * @brief Preserve identity.\n * ' + prose + '\n *\n * ```c\n * ' + prose + '\n * ```\n */\n'
    literal = 'const char *s = "' + prose + '";\n'
    rewritten, refs = transform(path, text + literal, registry)
    require(rewritten.count('@ref ama_rule_') == 1 and len(refs) == 1, 'rewrite must affect prose only')
    require(literal in rewritten and ' * ' + prose + '\n * ```' in rewritten, 'literal/fence bytes changed')
    require(rewritten.count('\n') == (text + literal).count('\n'), 'line count changed')
    example = '/**\n * @code\n * ' + prose + '\n * @endcode\n */\n'
    require(transform(path, example, registry)[0] == example, 'Doxygen example rewritten')
    line_example = '/// ```c\n/// ' + prose + '\n/// ```\n'
    require(transform(path, line_example, registry)[0] == line_example, 'line-comment example rewritten')
    multiline = '> * paragraph `code\n>   [RT-13](ignored)` end [RT-13](actual)\n'
    observed = links(multiline)
    require(len(observed) == 1 and observed[0][3] == 'actual', 'nested multiline code was parsed as prose')
    table = '| a | b |\n|---|---|\n| `x` | [RT-13](actual) |\n'
    require(links(table)[0][3] == 'actual', 'table source map lost destination')
    reference = '[RT-13][rule]\n\n[rule]: actual\n'
    require(links(reference)[0][3] == 'actual', 'reference link not resolved by CommonMark')
    require(links('#') == [], 'empty heading failed')
    require(not links('![alt [RT-13](inside)](image)\n'), 'image text rewritten')
    trigraph = 'const char *s = "escaped??/\n\\\"/** fake */";\n/** real */'
    require([part.strip() for part, _ in comments(trigraph)] == ['real'], 'C translation phases corrupted comment recognition')
    spliced = '/\\\n** real */'
    require(list(comments(spliced))[0][0].strip() == 'real', 'spliced comment opener lost')


def registry():
    """Reject exact stale, duplicate, out-of-tree, mismatched and collision-forced canonical rule identities."""
    root = prepare('registry')
    rules = Registry(root)
    rejected(lambda: rules.reference('deep/file.c', 'RT-13', '../spec/runtime.md#missing', True))
    rejected(lambda: rules.reference('deep/file.c', 'RT-99', '../spec/runtime.md#rt-13', True))
    rejected(lambda: rules.reference('deep/file.c', 'RT-13', '../../escape.md#rt-13', True))
    rejected(lambda: rules.reference('deep/file.c', 'RT-13', 'https://example.invalid/#rt-13', True))
    chapter = root / 'spec/runtime.md'
    original = chapter.read_text()
    chapter.write_text(original + '<a id="rt-13"></a>\n')
    rejected(lambda: Registry(root))
    chapter.write_text(original.replace('**RT-13.**', '**RT-99.**'))
    rejected(lambda: Registry(root))
    chapter.write_text(original)
    collision = Registry(root, digest=lambda _: 'collision')
    rejected(lambda: collision.identity('rule', 'other.md', 'OTHER-01'))
    index = root / 'spec/requirements.md'
    index.write_text(index.read_text() + '\n[RT-13](runtime.md#rt-13)\n')
    rejected(lambda: Registry(root))


def rendered():
    """Run real Doxygen on nested and moved sources, verify originating file/page links and reject tampered rendered canonical destinations."""
    root = prepare('rendered')
    (root / 'docs').mkdir()
    shutil.copyfile(ROOT / 'docs/Doxyfile', root / 'docs/Doxyfile')
    (root / 'docs/documentation.md').write_text('# Fixture\n')
    (root / 'docs/implementation').mkdir()
    (root / 'docs/implementation/library-tests-and-build.md').write_text('# Fixture lifetime contract\n\n#### Preserved child\n\n[Child](#preserved-child)\n')
    nested = root / 'src/a/b/c'
    nested.mkdir(parents=True)
    code = '/** @file\n * @brief Keep identity under [RT-13](../../../../spec/runtime.md#rt-13).\n * @see [Fixture contract](../../../../docs/implementation/library-tests-and-build.md)\n */\n/** Provide one constant fixture value. */\nint fixture(void) { return 0; }\n'
    original = nested / 'fixture.c'
    original.write_text(code)
    included = nested / 'ranges.inc'
    included.write_text(code.replace('int fixture(void) { return 0; }', 'static const int fixture_ranges[] = {0};'))
    hashes = {path: hashlib.sha256(path.read_bytes()).hexdigest() for path in root.rglob('*') if path.is_file()}
    output = root / 'html'
    generate(root, output, shutil.which('doxygen'))
    require(all(hashlib.sha256(path.read_bytes()).hexdigest() == value for path, value in hashes.items()), 'maintained bytes changed')
    registry = Registry(root)
    anchor = registry.identity('rule', 'spec/runtime.md', 'RT-13')
    file_page = next(path for path in output.glob('*.html') if path.name.endswith('_8c.html'))
    require(any('#' + anchor in href for href in Page(file_page.read_text()).hrefs), 'file purpose has no rendered rule link')
    validate(output, {'src/a/b/c/ranges.inc': [anchor]})
    contract_page = registry.identity('page', 'docs/implementation/library-tests-and-build.md')
    validate(output, {'src/a/b/c/fixture.c': [contract_page]})
    # A rendered file page alone cannot certify its extracted interfaces.
    import xml.etree.ElementTree as ET
    interface_file = next(path for path in (output / 'xml').glob('*.xml')
                          if ET.parse(path).getroot().find(".//memberdef[@kind='function']") is not None)
    original_xml = interface_file.read_bytes()
    xml_index = output / 'xml/index.xml'
    original_index = xml_index.read_bytes()
    xml_index.write_text('<not-closed>')
    rejected(lambda: interfaces(output))
    xml_index.write_bytes(original_index)
    interface_file.unlink()
    rejected(lambda: interfaces(output))
    interface_file.write_text('<doxygen/>')
    rejected(lambda: interfaces(output))
    interface_file.write_bytes(original_xml)
    tree = ET.parse(interface_file)
    declaration = tree.getroot().find(".//memberdef[@kind='function']")
    declaration.find('briefdescription').clear()
    declaration.find('detaileddescription').clear()
    tree.write(interface_file)
    rejected(lambda: interfaces(output))
    interface_file.write_bytes(original_xml)
    require(interfaces(output) > 0, 'documented interface fixture was not extracted')
    original.write_text(code.replace(' * @brief Keep', ' * @C\n * @brief Keep', 1))
    rejected(lambda: generate(root, root / 'rejected-html', shutil.which('doxygen')))
    original.write_text(code)

    content = file_page.read_text()
    file_page.write_text(content.replace('#' + anchor, '#ama_rule_missing'))
    rejected(lambda: validate(output, [anchor]))
    file_page.write_text(content.replace('#' + anchor, '#ordinary_missing_anchor'))
    rejected(lambda: validate(output, {'src/a/b/c/fixture.c': [anchor]}))
    file_page.write_text(content)
    rejected(lambda: validate(output, [anchor], {anchor: registry.identity('page', 'spec/requirements.md')}))
    moved = root / 'src/moved.c'
    moved.write_text(code)
    rejected(lambda: transform('src/moved.c', code, registry))
    moved.write_text(code.replace('../../../../spec/', '../spec/').replace('../../../../docs/', '../docs/'))
    original.unlink()
    shutil.rmtree(output)
    generate(root, output, shutil.which('doxygen'))
    validate(output, [anchor])
    rejected(lambda: Page('<a id="ama_rule_x"></a><a id="ama_rule_x"></a>'))
    missing_generator = subprocess.run([sys.executable, str(ROOT / 'tools/documentation/build.py'), '--doxygen', '/no/such/doxygen'], capture_output=True)
    require(missing_generator.returncode != 0 and b'not found' in missing_generator.stderr, 'missing generator did not fail')


def coverage_scenario():
    """Verify exact inventory retention, header acceptance/rejection contracts, compiler-rule requirements and declared actual test signatures; reject each invalid boundary."""
    root = prepare('coverage')
    home = root / 'docs/implementation/documentation'
    home.mkdir(parents=True)
    (root / 'TODO.md').write_text('- [ ] **T145 — Close debt.**\n')
    code = root / 'sample.h'
    code.write_text('/** @file\n * @brief Supply a harmless public declaration.\n */\nint sample(void);\n')
    case = root / 'case.c'
    case.write_text('void it_checks_sample(void) {}\n')
    (root / 'Makefile').write_text('.RECIPEPREFIX := >\nCOMPILE_COMMAND = cc -std=c11\n')
    entry = {'purpose': 'Fixture header', 'contract': 'Pure declaration',
             'tests': [{'source': 'case.c', 'case': 'it_checks_sample', 'command': 'fixture'}],
             'standalone_header_command': ['cc', '-std=c11', '-pedantic-errors', '-I.', '-x', 'c', '-fsyntax-only', '-']}
    ready = {'sample.h': entry}
    debt = {'case.c': {'sha256': hashlib.sha256(case.read_bytes()).hexdigest(), 'owner': 'T145', 'reason': 'fixture'}}
    def save():
        (home / 'ready.json').write_text(json.dumps(ready))
        (home / 'debt.json').write_text(json.dumps(debt))
    save()
    # Closing debt must not remove non-C expected observations from inventory.
    original_inventory = coverage.inventory
    original_listing = subprocess.check_output
    observation = root / 'tests/fixture/expected-error.txt'
    observation.parent.mkdir(parents=True)
    observation.write_text('exact diagnostic bytes\n')
    library_contract = root / 'stdlib/README.md'
    library_contract.parent.mkdir(parents=True)
    library_contract.write_text('# Retained library contract\n')
    def inventory_listing(command, cwd):
        require(command[0] == 'git' and cwd == root, 'unexpected inventory source')
        return b'sample.h\0case.c\0tests/fixture/expected-error.txt\0stdlib/README.md\0'
    try:
        (home / 'debt.json').write_text('{}')
        subprocess.check_output = inventory_listing
        maintained = original_inventory(root)
        require({'tests/fixture/expected-error.txt', 'stdlib/README.md'} <= maintained,
                'closed non-C inputs vanished with historical debt')
    finally:
        subprocess.check_output = original_listing
        save()
    coverage.inventory = lambda _: {'sample.h', 'case.c'}
    coverage.check(root)
    # Narrative page references cannot stand in for compiler requirement links.
    contract = root / 'docs/implementation/library-tests-and-build.md'
    contract.write_text('# Fixture contract\n')
    compiler = root / 'src/only_contract.c'
    compiler.parent.mkdir()
    compiler.write_text('/** @file\n * @brief Only a [fixture contract](../docs/implementation/library-tests-and-build.md).\n */\n')
    ready['src/only_contract.c'] = {'purpose': 'Missing rule fixture', 'contract': 'Fixture',
                                    'tests': entry['tests']}
    save()
    coverage.inventory = lambda _: {'sample.h', 'case.c', 'src/only_contract.c'}
    rejected(lambda: coverage.check(root))
    del ready['src/only_contract.c']
    save()
    coverage.inventory = lambda _: {'sample.h', 'case.c'}
    rejected(lambda: coverage.check(root, True))
    entry['tests'][0]['case'] = 'it_absent'
    save()
    rejected(lambda: coverage.check(root))
    entry['tests'][0]['case'] = 'it_checks_sample'
    save()
    coverage.inventory = lambda _: {'sample.h', 'case.c', 'unregistered.c'}
    rejected(lambda: coverage.check(root))
    coverage.inventory = lambda _: {'sample.h', 'case.c'}
    code.write_text(code.read_text() + 'typedef no_such_type invalid_header;\n')
    rejected(lambda: coverage.check(root))
    # A deliberate #error header is independently checked as rejection evidence,
    # never skipped or treated as an ordinary successfully compiled interface.
    negative = root / 'tests/fixture/project/rejected.h'
    negative.parent.mkdir(parents=True)
    negative.write_text('/** @file\n * @brief Reject forbidden native compilation.\n */\n#error Native compilation is forbidden\n')
    command = entry['standalone_header_command']
    contract = {'exit_code': 1, 'diagnostic': '#error Native compilation is forbidden'}
    coverage.check_header(root, 'tests/fixture/project/rejected.h', command, contract)
    rejected(lambda: coverage.check_header(root, 'sample.h', command, contract))
    rejected(lambda: coverage.check_header(root, 'tests/fixture/project/rejected.h', command, []))
    rejected(lambda: coverage.check_header(root, 'tests/fixture/project/rejected.h', command,
                                         {'exit_code': True, 'diagnostic': contract['diagnostic']}))
    rejected(lambda: coverage.check_header(root, 'tests/fixture/project/rejected.h', command,
                                         {'exit_code': 1, 'diagnostic': 'another failure'}))
    negative.write_text('/** @file\n * @brief No longer reject compilation.\n */\n')
    coverage.check_header(root, 'tests/fixture/project/rejected.h', command, None)
    rejected(lambda: coverage.check_header(root, 'tests/fixture/project/rejected.h', command, contract))
    negative.write_text('#error Native compilation is forbidden\n')
    rejected(lambda: coverage.check_header(root, 'tests/fixture/project/rejected.h',
                                         [sys.executable, '-c', 'raise SystemExit(127)'], contract))
    case.write_text(case.read_text() + '\n')
    rejected(lambda: coverage.check(root))


SCENARIOS = {'source': source, 'registry': registry, 'rendered': rendered, 'coverage': coverage_scenario}
SCENARIOS[sys.argv[2]]()
