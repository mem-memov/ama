## @file
# @brief Orchestrate staged Doxygen generation and verify reproducibility.
#
# Doxygen renders every reference page. This script resolves canonical links,
# writes disposable input views, and validates actual rendered destinations.
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

from linking.registry import Registry
from linking.view import transform
from rendered import file_pages, interfaces, validate


def generate(root, destination, doxygen):
    """Generate from an isolated view; fail without editing maintained inputs."""
    registry = Registry(root)
    work = root / 'build/documentation'
    view = work / 'view'
    if view.exists():
        shutil.rmtree(view)
    view.mkdir(parents=True)
    inputs = []
    expected = {}
    maintained = sorted(path for base in ['src', 'stdlib/modules', 'tools', 'tests', 'spec', 'examples']
                        for path in (root / base).rglob('*')
                        if path.is_file() and path.suffix in ('.c', '.h', '.inc', '.py', '.md')
                        and not {'generated', 'build', '__pycache__'}.intersection(path.relative_to(root).parts))
    # Keep specification authority separate from derived narrative documents;
    # include the explicitly referenced fixture contract alongside the entry page.
    maintained = [path for path in maintained if path.suffix != '.md' or path.is_relative_to(root / 'spec')]
    maintained.extend(root / name for name in registry.pages)
    hashes = {}
    for path in maintained:
        relative = path.relative_to(root).as_posix()
        original = path.read_text()
        content, references = transform(relative, original, registry)
        if content.count('\n') != original.count('\n'):
            raise ValueError('source line count changed: ' + relative)
        staged = view / relative
        staged.parent.mkdir(parents=True, exist_ok=True)
        staged.write_text(content)
        inputs.append(relative)
        if references:
            origin = registry.identity('page', relative) if relative.endswith('.md') else relative
            expected[origin] = references
        hashes[relative] = hashlib.sha256(path.read_bytes()).hexdigest()
    config = (root / 'docs/Doxyfile').read_text()
    config += '\nINPUT = ' + ' '.join('"' + name + '"' for name in inputs) + '\n'
    config += 'OUTPUT_DIRECTORY = "' + str(destination) + '"\n'
    config += 'STRIP_FROM_PATH = "' + str(view) + '"\n'
    config += 'GENERATE_TAGFILE = "' + str(destination / 'ama.tag') + '"\n'
    config += 'WARN_LOGFILE = "' + str(work / 'warnings.log') + '"\n'
    config += 'MARKDOWN_ID_STYLE = GITHUB\n'
    configuration = work / 'Doxyfile'
    configuration.write_text(config)
    destination.mkdir(parents=True, exist_ok=True)
    subprocess.run([doxygen, str(configuration)], cwd=view, check=True)
    rule_pages = {registry.identity('rule', chapter, rule): registry.identity('page', chapter)
                  for rule, (chapter, _) in registry.rules.items()}
    result = validate(destination, expected, rule_pages)
    rendered_files = file_pages(destination)
    required_files = {name for name in inputs if name.endswith(('.c', '.h', '.inc', '.py'))}
    if not required_files.issubset(rendered_files):
        raise ValueError('Doxygen omitted maintained file pages: ' + ', '.join(sorted(required_files - rendered_files.keys())))
    result['maintained_file_pages'] = len(required_files)
    result['documented_interfaces'] = interfaces(destination)
    for relative, digest in hashes.items():
        if hashlib.sha256((root / relative).read_bytes()).hexdigest() != digest:
            raise ValueError('maintained input changed during generation: ' + relative)
    result['inputs_sha256'] = hashes
    result['doxygen'] = subprocess.check_output([doxygen, '--version'], text=True).strip()
    (work / 'report.json').write_text(json.dumps(result, indent=2, sort_keys=True) + '\n')
    print(json.dumps({key: value for key, value in result.items() if key != 'inputs_sha256'}, sort_keys=True))


def main():
    """Parse the explicit Doxygen/reproducibility request, generate complete reference output and optionally compare two complete byte manifests; failures propagate as unsuccessful process status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--doxygen', default='doxygen')
    parser.add_argument('--reproducible', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    executable = shutil.which(args.doxygen)
    if not executable:
        raise ValueError('Doxygen executable not found: ' + args.doxygen)
    output = root / 'docs/technical'
    if output.exists():
        shutil.rmtree(output)
    generate(root, output, executable)
    if args.reproducible:
        baseline = {path.relative_to(output): hashlib.sha256(path.read_bytes()).hexdigest()
                    for path in output.rglob('*') if path.is_file()}
        shutil.rmtree(output)
        generate(root, output, executable)
        repeated = {path.relative_to(output): hashlib.sha256(path.read_bytes()).hexdigest()
                    for path in output.rglob('*') if path.is_file()}
        if baseline != repeated:
            changed = sorted(str(path) for path in baseline.keys() | repeated.keys() if baseline.get(path) != repeated.get(path))
            raise ValueError('nonreproducible Doxygen outputs: ' + ', '.join(changed))
        print('Doxygen outputs are byte-identical across two complete generations')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print('documentation: ' + str(error), file=sys.stderr)
        sys.exit(1)
