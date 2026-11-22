## @file
# @brief Independently decode source-domain observations and check known compiler fixtures.
import json
from pathlib import Path
import sys


def raw(value):
    """Verify the wire's redundant UTF-8/hex fields and return exact original bytes."""
    assert set(value) == {'utf8', 'hex'}
    data = bytes.fromhex(value['hex'])
    assert data.hex() == value['hex']
    try:
        text = data.decode('utf-8')
    except UnicodeDecodeError:
        text = None
    assert value['utf8'] == text
    return data


def verify():
    """Assert concrete known source/configuration/edge facts, independent of producer syntax."""
    document = json.loads(Path('generated/domain.json').read_text())
    assert set(document) == {'format', 'inputsSealed', 'versions', 'sources', 'definitions'}
    assert document['format'] == 'ama.source-domain.v1'
    assert document['inputsSealed'] is False
    if sys.argv[1] == 'empty':
        assert document['versions'] == document['sources'] == document['definitions'] == []
        return
    assert sys.argv[1] == 'populated'
    version, = document['versions']
    assert version['identity'] == '/vendor/pkg/v1.0.0'
    assert raw(version['directory']) == b'physical/version'
    assert raw(version['configuration']['path']) == b'captured-package.ini'
    assert raw(version['configuration']['content']) == b'# metadata\n'
    sources = {row['identity']: row for row in document['sources']}
    assert set(sources) == {'//main', '//palette', '//element', '//helper', '//marker', '//native', '//draft'}
    assert len(document['sources']) == len(sources)
    main = sources['//main']
    assert raw(main['source']['path']) == b'physical/\xffsource'
    assert raw(main['source']['content']) == b'@!\nitems[]\n ~chosen[]\n'
    assert main['configurationCaptured'] is True
    assert raw(main['configuration']['content']) == b'~chosen = //palette/values\n'
    assert sources['//draft']['configurationCaptured'] is True
    assert sources['//draft']['configuration'] is None
    assert raw(sources['//draft']['source']['content']) == b'unfinished !\n'
    native = sources['//native']['native']
    assert native['captured'] is True
    assert raw(native['body']) == b'missing-body.c'
    assert raw(native['header']) == b'missing-header.h'
    assert raw(native['abi']['content']) == b'+ initialize\n- destroy\n'
    assert sources['//element']['native'] == {'captured': False, 'body': None, 'header': None, 'abi': None}
    definitions = {row['identity']: row for row in document['definitions']}
    assert set(definitions) == set(sources) - {'//draft'}
    assert [row['identity'] for row in document['definitions']] == sorted(definitions)
    assert definitions['//helper']['kind'] == 'parameterized'
    assert definitions['//marker']['kind'] == 'static'
    assert definitions['//palette']['kind'] == 'constants'
    assert definitions['//native']['kind'] == 'concrete'
    edge, = definitions['//main']['dependencies']
    assert edge['target'] == '//palette' and edge['member'] == 'values'
    assert edge['implicit'] is False
    start, end = edge['origin']['start'], edge['origin']['end']
    assert isinstance(start, str) and isinstance(end, str)
    assert str(int(start)) == start and str(int(end)) == end
    assert raw(edge['origin']['path']) == b'physical/\xffsource'
    assert raw(main['source']['content'])[int(start):int(end)] == b'~chosen'
    target, = definitions['//palette']['dependencies']
    assert target['target'] == '//element' and target['member'] is None


if __name__ == '__main__':
    verify()
