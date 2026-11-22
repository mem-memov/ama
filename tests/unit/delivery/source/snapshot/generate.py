## @file
# @brief Generate independent valid and intentionally malformed captured-input models.
import copy
import json
from pathlib import Path


def data(value):
    """Encode exact bytes with the independent Python UTF-8 decoder."""
    try:
        text = value.decode('utf-8')
    except UnicodeDecodeError:
        text = None
    return {'utf8': text, 'hex': value.hex()}


def text(path, content):
    """Describe a captured source without creating the named physical path."""
    return {'path': data(path), 'content': data(content)}


def generate():
    """Create a fixed version/source/native/draft model and one-error mutations."""
    output = Path('generated')
    output.mkdir(exist_ok=True)
    configuration = text(b'not-published/version/package.ini', b'; saved\n')
    prefix = '/vendor/pkg/v1.0.0/src/'
    identities = [prefix + 'main', prefix + 'helper', prefix + 'draft', '//native']
    contents = [b'@!\nlink&\n//native\n', b'@<\nvalue\n.T\n@!\n', b'unfinished !\n', b'@C\n']
    paths = [b'not-published/\xffmain', b'not-published/helper', b'not-published/draft', b'not-published/native']
    rows = []
    for index, identity in enumerate(identities):
        rows.append({'identity': identity, 'source': text(paths[index], contents[index]),
                     'device': '7', 'inode': str(index + 1), 'configuration': None,
                     'companions': {'body': None, 'header': None, 'abi': None}})
    rows[1]['configuration'] = copy.deepcopy(configuration)
    rows[2]['companions']['abi'] = text(b'not-published/draft.abi', b'\xff\x00')
    rows[3]['companions'] = {'body': data(b'not-published/native.c'),
                             'header': data(b'not-published/native.h'),
                             'abi': text(b'not-published/native.abi', b'+ initialize\n- destroy\n')}
    base = {'format': 'ama.source-snapshot.v1',
            'versions': [{'identity': '/vendor/pkg/v1.0.0', 'directory': data(b'not-published/version'),
                          'configuration': configuration}], 'sources': rows}

    def write(name, document):
        (output / name).write_text(json.dumps(document, ensure_ascii=False), encoding='utf-8')

    write('base.json', base)
    write('empty.json', {'format': 'ama.source-snapshot.v1', 'versions': [], 'sources': []})
    bad = copy.deepcopy(base)
    bad['sources'][0]['source']['content'] = data(b'@!\nmissing !\n')
    write('bad-source.json', bad)
    bad = copy.deepcopy(base)
    bad['sources'][0]['source']['content'] = data(b'\xff')
    write('bad-source-utf8.json', bad)
    bad = copy.deepcopy(base)
    bad['sources'][3]['companions']['abi']['content'] = data(b'\xff')
    write('bad-native-abi.json', bad)
    bad = copy.deepcopy(base)
    bad['sources'][1]['device'] = bad['sources'][0]['device']
    bad['sources'][1]['inode'] = bad['sources'][0]['inode']
    write('aliased-metadata.json', bad)
    invalid = []

    def case(label, edit):
        document = copy.deepcopy(base)
        edit(document)
        name = 'invalid-' + label + '.json'
        write(name, document)
        invalid.append(name)

    case('format', lambda d: d.update(format='unknown'))
    case('missing-format', lambda d: d.pop('format'))
    case('extra-root-field', lambda d: d.update(outputs=[]))
    case('version-array', lambda d: d.update(versions={}))
    case('source-array', lambda d: d.update(sources={}))
    case('version-identity', lambda d: d['versions'][0].update(identity='//bad'))
    case('version-duplicate', lambda d: d['versions'].append(copy.deepcopy(d['versions'][0])))
    case('version-configuration', lambda d: d['versions'][0].update(configuration=None))
    case('version-directory', lambda d: d['versions'][0].update(directory=None))
    case('module-version', lambda d: d.update(versions=[]))
    case('module-identity', lambda d: d['sources'][0].update(identity='/not/a/module'))
    case('module-source', lambda d: d['sources'][0].update(source=None))
    case('empty-path', lambda d: d['sources'][0]['source'].update(path=data(b'')))
    case('null-path', lambda d: d['sources'][0]['source'].update(path=None))
    case('nul-path', lambda d: d['sources'][0]['source'].update(path=data(b'a\x00b')))
    case('module-duplicate', lambda d: d['sources'][1].update(identity=d['sources'][0]['identity']))
    case('physical-duplicate', lambda d: d['sources'][1].update(source=copy.deepcopy(d['sources'][0]['source'])))
    case('conflicting-bytes', lambda d: d['sources'][1]['configuration'].update(content=data(b'; changed\n')))
    case('device-type', lambda d: d['sources'][0].update(device=7))
    case('device-overflow', lambda d: d['sources'][0].update(device=str(2**256)))
    case('inode-negative', lambda d: d['sources'][0].update(inode='-1'))
    case('body-hex', lambda d: d['sources'][3]['companions']['body'].update(hex='f'))
    case('companions-missing', lambda d: d['sources'][0].pop('companions'))
    case('configuration-type', lambda d: d['sources'][0].update(configuration='invalid'))
    case('unknown-source-field', lambda d: d['sources'][0].update(unexpected=True))
    case('unknown-companion-field', lambda d: d['sources'][0]['companions'].update(unexpected=True))
    case('byte-disagreement', lambda d: d['sources'][0]['source']['content'].update(utf8='different'))
    (output / 'invalid.txt').write_text('\n'.join(invalid) + '\n', encoding='utf-8')


if __name__ == '__main__':
    generate()
