"""Build the qbsolv package."""

from setuptools import setup
from setuptools.extension import Extension
from setuptools.command.build_ext import build_ext
from Cython.Build import cythonize

extra_compile_args = {
    'msvc': [],
    'unix': ['-std=gnu99', '-Ofast', '-Wall', '-Wextra', '-flto'],
    # 'unix': ['-std=c++1y','-w','-O0', '-g', '-fipa-pure-const'],
}

extra_link_args = {
    'msvc': [],
    'unix': [],
}


class build_ext_compiler_check(build_ext):
    def build_extensions(self):
        compiler = self.compiler.compiler_type

        compile_args = extra_compile_args[compiler]
        for ext in self.extensions:
            ext.extra_compile_args = compile_args

        link_args = extra_compile_args[compiler]
        for ext in self.extensions:
            ext.extra_compile_args = link_args

        build_ext.build_extensions(self)


extensions = [Extension('dwave_qbsolv.qbsolv_binding',
                        ['dwave_qbsolv/qbsolv_binding.pyx',
                         './globals.c',
                         '../src/solver.c',
                         '../src/debugs.c',
                         '../src/dwsolv.c',
                         '../src/util.c'],
                        include_dirs=['.', '../src', '../include'],
                        )]

packages = ['dwave_qbsolv']

setup(
    name='dwave_qbsolv',
    version='0.1.2',
    packages=packages,
    install_requires=['dimod>=0.3.1', 'cython'],
    ext_modules=cythonize(extensions),
    cmdclass={'build_ext': build_ext_compiler_check}
)
