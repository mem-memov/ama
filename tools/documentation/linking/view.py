## @file
# @brief Build read-only Doxygen input views with source-stable rule references.
#
# Edits are applied backwards to mapped documentation spans only. Maintained
# source and specification files are never opened for writing.
import re

from .markdown import compact_heading_levels, links
from .source import comments, mask_examples


def transform(path, content, registry):
    """Return a line-count-preserving view and the canonical anchors it uses."""
    edits, references = [], []
    if path.endswith('.md'):
        edits.extend(compact_heading_levels(content))
        regions = [(content, list(range(len(content))))]
        for rule, (chapter, fragment) in registry.rules.items():
            if chapter == path:
                anchor = '<a id="' + fragment + '"></a>'
                start = content.index(anchor)
                edits.append((start, start + len(anchor), '@anchor ' + registry.identity('rule', path, rule)))
        first_heading = re.search(r'(?m)^# [^\n]+', content)
        if first_heading:
            edits.append((first_heading.end(), first_heading.end(), ' {#' + registry.identity('page', path) + '}'))
    elif path.endswith(('.c', '.h', '.inc')):
        regions = comments(content)
    else:
        return content, []
    for fragment, physical in regions:
        for start, end, label, href in links(mask_examples(fragment)):
            strict = not path.endswith('.md')
            anchor = registry.reference(path, label, href, strict)
            if not anchor:
                continue
            begin, stop = physical[start], physical[end - 1] + 1
            replacement = '[' + label + '](@ref ' + anchor + ')'
            newline_count = content[begin:stop].count('\n') - replacement.count('\n')
            if newline_count < 0:
                raise ValueError('link rewrite would add source lines')
            replacement += '\n' * newline_count
            edits.append((begin, stop, replacement))
            references.append(anchor)
    previous = len(content) + 1
    for start, end, replacement in sorted(edits, reverse=True):
        if end > previous:
            raise ValueError('overlapping documentation edits')
        content = content[:start] + replacement + content[end:]
        previous = start
    return content, references
