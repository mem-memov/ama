## @file
# @brief Encode known direct concrete/open/grounded/static test cases and a changed-graph process response.
import json
from pathlib import Path
import runpy


def generate():
    """Use independent known module bodies and original protocol models."""
    home = Path(__file__).resolve().parents[4] / 'delivery/source/response/generate.py'
    helper = runpy.run_path(str(home))
    helper['generate']()
    output = Path('generated')
    request = json.loads((output / 'base.json').read_text())
    fixture = runpy.run_path(str(home.parent.parent / 'snapshot/generate.py'))
    data, text = fixture['data'], fixture['text']
    old, new = '/vendor/pkg/v1.0.0/src/', '/vendor/pkg/v1.0.0/tests/sort/'
    for row in request['sources']:
        if row['identity'].startswith(old):
            row['identity'] = new + ('check' if row['identity'].endswith('/main') else row['identity'][len(old):])
            physical = 'not-published/version/tests/sort/' + row['identity'][len(new):]
            row['source']['path'] = data(physical.encode())
            if row['configuration'] is not None:
                row['configuration'] = text((physical + '.ini').encode(), b'; helper\n')
            if row['companions']['abi'] is not None:
                row['companions']['abi']['path'] = data((physical + '.abi').encode())
    request['sources'][3]['companions']['abi'] = text(b'not-published/native.abi', b'+ initialize\n- destroy\n= clone\n')
    extra = [(new + 'marker', b'@...\n'),
             (new + 'grounded', b'@>\nresult\n.T\n //palette/value\n@!\n'),
             ('//palette', b'@=\nvalue\n//native\n')]
    for index, (identity, code) in enumerate(extra):
        physical = 'not-published/version/tests/sort/' + identity[len(new):] if identity.startswith(new) else 'not-published/palette'
        request['sources'].append({'identity': identity, 'source': text(physical.encode(), code),
                                   'device': '7', 'inode': str(index + 5), 'configuration': None,
                                   'companions': {'body': None, 'header': None, 'abi': None}})
    (output / 'request.json').write_text(json.dumps(request, ensure_ascii=False), encoding='utf-8')
    changed = json.loads((output / 'captured-response.json').read_text())
    changed['definitions'][1]['kind'] = 'concrete'
    (output / 'changed-graph.json').write_text(json.dumps(changed, ensure_ascii=False), encoding='utf-8')


if __name__ == '__main__':
    generate()
