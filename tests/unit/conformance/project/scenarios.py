## @file
# @brief Supply private real repositories and independent evidence-failure observations to the explicit C/assert conformance leaf.
import copy
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(sys.argv[1]).resolve()
sys.path.insert(0, str(ROOT / 'tools/documentation'))
sys.path.insert(0, str(ROOT / 'tools/conformance'))
import canonical
import check
import evidence
import schema


def require(condition, message):
    """Report one independently expected observation to the C process owner."""
    if not condition:
        raise ValueError(message)


def rejected(operation):
    """Require a declared validation, IO or actual compiler failure; normal return is never refusal evidence."""
    try:
        operation()
    except (ValueError, OSError, subprocess.CalledProcessError):
        return
    raise ValueError('invalid evidence was accepted')


def fixture(name):
    """Create one isolated real VCS/Make/C evidence domain; no maintained source/spec file is edited."""
    root = (Path('generated') / ('проверка ' + name)).resolve()
    if root.exists():
        shutil.rmtree(root)
    (root / 'spec/implementation').mkdir(parents=True)
    (root / 'docs/implementation/documentation').mkdir(parents=True)
    (root / 'spec/requirements.md').write_text('# Index\n\n[FIXTURE-01](fixture.md#fixture-01)\n')
    (root / 'spec/fixture.md').write_text('# Model\n\n<a id="fixture-01"></a>\n\n**FIXTURE-01.** Identity preserves its argument.\n')
    (root / 'spec/implementation/tools-and-conformance.md').write_text(
        '# Checks\n\n```md\n## Acceptance matrix\n| fake | ignored |\n```\n\n'
        '> ## Acceptance matrix\n> | Area | Required acceptance evidence |\n> |---|---|\n> | Ignored | Quoted example |\n\n'
        '## Acceptance matrix\n\n### Values\n\n| Area | Required acceptance evidence |\n|---|---|\n'
        '| Identity | Keep `a\\|b` unchanged. |\n| Composition | Nested identity is unchanged. |\n')
    (root / 'contract.md').write_text('# Identity argument\n\nA finite total identity function preserves each nested value.\n')
    (root / 'sample.h').write_text('/** @file @brief Declare the fixture value domain. */\n#define FIXTURE_VALUE 3\n')
    (root / 'case.c').write_text(
        '/** @file @brief Exercise actual fixture component/composition interfaces. */\n#include <assert.h>\n#include "sample.h"\n'
        'static int identity(int value) { return value; }\n'
        'static void it_component(void) { assert(identity(FIXTURE_VALUE) == 3); }\n'
        'static void it_composition(void) { assert(identity(identity(FIXTURE_VALUE)) == 3); }\n'
        'void run(void) { it_component(); it_composition(); }\nint main(void) { run(); return 0; }\n')
    (root / 'Makefile').write_text('.RECIPEPREFIX := >\nCOMPILE_COMMAND = cc -std=c11 -Wall -Wextra -Werror -UNDEBUG -I.\n')
    tests = [{'source': 'case.c', 'case': name, 'command': 'make -C .'}
             for name in ('it_component', 'it_composition')]
    ready = {path: {'purpose': 'Fixture identity domain', 'contract': 'contract.md', 'tests': tests}
             for path in ('case.c', 'sample.h', 'Makefile')}
    home = root / 'docs/implementation/documentation'
    (home / 'ready.json').write_text(json.dumps(ready))
    (home / 'debt.json').write_text('{}')
    subprocess.run(['git', 'init', '-q', str(root)], check=True)
    group = {'purpose': 'Fixture identity and nested composition', 'contracts': ['contract.md'],
             'sources': ['case.c', 'sample.h'], 'component': tests[:1], 'composition': tests[1:]}
    rows = [dict(row, id='ACC-' + str(index).zfill(3), status='covered',
                 argument='Nested values follow the identity law.', groups=['identity'])
            for index, row in enumerate(canonical.acceptance(root), 1)]
    matrix = {'schema': 'ama.conformance.v1', 'specification': {}, 'inputs': {}, 'arguments': {},
              'registry_sha256': '', 'groups': {'identity': group},
              'requirements': {'FIXTURE-01': {'canonical': 'spec/fixture.md#fixture-01',
                  'status': 'covered', 'argument': 'The total identity preserves its input.', 'groups': ['identity']}},
              'acceptance': rows}
    refresh(root, matrix)
    return root, matrix


def refresh(root, matrix):
    """Explicitly stage the fixture's independently changed bytes for a later narrow validator test."""
    ready = schema.load(root / 'docs/implementation/documentation/ready.json')
    matrix['specification'] = canonical.fingerprints(root)
    matrix['inputs'] = evidence.fingerprints(root, ready)
    matrix['registry_sha256'] = evidence.fingerprints(root, ['docs/implementation/documentation/ready.json'])['docs/implementation/documentation/ready.json']
    matrix['arguments'] = evidence.fingerprints(root, {path for group in matrix['groups'].values() for path in group['contracts']})


def validate(root, matrix, complete=True, compile_interfaces=False):
    """Run the actual checker with one fixture manifest, separating metadata tests from requested real C compilation."""
    path = root / 'matrix.json'
    path.write_text(json.dumps(matrix, ensure_ascii=True))
    return check.check(root, path, complete, compile_interfaces)


