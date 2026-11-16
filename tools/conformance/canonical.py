## @file
# @brief Read the complete canonical rule index and acceptance tables, retaining exact specification fingerprints.
import hashlib
from importlib.metadata import version

from markdown_it import MarkdownIt
from linking.registry import Registry


def fingerprints(root):
    """Fingerprint every specification file; any authorized change requires explicit evidence review."""
    return {path.relative_to(root).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted((root / 'spec').rglob('*')) if path.is_file()}


def acceptance(root):
    """Parse only the named canonical matrix with the pinned real Markdown table parser.

    Return ordered section/area/criteria records. Source examples, code fences and
    unrelated tables cannot create acceptance obligations. Criteria hashes retain
    the parser's complete inline Markdown rather than copying normative prose.
    """
    if version('markdown-it-py') != '3.0.0':
        raise ValueError('conformance requires markdown-it-py==3.0.0')
    source = (root / 'spec/implementation/tools-and-conformance.md').read_text()
    tokens = MarkdownIt('commonmark').enable('table').parse(source)
    active, section, rows, cells = False, '', [], None
    matrices, in_table = 0, False
    header, body, columns = False, False, None
    for index, token in enumerate(tokens):
        if token.type == 'heading_open' and token.level == 0:
            title = tokens[index + 1].content
            if token.tag == 'h2':
                active = title == 'Acceptance matrix'
                if active:
                    matrices += 1
            elif active and token.tag == 'h3':
                section = title
        if not active:
            continue
        if token.type == 'table_open' and token.level == 0:
            in_table = True
            columns = None
        elif token.type == 'table_close' and token.level == 0:
            in_table = False
        if not in_table:
            continue
        if token.type == 'thead_open':
            header = True
        elif token.type == 'thead_close':
            header = False
        elif token.type == 'tbody_open':
            body = True
        elif token.type == 'tbody_close':
            body = False
        elif token.type == 'tr_open':
            cells = []
        elif token.type == 'inline' and cells is not None:
            cells.append(token.content)
        elif token.type == 'tr_close':
            if header:
                if cells != ['Area', 'Required acceptance evidence']:
                    raise ValueError('unexpected canonical acceptance columns')
                columns = tuple(cells)
            elif body:
                if not section or columns is None or len(cells) != 2 or not all(cells):
                    raise ValueError('incomplete canonical acceptance row')
                rows.append({'section': section, 'area': cells[0],
                             'criteria_sha256': hashlib.sha256(cells[1].encode()).hexdigest()})
            cells = None
    if matrices != 1 or not rows or len({(row['section'], row['area']) for row in rows}) != len(rows):
        raise ValueError('missing or duplicate canonical acceptance rows')
    return rows


def requirements(root):
    """Reuse exact canonical ID/anchor/definition validation, never derive a filename from an ID prefix."""
    registry = Registry(root)
    return {name: path + '#' + fragment for name, (path, fragment) in registry.rules.items()}
