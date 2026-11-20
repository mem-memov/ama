## @file
# @brief Enforce explicit per-file documentation/test ownership and visible debt.
#
# The frozen T001 audit is historical evidence. This current registry instead
# partitions maintained inputs into reviewed ready entries and fingerprinted debt.
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

from linking.registry import Registry
from linking.source import comments, mask_examples
from linking.markdown import links
from linking.view import transform


def inventory(root):
    """Enumerate maintained inputs using the VCS, never generated build trees."""
    output = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=root)
    result = set()
    historical = json.loads((root / 'docs/implementation/documentation/debt.json').read_text())
    for encoded in output.split(b'\0'):
        if not encoded:
            continue
        relative = encoded.decode()
        path = Path(relative)
        if {'generated', 'build', '__pycache__'}.intersection(path.parts):
            continue
        is_code = path.suffix in ('.c', '.h', '.inc', '.py', '.sh', '.mk', '.abi', '.exports') or path.name in ('Makefile', 'Doxyfile')
        is_fixture = (relative.startswith('tests/') and '/project/' in relative) or (relative.startswith('stdlib/modules/') and path.suffix != '.md')
        is_ama_example = path.suffix == '' and relative.startswith('examples/')
        is_observation = (relative.startswith('tests/') and path.name in (
            'expected.txt', 'expected-error.txt', 'expected-abi-error.txt',
            'expected-ambiguity.txt'))
        is_library_contract = relative == 'stdlib/README.md'
        if is_code or is_fixture or is_ama_example or is_observation or is_library_contract or relative in historical:
            if (root / relative).is_file():
                result.add(relative)
    return result


def check_test_interfaces(root, ready):
    """Compile exact registered case signatures in each real leaf Make context.

    Names in comments or string literals cannot satisfy this check. The compiler
    resolves the included test translation unit with its actual fixture flags.
    """
    cases = {}
    for entry in ready.values():
        for test in entry['tests']:
            cases.setdefault(test['source'], set()).add(test['case'])
    workspace = root / 'build/documentation'
    workspace.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='interfaces-', dir=workspace) as temporary:
        temporary = Path(temporary)
        makefile = temporary / 'probe.mk'
        makefile.write_text('.RECIPEPREFIX := >\n.PHONY: documentation-interface-check\n'
                            'documentation-interface-check:\n'
                            '>    $(filter-out -MMD -MP,$(COMPILE_COMMAND)) -fsyntax-only "$(DOCUMENTATION_PROBE)"\n')
        for index, (source, names) in enumerate(sorted(cases.items())):
            probe = temporary / (str(index) + '.c')
            content = '#define main ama_documentation_original_main\n#include "' + str(root / source) + '"\n#undef main\n'
            for name in sorted(names):
                if not re.fullmatch(r'it_[A-Za-z0-9_]+', name):
                    raise ValueError('invalid registered C case identifier: ' + name)
                content += '_Static_assert(_Generic(&' + name + ', void (*)(void): 1, default: 0), "test signature");\n'
            probe.write_text(content)
            subprocess.run(['make', '--no-print-directory', '-s', '-C', str((root / source).parent),
                            '-f', 'Makefile', '-f', str(makefile),
                            'DOCUMENTATION_PROBE=' + str(probe), 'documentation-interface-check'], check=True)



def check_header(root, path, command, expected_failure):
    """Compile the actual header; explicit rejection fixtures must fail as declared.

    Only maintained test-project headers may declare a negative contract. Check
    the exact compiler exit and diagnostic text, so a missing tool, unexpected
    acceptance or unrelated failure cannot become positive coverage evidence.
    Ordinary headers still require successful independent C compilation.
    """
    # A header may intentionally contain only macros or companion markers. A
    # neutral C11 declaration keeps that valid header from making an empty TU;
    # it supplies no missing type, namespace or definition to the header itself.
    source = ('#include "' + path + '"\n'
              '_Static_assert(1, "Standalone header probe");\n')
    if expected_failure is None:
        subprocess.run(command, cwd=root, input=source, text=True, check=True)
        return
    if (not path.startswith('tests/') or '/project/' not in path
            or not isinstance(expected_failure, dict)
            or set(expected_failure) != {'exit_code', 'diagnostic'}
            or type(expected_failure['exit_code']) is not int
            or expected_failure['exit_code'] != 1
            or not isinstance(expected_failure['diagnostic'], str)
            or not expected_failure['diagnostic']):
        raise ValueError('invalid negative header contract: ' + path)
    completed = subprocess.run(command, cwd=root, input=source, text=True,
                               capture_output=True, check=False)
    if (completed.returncode != expected_failure['exit_code']
            or expected_failure['diagnostic'] not in completed.stderr):
        raise ValueError('negative header contract was not observed: ' + path)


