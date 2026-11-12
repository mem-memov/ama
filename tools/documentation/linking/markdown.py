## @file
# @brief Locate real Markdown links while preserving physical source positions.
#
# CommonMark's parser owns block and inline recognition. Wrappers record offsets
# while nested list/quote state still exists; no substring search guesses which
# occurrence is prose. The pinned parser version is part of this tool's contract.
from importlib.metadata import version

from markdown_it import MarkdownIt
from markdown_it.rules_block import heading, lheading, paragraph, table
from markdown_it.rules_inline import link


def _trim(text, offsets):
    """Remove exactly Python strip whitespace from both mapped boundaries."""
    start = len(text) - len(text.lstrip())
    end = len(text.rstrip())
    return text[start:end], offsets[start:end]


def _lines(state, first, last):
    """Mirror StateBlock.getLines with a physical offset for every character."""
    text, offsets = [], []
    for line in range(first, last):
        start = state.bMarks[line]
        end = state.eMarks[line]
        indent = 0
        while start < end and indent < state.blkIndent:
            char = state.src[start]
            if char == ' ':
                indent += 1
            elif char == '\t':
                indent += 4 - (indent + state.bsCount[line]) % 4
            elif start - state.bMarks[line] < state.tShift[line]:
                indent += 1
            else:
                break
            start += 1
        if indent > state.blkIndent:
            text.extend(' ' * (indent - state.blkIndent))
            offsets.extend([start - 1] * (indent - state.blkIndent))
        text.extend(state.src[start:end])
        offsets.extend(range(start, end))
        if line + 1 < last:
            text.append('\n')
            offsets.append(end)
    return _trim(''.join(text), offsets)


def _cells(state, line):
    """Split table cells exactly as markdown-it 3.0.0 escapedSplit does."""
    start = state.bMarks[line] + state.tShift[line]
    end = state.eMarks[line]
    text, offsets = _trim(state.src[start:end], list(range(start, end)))
    cells, chars, positions = [], [], []
    escaped = False
    for index, char in enumerate(text):
        if char == '|' and not escaped:
            cells.append((''.join(chars), positions))
            chars, positions = [], []
        elif char == '|' and escaped:
            chars[-1:] = ['|']
            positions[-1:] = [offsets[index]]
        else:
            chars.append(char)
            positions.append(offsets[index])
        escaped = char == '\\'
    cells.append((''.join(chars), positions))
    if cells and not cells[0][0]:
        cells.pop(0)
    if cells and not cells[-1][0]:
        cells.pop()
    return [_trim(*cell) for cell in cells]


def links(source):
    """Return (start, end, label, URL) for links outside Markdown code/images.

    Offsets index the unchanged caller string. Unsupported parser versions fail
    closed rather than silently changing the mapping algorithm.
    """
    if version('markdown-it-py') != '3.0.0':
        raise ValueError('documentation requires markdown-it-py==3.0.0')
    parser = MarkdownIt('commonmark').enable('table')
    collected = []

    def wrap_block(original, kind):
        def wrapped(state, first, last, silent):
            before = len(state.tokens)
            accepted = original(state, first, last, silent)
            if not accepted or silent:
                return accepted
            row_counts = {}
            for token in state.tokens[before:]:
                if token.type != 'inline' or 'physical' in token.meta:
                    continue
                begin, end = token.map
                if kind == 'table':
                    number = row_counts.get(begin, 0)
                    cells = _cells(state, begin)
                    fragment, positions = cells[number] if number < len(cells) else ('', [])
                    row_counts[begin] = number + 1
                elif kind == 'heading':
                    start = state.bMarks[begin] + state.tShift[begin]
                    stop = state.eMarks[begin]
                    while start < stop and state.src[start] == '#':
                        start += 1
                    while stop > start and state.src[stop - 1] in ' \t':
                        stop -= 1
                    hashes = stop
                    while hashes > start and state.src[hashes - 1] == '#':
                        hashes -= 1
                    if hashes > start and state.src[hashes - 1] in ' \t':
                        stop = hashes
                    fragment, positions = _trim(state.src[start:stop], list(range(start, stop)))
                else:
                    fragment, positions = _lines(state, begin, end)
                if fragment != token.content:
                    raise ValueError('Markdown parser/source-map disagreement')
                token.meta['physical'] = positions
            return accepted
        return wrapped

    for name, function in [('paragraph', paragraph), ('heading', heading),
                           ('lheading', lheading), ('table', table)]:
        parser.block.ruler.at(name, wrap_block(function, name))

    def capture(state, silent):
        start, before = state.pos, len(state.tokens)
        label_end = state.md.helpers.parseLinkLabel(state, start, True) if state.src[start:start + 1] == '[' else -1
        accepted = link(state, silent)
        if accepted and not silent and state.tokens is state.env.get('root_tokens'):
            opened = next(token for token in state.tokens[before:] if token.type == 'link_open')
            mapping = state.env['physical']
            collected.append((mapping[start], mapping[state.pos - 1] + 1,
                              state.src[start + 1:label_end], opened.attrGet('href')))
        return accepted

    parser.inline.ruler.at('link', capture)

    def inline(state):
        for token in state.tokens:
            if token.type != 'inline':
                continue
            children = []
            environment = dict(state.env, physical=token.meta['physical'], root_tokens=children)
            parser.inline.parse(token.content, parser, environment, children)
            token.children = children

    parser.core.ruler.at('inline', inline)
    # markdown-it normalizes CRLF and NUL. Reject them explicitly rather than
    # returning offsets into a different string. Maintained inputs use LF/UTF-8.
    if '\r' in source or '\0' in source:
        raise ValueError('documentation inputs require LF and no NUL')
    parser.parse(source)
    return collected


def compact_heading_levels(source):
    """Return exact ATX prefix edits preserving the original heading parent tree.

    CommonMark identifies headings, so fenced examples and nested containers are
    never guessed from a hash substring. The parent is the nearest preceding
    top-level heading of lower original rank. Assign parent display rank plus one
    to eliminate absent intermediate ranks in Doxygen's temporary view. Titles,
    order, parent relations, GitHub title anchors and line counts stay unchanged.
    """
    if version('markdown-it-py') != '3.0.0':
        raise ValueError('documentation requires markdown-it-py==3.0.0')
    if '\r' in source or '\0' in source:
        raise ValueError('documentation inputs require LF and no NUL')
    # CommonMark line maps count LF, not every Unicode splitlines separator.
    lines = source.split('\n')
    starts, offset = [], 0
    for line in lines:
        starts.append(offset)
        offset += len(line) + 1
    stack, edits = [], []
    for token in MarkdownIt('commonmark').parse(source):
        if token.type != 'heading_open' or token.level != 0:
            continue
        original = int(token.tag[1:])
        while stack and stack[-1][0] >= original:
            stack.pop()
        display = stack[-1][1] + 1 if stack else original
        stack.append((original, display))
        if display == original:
            continue
        if not token.markup.startswith('#'):
            raise ValueError('setext heading cannot need rank compaction')
        line = token.map[0]
        indentation = len(lines[line]) - len(lines[line].lstrip(' '))
        start = starts[line] + indentation
        end = start + len(token.markup)
        if source[start:end] != '#' * original:
            raise ValueError('heading parser/source-map disagreement')
        edits.append((start, end, '#' * display))
    return edits
