## @file
# @brief Own exact Unicode source fixtures and real product processes without inspecting binary artifacts.
from pathlib import Path
import shutil
import subprocess


class Product:
    """Keep source, standard-library and generated-product lifetimes in one independently owned case tree."""

    def __init__(self, repository, name):
        """Construct one private complete source closure; filesystem failures abort before a product command runs."""
        self.repository = repository
        self.home = Path('generated') / name
        self.home.mkdir()
        self.source = self.home / 'проект Ама'
        self.standard = self.home / 'стандартная библиотека'
        self.output = self.home / 'выходные файлы'
        self.source.mkdir()
        self.standard.mkdir()
        (self.source / 'ветка+').mkdir()
        self.leaf = self.source / 'ветка+/лист'
        self.write(self.source / 'старт', '@!\nworker\n.ветка\nworker !\n')
        self.write(self.source / 'ветка', '@!\nworker\n.ветка.лист\nworker !\n')
        self.body = '@!\nvalue\n..integer\n |7\nshow\n.show\nshow !\n < value\n'
        self.write(self.leaf, self.body)
        observer = repository / 'tests/integration/standard-integer/project'
        for suffix in ('', '.h', '.c', '.abi'):
            shutil.copyfile(observer / ('show' + suffix), self.source / ('show' + suffix))
        self.header = self.source / 'detail/value.h'
        self.header.parent.mkdir()
        self.write(self.header, '#define PRODUCT_BASE 0\n#ifndef PRODUCT_OFFSET\n#define PRODUCT_OFFSET 0\n#endif\n#define PRODUCT_FAIL 0\n')
        self.native = self.source / 'show.c'
        text = self.native.read_text()
        text = '#include "detail/value.h"\n' + text
        text = text.replace('size_t number = 0;', 'if (PRODUCT_FAIL) { *status=ama_status_make(AMA_NATIVE_FAILURE,NULL,0); return; }\n    size_t number = 0;')
        text = text.replace('printf("length=%zu\\n", number)', 'printf("length=%zu\\n", number + PRODUCT_BASE + PRODUCT_OFFSET)')
        self.write(self.native, text)
        standard = repository / 'stdlib/modules'
        for suffix in ('', '.h', '.c', '.abi'):
            shutil.copyfile(standard / ('integer' + suffix), self.standard / ('integer' + suffix))
        shutil.copytree(standard / 'integer+/native', self.standard / 'integer+/native')

    @staticmethod
    def write(path, content):
        """Write only a case-owned authored input, retaining explicit UTF-8 bytes and physical newlines."""
        path.write_bytes(content.encode('utf-8'))

    def inputs(self):
        """Snapshot all authored bytes, including native headers and ABI companions, independently of build output."""
        return {str(p.relative_to(self.home)): p.read_bytes()
                for root in (self.source, self.standard) for p in sorted(root.rglob('*')) if p.is_file()}

    def products(self):
        """Compare declared generated source/tooling/ownership text as bytes; never read an executable or object."""
        return {str(p.relative_to(self.output)): p.read_bytes() for p in sorted(self.output.rglob('*'))
                if p.is_file() and (p.suffix in ('.c', '.h', '.json') or p.name == '.ama-native-outputs')}

    def emit(self, flags=(), extra=(), accepted=True):
        """Invoke the actual compiler with exact argv and require the expected publication outcome."""
        arguments = [str(self.repository / 'ama'), '--emit', str(self.source), '.старт',
                     '--standard-root', str(self.standard), '--out', str(self.output),
                     '--sdk', str(self.repository / 'src'), '--compile']
        for flag in flags:
            arguments.extend(('--c-flag', flag))
        result = subprocess.run(arguments + list(extra), capture_output=True)
        assert (result.returncode == 0) == accepted, result.stderr.decode(errors='replace')
        if accepted:
            assert not result.stderr, result.stderr
        return result

    def execute(self, expected, accepted=True):
        """Observe actual verified program output/status with no generated-source interpretation."""
        result = subprocess.run([str(self.output.resolve() / 'program')], capture_output=True)
        assert (result.returncode == 0) == accepted, result.stderr.decode(errors='replace')
        assert result.stdout == expected
        if accepted:
            assert not result.stderr, result.stderr
        return result