def check_rule_contracts(path, content, registry):
    """Require explicit canonical context on every compiler Doxygen contract.

    This uniform stronger check avoids guessing which function is nontrivial.
    Narrow rule implementations cite their rules; generic helpers can explicitly
    cite the surrounding component's declared rules. Existing purpose/invariant
    prose remains necessary. Examples and literal links never supply a contract.
    """
    for fragment, positions in comments(content):
        targets = [registry.reference(path, label, href, True)
                   for _, _, label, href in links(mask_examples(fragment))]
        if not any(target and target.startswith('ama_rule_') for target in targets):
            line = content.count('\n', 0, positions[0]) + 1 if positions else 1
            raise ValueError('compiler contract lacks canonical rule reference: ' + path + ':' + str(line))


def check(root, complete=False):
    """Reject unregistered files/stale evidence; full mode also rejects all debt."""
    home = root / 'docs/implementation/documentation'
    debt = json.loads((home / 'debt.json').read_text())
    ready = json.loads((home / 'ready.json').read_text())
    actual = inventory(root)
    if set(debt) & set(ready):
        raise ValueError('file classified both ready and debt')
    if actual != set(debt) | set(ready):
        raise ValueError('unregistered/deleted inputs: ' + ', '.join(sorted(actual ^ (set(debt) | set(ready)))))
    todo = (root / 'TODO.md').read_text()
    for path, entry in debt.items():
        if hashlib.sha256((root / path).read_bytes()).hexdigest() != entry['sha256']:
            raise ValueError('changed debt needs explicit review: ' + path)
        if not re.search(r'\*\*' + entry['owner'] + r' —', todo):
            raise ValueError('unknown debt owner: ' + path)
        if not entry['reason']:
            raise ValueError('missing debt explanation: ' + path)
    registry = Registry(root)
    for path, entry in ready.items():
        if not entry.get('purpose') or not entry.get('contract') or not entry.get('tests'):
            raise ValueError('missing reviewed purpose/contract/tests: ' + path)
        for test in entry['tests']:
            if not (root / test['source']).is_file() or not test['case'] or not test['command']:
                raise ValueError('missing executable test evidence: ' + path)
        if path.endswith(('.c', '.h', '.inc')):
            content = (root / path).read_text()
            prose = '\n'.join(fragment for fragment, _ in comments(content))
            if not re.search(r'[@\\]file\b', prose) or not re.search(r'[@\\]brief\s+\S', prose):
                raise ValueError('missing Doxygen file purpose: ' + path)
            _, targets = transform(path, content, registry)
            if path.startswith('src/') and not any(target.startswith('ama_rule_') for target in targets):
                raise ValueError('compiler purpose lacks canonical rule reference: ' + path)
            if path.startswith('src/'):
                check_rule_contracts(path, content, registry)
        if path.endswith('.h'):
            command = entry.get('standalone_header_command')
            if not command:
                raise ValueError('missing independent header compilation: ' + path)
            check_header(root, path, command, entry.get('header_expected_failure'))
    check_test_interfaces(root, ready)
    if complete and debt:
        raise ValueError(str(len(debt)) + ' explicitly owned documentation/coverage debts remain')
    print(str(len(ready)) + ' ready inputs checked; ' + str(len(debt)) + ' explicit debts remain')


def main():
    """Parse the complete-coverage gate and validate the exact maintained input partition, independent header contracts and actual named test signatures."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--complete', action='store_true')
    args = parser.parse_args()
    check(Path(__file__).resolve().parents[2], args.complete)


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print('documentation coverage: ' + str(error), file=sys.stderr)
        sys.exit(1)
