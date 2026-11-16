## @file
# @brief Check exact current requirement/acceptance evidence and compile its actual named test interfaces.
import argparse
import json
from pathlib import Path
import sys
import subprocess

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'documentation'))
from coverage import check_test_interfaces, inventory
import canonical
import evidence
import schema


def requirement_rows(matrix, actual, complete):
    """Require a bijection with canonical IDs and exact targets; review status never follows from a prefix match."""
    rows = matrix['requirements']
    if not isinstance(rows, dict) or set(rows) != set(actual):
        raise ValueError('requirement inventory differs from canonical index')
    for name, row in rows.items():
        schema.fields(row, ('canonical', 'status', 'argument', 'groups'), name)
        if row['canonical'] != actual[name]:
            raise ValueError('noncanonical requirement target: ' + name)
        schema.record({key: row[key] for key in ('status', 'argument', 'groups')}, name, matrix['groups'], complete)


def acceptance_rows(matrix, actual, complete):
    """Match every ordered canonical row and its complete criteria digest exactly once."""
    rows = matrix['acceptance']
    if not isinstance(rows, list) or len(rows) != len(actual):
        raise ValueError('acceptance inventory differs from canonical matrix')
    for index, (row, original) in enumerate(zip(rows, actual), 1):
        name = 'ACC-' + str(index).zfill(3)
        schema.fields(row, ('id', 'section', 'area', 'criteria_sha256', 'status', 'argument', 'groups'), name)
        if row['id'] != name or any(row[key] != original[key] for key in original):
            raise ValueError('noncanonical acceptance row: ' + name)
        schema.record({key: row[key] for key in ('status', 'argument', 'groups')}, name, matrix['groups'], complete)


def provenance(root, matrix, ready):
    """Require the complete live maintained inventory and reviewed specification/code/test/argument bytes."""
    if set(ready) != inventory(root):
        raise ValueError('maintained inventory changed; documentation review is required')
    if matrix['specification'] != canonical.fingerprints(root):
        raise ValueError('specification changed; conformance review is required')
    if matrix['inputs'] != evidence.fingerprints(root, ready):
        raise ValueError('implementation/test inputs changed; conformance review is required')
    if matrix['registry_sha256'] != evidence.fingerprints(root, ['docs/implementation/documentation/ready.json'])['docs/implementation/documentation/ready.json']:
        raise ValueError('case registrations changed; conformance review is required')
    arguments = {path for group in matrix['groups'].values() for path in group['contracts']}
    if matrix['arguments'] != evidence.fingerprints(root, arguments):
        raise ValueError('component arguments changed; conformance review is required')


def check(root, manifest, complete=False, compile_interfaces=True):
    """Validate exact specification, records, concrete evidence and actual C case types without executing Ama or writing sources."""
    matrix = schema.load(manifest)
    schema.fields(matrix, ('schema', 'specification', 'inputs', 'arguments', 'registry_sha256', 'groups', 'requirements', 'acceptance'), 'matrix')
    if matrix['schema'] != 'ama.conformance.v1':
        raise ValueError('unknown conformance schema')
    if matrix['specification'] != canonical.fingerprints(root):
        raise ValueError('specification changed; conformance review is required')
    if not isinstance(matrix['groups'], dict) or not matrix['groups']:
        raise ValueError('missing conformance evidence groups')
    requirement_rows(matrix, canonical.requirements(root), complete)
    acceptance_rows(matrix, canonical.acceptance(root), complete)
    ready = schema.load(root / 'docs/implementation/documentation/ready.json')
    registered = evidence.registered_cases(ready)
    checks = {}
    used = {name for row in list(matrix['requirements'].values()) + matrix['acceptance'] for name in row['groups']}
    if used != set(matrix['groups']):
        raise ValueError('unused evidence group has no reviewed obligation')
    for name, group in matrix['groups'].items():
        checks[name] = {'tests': evidence.group(root, name, group, ready, registered)}
    provenance(root, matrix, ready)
    if compile_interfaces:
        check_test_interfaces(root, checks)
    provenance(root, matrix, ready)
    return {'requirements': len(matrix['requirements']), 'acceptance_rows': len(matrix['acceptance']),
            'groups': len(matrix['groups']), 'complete': complete}


def main():
    """Expose the current complete gate and an explicit alternate manifest for independently tested refusal cases."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--manifest', type=Path)
    parser.add_argument('--complete', action='store_true')
    arguments = parser.parse_args()
    root = arguments.root.resolve()
    manifest = arguments.manifest or root / 'docs/implementation/conformance/matrix.json'
    print(json.dumps(check(root, manifest, arguments.complete), sort_keys=True))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, TypeError, KeyError, subprocess.CalledProcessError) as error:
        print('conformance: ' + str(error), file=sys.stderr)
        sys.exit(1)
