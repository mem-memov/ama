## @file
# @brief Independently compare the serialized input model with its original Python-generated authority.
import json
from pathlib import Path


def verify():
    """Require every original field/byte/integer without accepting extra or omitted authority."""
    original = json.loads(Path('generated/base.json').read_text())
    written = json.loads(Path('generated/roundtrip.json').read_text())
    assert written == original
    assert written['sources'][2]['companions']['abi']['content'] == {'utf8': None, 'hex': 'ff00'}
    assert written['sources'][0]['source']['path']['utf8'] is None


if __name__ == '__main__':
    verify()