def schema_cases():
    """Reject JSON extensions, duplicate keys, wrong schema/fields and unsupported or open statuses."""
    root, original = fixture('schema')
    validate(root, original)
    raw = root / 'duplicate.json'
    for text in ('{"a":1,"a":2}', '{"a":NaN}', '{"a":Infinity}', '{"a":-Infinity}'):
        raw.write_text(text)
        rejected(lambda: schema.load(raw))
    for value in (None, '', ' ', '\0', '\ud800'):
        rejected(lambda: schema.text(value, 'fixture'))
    variants = []
    item = copy.deepcopy(original); item['schema'] = 'unknown'; variants.append(item)
    item = copy.deepcopy(original); item['extra'] = True; variants.append(item)
    item = copy.deepcopy(original); item['requirements']['FIXTURE-01']['status'] = 'assumed'; variants.append(item)
    item = copy.deepcopy(original); item['requirements']['FIXTURE-01']['argument'] = ' '; variants.append(item)
    for item in variants:
        rejected(lambda: validate(root, item))
    opened = copy.deepcopy(original)
    opened['requirements']['FIXTURE-01']['status'] = 'open'
    require(not validate(root, opened, False)['complete'], 'partial mode falsely claimed completion')
    rejected(lambda: validate(root, opened))


def record_cases():
    """Match complete exact canonical rule and ordered acceptance inventories without dropped or repaired obligations."""
    root, original = fixture('records')
    variants = []
    item = copy.deepcopy(original); item['requirements'].clear(); variants.append(item)
    item = copy.deepcopy(original); item['requirements']['EXTRA-01'] = item['requirements']['FIXTURE-01']; variants.append(item)
    item = copy.deepcopy(original); item['requirements']['FIXTURE-01']['canonical'] = 'spec/fixture.md#missing'; variants.append(item)
    item = copy.deepcopy(original); item['acceptance'].pop(); variants.append(item)
    item = copy.deepcopy(original); item['acceptance'].reverse(); variants.append(item)
    item = copy.deepcopy(original); item['acceptance'][0]['criteria_sha256'] = '0' * 64; variants.append(item)
    item = copy.deepcopy(original); item['acceptance'][0]['id'] = 'ACC-999'; variants.append(item)
    item = copy.deepcopy(original); item['groups']['orphan'] = item['groups']['identity']; variants.append(item)
    item = copy.deepcopy(original); item['requirements']['FIXTURE-01']['groups'] = ['missing']; variants.append(item)
    for item in variants:
        rejected(lambda: validate(root, item))


def provenance_cases():
    """Changed code, spec, registration, proof or file membership cannot reuse old authority."""
    root, original = fixture('provenance')
    for name in ('case.c', 'sample.h', 'spec/fixture.md', 'contract.md', 'docs/implementation/documentation/ready.json'):
        path = root / name
        before = path.read_bytes()
        path.write_bytes(before + b'\n')
        rejected(lambda: validate(root, original))
        path.write_bytes(before)
    extra = root / 'unregistered.c'
    extra.write_text('int extra;\n')
    rejected(lambda: validate(root, original))
    extra.unlink()
    validate(root, original)


def interface_cases():
    """Reject fabricated evidence and compile the real declared C function types, not names in prose."""
    root, original = fixture('interfaces')
    validate(root, original, compile_interfaces=True)
    variants = []
    item = copy.deepcopy(original); item['groups']['identity']['sources'] = ['absent.c']; variants.append(item)
    item = copy.deepcopy(original); item['groups']['identity']['component'][0]['case'] = 'it_absent'; variants.append(item)
    item = copy.deepcopy(original); item['groups']['identity']['composition'] = []; variants.append(item)
    item = copy.deepcopy(original); item['groups']['identity']['composition'] = item['groups']['identity']['component']; variants.append(item)
    item = copy.deepcopy(original); item['groups']['identity']['component'][0]['command'] = 'pretend'; variants.append(item)
    for item in variants:
        rejected(lambda: validate(root, item))
    source = root / 'case.c'
    source.write_text(source.read_text().replace('static void it_component(void) { assert(identity(FIXTURE_VALUE) == 3); }',
        'static int it_component(void) { assert(identity(FIXTURE_VALUE) == 3); return 0; }'))
    changed = copy.deepcopy(original)
    refresh(root, changed)
    rejected(lambda: validate(root, changed, compile_interfaces=True))


def canonical_cases():
    """Use real pinned Markdown and exact anchor validation, including nested/fenced false matrices."""
    root, matrix = fixture('canonical')
    require(canonical.requirements(root) == {'FIXTURE-01': 'spec/fixture.md#fixture-01'}, 'canonical rule lost')
    rows = canonical.acceptance(root)
    require(len(rows) == 2 and rows[0]['section'] == 'Values' and rows[0]['area'] == 'Identity', 'fake matrix became canonical')
    require(rows[0]['criteria_sha256'] == hashlib.sha256(b'Keep `a|b` unchanged.').hexdigest(), 'escaped table criterion changed')
    path = root / 'spec/implementation/tools-and-conformance.md'
    original = path.read_text()
    for changed in (original + '\n## Acceptance matrix\n',
                    original.rstrip('\n') + '\n| Identity | Duplicate |\n'):
        path.write_text(changed)
        rejected(lambda: canonical.acceptance(root))
    path.write_text(original)
    rule = root / 'spec/fixture.md'
    rule.write_text(rule.read_text() + '<a id="fixture-01"></a>\n')
    rejected(lambda: canonical.requirements(root))


def current():
    """Require the complete actually reviewed repository matrix and all selected real C interfaces."""
    result = check.check(ROOT, ROOT / 'docs/implementation/conformance/matrix.json', True)
    require(result['requirements'] == 200 and result['acceptance_rows'] == 108 and result['complete'],
            'current conformance inventory is incomplete')


SCENARIOS = {'schema': schema_cases, 'records': record_cases, 'provenance': provenance_cases,
             'interfaces': interface_cases, 'canonical': canonical_cases, 'current': current}
SCENARIOS[sys.argv[2]]()
