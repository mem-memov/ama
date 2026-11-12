## @file
# @brief Check generated Doxygen destinations instead of trusting successful exit.
#
# Parse HTML/tag XML, require unique canonical IDs, and resolve local file/fragment
# references exactly. Doxygen owns rendering and file/API reference generation.
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit
import xml.etree.ElementTree as ET


class Page(HTMLParser):
    """Collect HTML IDs/links; duplicate canonical IDs are hard errors."""
    def __init__(self, text):
        super().__init__(convert_charrefs=True)
        self.ids, self.hrefs = set(), []
        self.feed(text)

    def handle_starttag(self, tag, attributes):
        attrs = dict(attributes)
        identity = attrs.get('id') or (attrs.get('name') if tag == 'a' else None)
        if identity:
            if identity in self.ids and identity.startswith(('ama_rule_', 'ama_page_')):
                raise ValueError('duplicate rendered canonical anchor ' + identity)
            self.ids.add(identity)
        if tag == 'a' and 'href' in attrs:
            self.hrefs.append(attrs['href'])


def validate(directory, expected, rule_pages=None):
    """Check each canonical reference reaches its exact canonical definition."""
    directory = Path(directory).resolve()
    pages = {path.name: Page(path.read_text()) for path in directory.glob('*.html')}
    definitions = {}
    for filename, page in pages.items():
        for anchor in page.ids:
            if anchor.startswith('ama_rule_') or anchor.startswith('ama_page_'):
                if anchor in definitions:
                    raise ValueError('duplicate cross-page canonical anchor ' + anchor)
                definitions[anchor] = filename
    for compound in ET.parse(directory / 'ama.tag').getroot().findall('compound'):
        if compound.get('kind') == 'page':
            name = compound.findtext('name', '')
            if name.startswith('ama_page_'):
                filename = compound.findtext('filename', '')
                if not filename.endswith('.html'):
                    filename += '.html'
                if filename not in pages:
                    raise ValueError('missing rendered page ' + filename)
                definitions[name] = filename
    for rule, page in (rule_pages or {}).items():
        if rule not in definitions or definitions[rule] != definitions.get(page):
            raise ValueError('canonical definition rendered on wrong chapter: ' + rule)
    referenced = set()
    by_page = {}
    for filename, page in pages.items():
        by_page[filename] = set()
        for href in page.hrefs:
            url = urlsplit(href)
            if url.scheme or url.netloc:
                continue
            if not url.fragment and Path(url.path).name.startswith('ama_page_'):
                for anchor, destination in definitions.items():
                    if destination == Path(url.path).name and anchor.startswith('ama_page_'):
                        referenced.add(anchor)
                        by_page[filename].add(anchor)
                continue
            if not url.fragment.startswith(('ama_rule_', 'ama_page_')):
                continue
            destination = (directory / filename).parent / unquote(url.path) if url.path else directory / filename
            if destination.parent.resolve() != directory or destination.name not in pages:
                raise ValueError('missing rendered destination ' + href)
            if definitions.get(url.fragment) != destination.name:
                raise ValueError('wrong rendered rule destination ' + href)
            referenced.add(url.fragment)
            by_page[filename].add(url.fragment)
    if isinstance(expected, dict):
        files = file_pages(directory)
        for source, anchors in expected.items():
            filename = definitions.get(source, files.get(source))
            if filename not in by_page or not set(anchors).issubset(by_page[filename]):
                raise ValueError('missing canonical link on originating page: ' + source)
        expected = {anchor for anchors in expected.values() for anchor in anchors}
    missing = set(expected) - referenced
    if missing:
        raise ValueError('unresolved canonical references: ' + ', '.join(sorted(missing)))
    if not (directory / 'index.html').is_file():
        raise ValueError('missing Doxygen entry page')
    return {'pages': len(pages), 'canonical_destinations': len(definitions), 'referenced_destinations': len(referenced)}


def file_pages(directory):
    """Return actual Doxygen file compounds from its generated tag file."""
    result = {}
    for compound in ET.parse(Path(directory) / 'ama.tag').getroot().findall('compound'):
        if compound.get('kind') == 'file':
            name = compound.findtext('path', '') + compound.findtext('name', '')
            result[name] = compound.findtext('filename')
    return result
