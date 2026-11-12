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
        """Parse one complete generated HTML page and own its canonical ID/link sets for exact destination checking."""
        super().__init__(convert_charrefs=True)
        self.ids, self.hrefs = set(), []
        self.feed(text)

    def handle_starttag(self, tag, attributes):
        """Collect only real anchor IDs and hrefs; duplicate canonical IDs fail immediately rather than choosing one rendered location."""
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


def interfaces(directory):
    """Require real Doxygen descriptions for every extracted function/type.

    EXTRACT_ALL keeps missing declarations visible. Inspect its XML rather than
    suppressing diagnostics or guessing nontriviality from source length. Fields,
    macros and enum values use their enclosing type contract; every function,
    typedef, enum and class/struct/union must have its own rendered description.
    """
    directory = Path(directory) / 'xml'
    if not (directory / 'index.xml').is_file():
        raise ValueError('missing Doxygen interface XML index')
    try:
        index = ET.parse(directory / 'index.xml').getroot()
    except ET.ParseError as error:
        raise ValueError('malformed Doxygen interface XML index') from error
    indexed_members, indexed_types = set(), set()
    for compound in index.findall('compound'):
        identity = compound.get('refid', '')
        if not identity or Path(identity).name != identity or not (directory / (identity + '.xml')).is_file():
            raise ValueError('missing indexed Doxygen compound XML: ' + identity)
        if compound.get('kind') in {'class', 'struct', 'union'}:
            indexed_types.add(identity)
        indexed_members.update(member.get('refid', '') for member in compound.findall('member')
                               if member.get('kind') in {'function', 'typedef', 'enum'})
    found_members, found_types = set(), set()
    count, missing = 0, []
    for path in sorted(directory.glob('*.xml')):
        try:
            root = ET.parse(path).getroot()
        except ET.ParseError as error:
            raise ValueError('malformed Doxygen interface XML: ' + str(path)) from error
        declarations = [node for node in root.findall('./compounddef')
                        if node.get('kind') in {'class', 'struct', 'union'}]
        declarations += [node for node in root.findall('.//memberdef')
                         if node.get('kind') in {'function', 'typedef', 'enum'}]
        for node in declarations:
            if node.tag == 'compounddef':
                found_types.add(node.get('id', ''))
            else:
                found_members.add(node.get('id', ''))
            count += 1
            documented = any(''.join(description.itertext()).strip()
                             for name in ('briefdescription', 'detaileddescription')
                             if (description := node.find(name)) is not None)
            if not documented:
                location = node.find('location')
                source = location.get('file', str(path)) if location is not None else str(path)
                name = node.findtext('name') or node.findtext('compoundname') or '<unnamed>'
                missing.append(source + ': ' + node.get('kind', '') + ' ' + name)
    if not indexed_members.issubset(found_members) or not indexed_types.issubset(found_types):
        raise ValueError('indexed Doxygen interface missing from compound XML')
    if missing:
        raise ValueError('undocumented extracted interfaces: ' + '; '.join(missing))
    return count
