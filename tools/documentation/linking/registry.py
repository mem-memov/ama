## @file
# @brief Resolve canonical rule identities and produce collision-checked anchors.
#
# The requirement index supplies identity, not a filename/anchor naming guess.
# Resolution fails for stale, duplicate, out-of-tree or mismatched destinations.
import hashlib
import re
from pathlib import Path
from urllib.parse import unquote, urlsplit

from .markdown import links


def documentation_pages(root):
    """Enumerate maintained narrative Markdown separately from canonical rules.

    Keep every complete path and omit generated documentation/build trees. This
    finite explicit responsibility set includes repository guides and evidence,
    never promotes a narrative requirement to canonical specification authority.
    """
    root = Path(root)
    paths = set(root.glob('*.md'))
    for directory in ('docs', 'stdlib', 'third_party'):
        paths.update((root / directory).rglob('*.md'))
    pages = {path.relative_to(root).as_posix() for path in paths if path.is_file()
             and not {'technical', 'generated', 'build', '__pycache__'}.intersection(path.relative_to(root).parts)}
    pages.update(('docs/documentation.md', 'docs/implementation/library-tests-and-build.md'))
    return tuple(sorted(pages))


class Registry:
    """Own an immutable canonical-rule map for one repository root."""
    def __init__(self, root, digest=None):
        """Build and validate one exact canonical requirement map and narrative page set; reject duplicate IDs/anchors or unequal-key digest collisions before exposing links."""
        self.root = Path(root).resolve()
        self.pages = documentation_pages(self.root)
        self.rules = {}
        self.destinations = {}
        self.identities = {}
        self.digest = digest or (lambda value: hashlib.sha256(value).hexdigest())
        for _, _, label, href in links((self.root / 'spec/requirements.md').read_text()):
            if not re.fullmatch(r'[A-Z]+-\d+', label):
                continue
            destination = self.resolve('spec/requirements.md', href)
            if label in self.rules or destination in self.destinations:
                raise ValueError('duplicate canonical requirement ' + label)
            path, fragment = destination
            content = (self.root / path).read_text()
            anchor = '<a id="' + fragment + '"></a>'
            if not fragment or content.count(anchor) != 1:
                raise ValueError('missing/duplicate canonical anchor ' + label)
            suffix = content.split(anchor, 1)[1]
            if content.count('**' + label + '.**') != 1 or not suffix.lstrip().startswith('**' + label + '.**'):
                raise ValueError('requirement/definition mismatch ' + label)
            self.rules[label] = destination
            self.destinations[destination] = label
            self.identity('rule', path, label)
        if not self.rules:
            raise ValueError('empty requirement registry')

    def identity(self, kind, path, name=''):
        """Frame complete keys unambiguously and reject digest collisions."""
        key = (kind, str(path), name)
        encoded = b''.join(len(part.encode()).to_bytes(8, 'big') + part.encode() for part in key)
        anchor = 'ama_' + kind + '_' + self.digest(encoded)
        previous = self.identities.setdefault(anchor, key)
        if previous != key:
            raise ValueError('documentation identity collision')
        return anchor

    def resolve(self, source, href):
        """Resolve a local URL exactly, rejecting traversal and external URLs."""
        parsed = urlsplit(href)
        if parsed.scheme or parsed.netloc or parsed.query:
            raise ValueError('canonical link must be a local file/anchor: ' + href)
        path = (self.root / source).parent / unquote(parsed.path) if parsed.path else self.root / source
        path = path.resolve()
        if not path.is_relative_to(self.root / 'spec') or not path.is_file():
            raise ValueError('missing/outside-spec destination: ' + href)
        return path.relative_to(self.root).as_posix(), unquote(parsed.fragment)

    def reference(self, source, label, href, strict):
        """Return the exact canonical target or None for a non-rule URL."""
        parsed = urlsplit(href)
        if parsed.scheme or parsed.netloc:
            if strict and re.fullmatch(r'[A-Z]+-\d+', label):
                raise ValueError('rule reference must target canonical local spec')
            return None
        path = ((self.root / source).parent / unquote(parsed.path)).resolve() if parsed.path else self.root / source
        is_rule_label = re.fullmatch(r'[A-Z]+-\d+', label) is not None
        if not path.is_relative_to(self.root / 'spec'):
            if strict and is_rule_label:
                raise ValueError('rule reference outside spec: ' + href)
            if path.is_relative_to(self.root):
                relative = path.relative_to(self.root).as_posix()
                if relative in self.pages and not parsed.fragment:
                    if parsed.query or not path.is_file():
                        raise ValueError('missing/invalid documentation page: ' + href)
                    return self.identity('page', relative)
            return None
        target = self.resolve(source, href)
        rule = self.destinations.get(target)
        if strict and (not rule or label != rule):
            raise ValueError('mismatched rule reference: ' + label + ' -> ' + href)
        if is_rule_label and (not rule or label != rule):
            raise ValueError('stale requirement reference: ' + label)
        if rule:
            return self.identity('rule', target[0], rule)
        # Ordinary section/page links retain Doxygen's GitHub heading mapping.
        if target[1]:
            return None
        return self.identity('page', target[0])
