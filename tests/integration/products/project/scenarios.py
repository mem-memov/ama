## @file
# @brief Check independent expected product bytes, effects and source locations across exact controlled input changes.
from pathlib import Path
import shutil
import sys

from fixture import Product


def repeat(repository):
    """Same declared inputs reproduce all generated source/metadata after reuse and complete output removal."""
    product = Product(repository, 'repeat')
    inputs = product.inputs()
    product.emit()
    first = product.products()
    assert {'project.c', 'project.h', 'launcher.c', 'compile_commands.json', '.ama-native-outputs'} <= set(first)
    product.execute(b'length=7\n')
    product.emit()
    assert product.products() == first
    shutil.rmtree(product.output)
    product.emit()
    assert product.products() == first
    product.execute(b'length=7\n')
    assert product.inputs() == inputs


def changes(repository):
    """Each source/configuration change has an independent observable result; regeneration never edits authored bytes."""
    product = Product(repository, 'changes')
    product.emit()
    product.execute(b'length=7\n')
    flags = ('-DPRODUCT_OFFSET=2',)
    edits = [
        (None, None, b'length=9\n', True),
        (product.leaf, product.body.replace('|7', '|11'), b'length=13\n', True),
        (product.header, product.header.read_text().replace('PRODUCT_BASE 0', 'PRODUCT_BASE 5'), b'length=18\n', False),
        (product.native, product.native.read_text().replace('length=%zu', 'observed=%zu'), b'observed=18\n', False)]
    for path, text, expected, changed_text in edits:
        before = product.products()
        if path is not None:
            product.write(path, text)
        inputs = product.inputs()
        product.emit(flags)
        # A source include can stay byte-identical while its native dependency
        # changes. Executed behavior, not wrapper churn, proves that rebuild.
        if changed_text:
            assert product.products() != before
        product.execute(expected)
        assert product.inputs() == inputs
    product.write(product.native, product.native.read_text().replace('AMA_NATIVE(initialize)', 'AMA_NATIVE(initialize_second)'))
    abi = product.source / 'show.abi'
    product.write(abi, abi.read_text().replace('+ initialize', '+ initialize_second'))
    inputs = product.inputs()
    product.emit(flags)
    product.execute(b'observed=18\n')
    assert product.inputs() == inputs


def boundaries(repository):
    """Rejected original registries preserve old products; accepted source edits update real original-file traces."""
    product = Product(repository, 'boundaries')
    product.emit()
    first = product.products()
    duplicate = product.home / 'другой корень'
    duplicate.mkdir()
    product.write(duplicate / 'show', '@!\n')
    refused = product.emit(extra=('--root', str(duplicate)), accepted=False)
    assert refused.stderr
    assert product.products() == first
    product.execute(b'length=7\n')
    product.write(product.header, product.header.read_text().replace('PRODUCT_FAIL 0', 'PRODUCT_FAIL 1'))
    product.emit()
    diagnostic = product.execute(b'', accepted=False).stderr
    assert (str(product.leaf) + ':7:1').encode() in diagnostic, diagnostic
    product.write(product.leaf, product.body.replace('show !\n', '\n\n\nshow !\n'))
    inputs = product.inputs()
    product.emit()
    diagnostic = product.execute(b'', accepted=False).stderr
    assert (str(product.leaf) + ':10:1').encode() in diagnostic, diagnostic
    assert (str(product.leaf) + ':7:1').encode() not in diagnostic
    assert product.inputs() == inputs


{'repeat': repeat, 'changes': changes, 'boundaries': boundaries}[sys.argv[2]](Path(sys.argv[1]))
