## @file
# @brief Join reviewed requirement evidence to actual maintained files and executable C case registrations.
from pathlib import Path
import hashlib

import schema


def fingerprints(root, paths):
    """Retain exact reviewed file bytes, so changed code, tests or arguments cannot reuse an old coverage claim."""
    return {path: hashlib.sha256((root / path).read_bytes()).hexdigest() for path in sorted(paths)}


def contract(root, path):
    """Require a real nonempty repository argument document, not an external or generated substitute."""
    schema.text(path, 'argument path')
    destination = root / path
    if (Path(path).is_absolute() or not destination.resolve().is_relative_to(root)
            or {'generated', 'build', 'technical'}.intersection(Path(path).parts)
            or not destination.is_file() or not destination.read_text().strip()):
        raise ValueError('missing/invalid component argument: ' + path)


def registered_cases(ready):
    """Retain complete source/case/command triples; matching names from unrelated files grant no evidence."""
    return {(test['source'], test['case'], test['command'])
            for entry in ready.values() for test in entry['tests']}


def group(root, name, value, ready, registered):
    """Validate concrete code and both declared component/composition witnesses for one reviewed boundary."""
    schema.fields(value, ('purpose', 'contracts', 'sources', 'component', 'composition'), name)
    schema.text(value['purpose'], name)
    schema.strings(value['contracts'], name)
    for path in value['contracts']:
        contract(root, path)
    schema.strings(value['sources'], name)
    for path in value['sources']:
        if path not in ready or not (root / path).is_file():
            raise ValueError('unregistered conformance source: ' + path)
    selected = []
    scales = []
    for scale in ('component', 'composition'):
        tests = value[scale]
        if not isinstance(tests, list) or not tests:
            raise ValueError('missing ' + scale + ' evidence for ' + name)
        seen = set()
        for test in tests:
            schema.fields(test, ('source', 'case', 'command'), name)
            for field in test.values():
                schema.text(field, name)
            key = (test['source'], test['case'], test['command'])
            if key not in registered:
                raise ValueError('unregistered conformance test: ' + repr(key))
            if key in seen:
                raise ValueError('duplicate conformance test in ' + name)
            seen.add(key)
            selected.append(test)
        scales.append(seen)
    if scales[0] & scales[1]:
        raise ValueError('component and composition evidence must be distinct in ' + name)
    return selected
