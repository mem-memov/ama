## @file
# @brief Decode actual driver domain reports with an independent JSON implementation.
import json
from pathlib import Path
import sys


def verify():
    """Check known complete root content and original unused dependency provenance."""
    document = json.loads(Path('generated/domain.json').read_text())
    assert document['format'] == 'ama.source-domain.v1'
    assert document['inputsSealed'] is False
    version, = document['versions']
    if sys.argv[1] == 'snapshot':
        assert version['identity'] == '/vendor/pkg/v1.0.0'
        assert len(document['sources']) == 4 and len(document['definitions']) == 3
        definitions = {row['identity']: row for row in document['definitions']}
        assert definitions['/vendor/pkg/v1.0.0/src/helper']['kind'] == 'parameterized'
        edge, = definitions['/vendor/pkg/v1.0.0/src/main']['dependencies']
        assert edge['target'] == '//native'
        assert bytes.fromhex(edge['origin']['path']['hex']) == b'not-published/\xffmain'
        native = next(row for row in document['sources'] if row['identity'] == '//native')['native']
        assert native['captured'] is True
        assert bytes.fromhex(native['body']['hex']) == b'not-published/native.c'
        return
    assert version['identity'] == '/test/fixture/v1.0.0'
    assert version['configuration']['content'] == {'utf8': '', 'hex': ''}
    if sys.argv[1] == 'empty':
        assert document['sources'] == document['definitions'] == []
        return
    assert sys.argv[1] == 'populated'
    prefix = '/test/fixture/v1.0.0/src/'
    sources = {item['identity']: item for item in document['sources']}
    assert set(sources) == {prefix + name for name in ('main', 'unused', 'helper', 'marker', 'draft', 'native')}
    definitions = {item['identity']: item for item in document['definitions']}
    assert set(definitions) == set(sources) - {prefix + 'draft'}
    assert definitions[prefix + 'helper']['kind'] == 'parameterized'
    assert definitions[prefix + 'marker']['kind'] == 'static'
    edge, = definitions[prefix + 'unused']['dependencies']
    assert edge['target'] == prefix + 'main' and edge['member'] is None and edge['implicit'] is False
    original = bytes.fromhex(sources[prefix + 'unused']['source']['content']['hex'])
    assert original[int(edge['origin']['start']):int(edge['origin']['end'])] == b'./main'
    native = sources[prefix + 'native']['native']
    assert native['captured'] is True
    assert bytes.fromhex(native['body']['hex']).endswith(b'/src/native.c')
    assert bytes.fromhex(native['header']['hex']).endswith(b'/src/native.h')
    assert bytes.fromhex(native['abi']['content']['hex']) == b'+ initialize\n- destroy\n'


if __name__ == '__main__':
    verify()
