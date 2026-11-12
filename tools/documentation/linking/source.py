## @file
# @brief Extract C documentation comments without touching literals or code.
#
# A mapped translation-phase view handles C11 trigraphs and escaped newlines.
# Every reported documentation byte still points into the physical source file.
import re

_TRIGRAPHS = dict(zip("=/'()!<>-", "#\\^[]|{}~"))


def _logical(source):
    """Apply C11 trigraph replacement and escaped-newline removal while retaining an exact physical offset for every surviving logical character; do not mutate source."""
    chars, offsets = [], []
    index = 0
    while index < len(source):
        if source[index:index + 2] == '??' and source[index + 2:index + 3] in _TRIGRAPHS:
            chars.append(_TRIGRAPHS[source[index + 2]])
            offsets.append(index)
            index += 3
        else:
            chars.append(source[index])
            offsets.append(index)
            index += 1
    text = ''.join(chars)
    result, positions = [], []
    index = 0
    while index < len(text):
        if text[index:index + 2] == '\\\n':
            index += 2
        else:
            result.append(text[index])
            positions.append(offsets[index])
            index += 1
    return ''.join(result), positions


def comments(source):
    """Yield cleaned Doxygen comment text and exact physical-character offsets."""
    text, offsets = _logical(source)
    index = 0
    while index < len(text):
        if text[index] in '\"\'':
            quote = text[index]
            index += 1
            while index < len(text) and text[index] != quote:
                index += 2 if text[index] == '\\' else 1
            index += 1
            continue
        if text[index:index + 2] not in ('/*', '//'):
            index += 1
            continue
        begin = index
        block = text[index:index + 2] == '/*'
        if block:
            close = text.find('*/', index + 2)
            if close < 0:
                raise ValueError('unterminated C comment')
            end = close
            index = close + 2
        else:
            close = text.find('\n', index + 2)
            end = len(text) if close < 0 else close
            index = end
        documented = text[begin + 2:begin + 3] in ('*', '!') if block else text[begin + 2:begin + 3] in ('/', '!')
        if not documented:
            continue
        content_begin = begin + 3
        fragment = text[content_begin:end]
        physical = offsets[content_begin:end]
        if not block:
            while index < len(text) and text[index] == '\n':
                following = re.match(r'\n[ \t]*//[/!]', text[index:])
                if not following:
                    break
                following_begin = index + following.end()
                following_end = text.find('\n', following_begin)
                if following_end < 0:
                    following_end = len(text)
                fragment += '\n' + text[following_begin:following_end]
                physical += [offsets[index]] + offsets[following_begin:following_end]
                index = following_end
        # Decorative leading stars belong to the comment, not Markdown prose.
        for match in reversed(list(re.finditer(r'(?m)^([ \t]*\* ?)', fragment))):
            fragment = fragment[:match.start()] + fragment[match.end():]
            physical = physical[:match.start()] + physical[match.end():]
        yield fragment, physical


def mask_examples(text):
    """Hide Doxygen code/verbatim regions without shifting any offsets."""
    result = list(text)
    active = None
    for match in re.finditer(r'(?m)^.*(?:\n|$)', text):
        line = match.group()
        command = re.match(r'\s*[@\\](code|verbatim|endcode|endverbatim)\b', line)
        if command and command.group(1) in ('code', 'verbatim'):
            if active:
                raise ValueError('nested Doxygen example block')
            active = command.group(1)
        if active:
            result[match.start():match.end()] = ['\n' if char == '\n' else ' ' for char in line]
        if command and command.group(1) in ('endcode', 'endverbatim'):
            if command.group(1) != 'end' + str(active):
                raise ValueError('unmatched Doxygen example end')
            active = None
    if active:
        raise ValueError('unterminated Doxygen example')
    return ''.join(result)
