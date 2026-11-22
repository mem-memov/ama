## @file
# @brief Encode independent complete/cyclic/constant and malformed source-domain response models.
import copy
import json
from pathlib import Path
import runpy


def generate():
    """Use known fixture semantics, independently of the response decoder."""
    helpers = runpy.run_path(str(Path(__file__).resolve().parent.parent / 'snapshot/generate.py'))
    helpers['generate']()
    data, text = helpers['data'], helpers['text']
    output = Path('generated')
    request = json.loads((output / 'base.json').read_text())
    prefix = '/vendor/pkg/v1.0.0/src/'
    sources = []
    for row in request['sources']:
        native = row['identity'] == '//native'
        sources.append({'identity': row['identity'], 'source': row['source'],
                        'configurationCaptured': row['identity'] != prefix + 'draft',
                        'configuration': row['configuration'],
                        'native': {'captured': native, **(row['companions'] if native else {'body': None, 'header': None, 'abi': None})}})

    def edge(owner, target, operand, member=None):
        code = bytes.fromhex(owner['source']['content']['hex'])
        begin = code.index(operand)
        return {'target': target, 'implicit': False, 'member': member,
                'origin': {'path': owner['source']['path'], 'start': str(begin), 'end': str(begin + len(operand))}}

    definitions = [{'identity': '//native', 'kind': 'concrete', 'dependencies': []},
                   {'identity': prefix + 'helper', 'kind': 'parameterized', 'dependencies': []},
                   {'identity': prefix + 'main', 'kind': 'concrete', 'dependencies': [edge(sources[0], '//native', b'//native')]}]
    base = {'format': 'ama.source-domain.v1', 'inputsSealed': False, 'versions': request['versions'],
            'sources': sources, 'definitions': definitions}

    def write(name, model):
        (output / name).write_text(json.dumps(model, ensure_ascii=False), encoding='utf-8')

    write('response.json', base)
    captured = copy.deepcopy(base)
    for row in captured['sources']:
        row['configurationCaptured'] = True
    write('captured-response.json', captured)
    write('empty-response.json', {'format': 'ama.source-domain.v1', 'inputsSealed': False,
                                 'versions': [], 'sources': [], 'definitions': []})
    cycle = copy.deepcopy(base)
    cycle['sources'][0]['source']['content'] = data(b'@!\nnext&\n./helper\n')
    cycle['sources'][1]['source']['content'] = data(b'@!\nback&\n./main\n')
    cycle['definitions'][1]['kind'] = 'concrete'
    cycle['definitions'][1]['dependencies'] = [edge(cycle['sources'][1], prefix + 'main', b'./main')]
    cycle['definitions'][2]['dependencies'] = [edge(cycle['sources'][0], prefix + 'helper', b'./helper')]
    write('cycle-response.json', cycle)
    constant = copy.deepcopy(base)
    constant['sources'][0]['source']['content'] = data(b'@!\nitems[]\n ~chosen[]\n')
    constant['sources'][0]['configuration'] = text(b'not-published/main.ini', b'~chosen = //palette/values\n')
    for identity, code in [('//palette', b'@=\nvalues[]\n//element\n'), ('//element', b'@!\n')]:
        constant['sources'].append({'identity': identity, 'source': text(identity.encode(), code),
                                    'configurationCaptured': True, 'configuration': None,
                                    'native': {'captured': False, 'body': None, 'header': None, 'abi': None}})
    constant['definitions'][2]['dependencies'] = [edge(constant['sources'][0], '//palette', b'~chosen', 'values')]
    constant['definitions'] += [{'identity': '//element', 'kind': 'concrete', 'dependencies': []},
                                {'identity': '//palette', 'kind': 'constants',
                                 'dependencies': [edge(constant['sources'][-2], '//element', b'//element')]}]
    constant['definitions'].sort(key=lambda row: row['identity'])
    write('constant-response.json', constant)
    invalid = []

    def case(label, edit):
        model = copy.deepcopy(base)
        edit(model)
        name = 'bad-response-' + label + '.json'
        write(name, model)
        invalid.append(name)

    case('format', lambda d: d.update(format='unknown'))
    case('extra-root', lambda d: d.update(unexpected=True))
    case('sealed', lambda d: d.update(inputsSealed=True))
    case('sealed-type', lambda d: d.update(inputsSealed='false'))
    case('version-array', lambda d: d.update(versions={}))
    case('version-duplicate', lambda d: d['versions'].append(copy.deepcopy(d['versions'][0])))
    case('version-missing-config', lambda d: d['versions'][0].update(configuration=None))
    case('module-version', lambda d: d.update(versions=[]))
    case('source-identity', lambda d: d['sources'][0].update(identity='/invalid'))
    case('source-duplicate', lambda d: d['sources'][1].update(identity=d['sources'][0]['identity']))
    case('source-physical-duplicate', lambda d: d['sources'][1].update(source=copy.deepcopy(d['sources'][0]['source'])))
    case('source-missing-bytes', lambda d: d['sources'][0].update(source=None))
    case('configuration-state', lambda d: d['sources'][0].update(configurationCaptured=1))
    case('unobserved-configuration', lambda d: d['sources'][1].update(configurationCaptured=False))
    case('native-state', lambda d: d['sources'][3]['native'].update(captured='true'))
    case('unobserved-native', lambda d: d['sources'][3]['native'].update(captured=False))
    case('native-field', lambda d: d['sources'][3]['native'].update(extra=None))
    case('source-field', lambda d: d['sources'][0].update(extra=None))
    case('definitions-array', lambda d: d.update(definitions={}))
    case('definition-category', lambda d: d['definitions'][2].update(kind='unknown'))
    case('native-category', lambda d: d['definitions'][0].update(kind='parameterized'))
    case('definition-source', lambda d: d['definitions'][2].update(identity='//missing'))
    case('definition-duplicate', lambda d: d['definitions'].append(copy.deepcopy(d['definitions'][-1])))
    case('definition-order', lambda d: d['definitions'].reverse())
    case('active-unobserved-config', lambda d: d['sources'][0].update(configurationCaptured=False))
    case('unknown-target', lambda d: d['definitions'][2]['dependencies'][0].update(target='//missing'))
    case('draft-target', lambda d: d['definitions'][2]['dependencies'][0].update(target=prefix + 'draft'))
    case('implicit-type', lambda d: d['definitions'][2]['dependencies'][0].update(implicit='false'))
    case('member-name', lambda d: d['definitions'][2]['dependencies'][0].update(member='/invalid'))
    case('origin-source', lambda d: d['definitions'][2]['dependencies'][0]['origin'].update(path=data(b'other')))
    case('origin-negative', lambda d: d['definitions'][2]['dependencies'][0]['origin'].update(start='-1'))
    case('origin-end', lambda d: d['definitions'][2]['dependencies'][0]['origin'].update(end='99999'))
    case('origin-reversed', lambda d: d['definitions'][2]['dependencies'][0]['origin'].update(start='18', end='1'))
    case('edge-field', lambda d: d['definitions'][2]['dependencies'][0].update(extra=None))
    case('edge-array', lambda d: d['definitions'][2].update(dependencies={}))
    case('native-inactive', lambda d: d.update(definitions=[]))
    (output / 'bad-responses.txt').write_text('\n'.join(invalid) + '\n', encoding='utf-8')


if __name__ == '__main__':
    generate()
