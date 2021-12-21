"""Build the qbsolv package."""

from setuptools import setup
from setuptools.extension import Extension
from setuptools.command.build_ext import build_ext
import os

cwd = os.path.abspath(os.path.dirname(__file__))
if not os.path.exists(os.path.join(cwd, 'PKG-INFO')):
    try:
        from Cython.Build import cythonize
        USE_CYTHON = True
    except ImportError:
        USE_CYTHON = False
else:
    USE_CYTHON = False


extra_compile_args = {
    'msvc': [],
    'unix': ['-std=c++11', '-Ofast', '-Wall', '-Wextra'],
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


ext = '.pyx' if USE_CYTHON else '.cpp'

extensions = [Extension('dwave_qbsolv.qbsolv_binding',
                        ['python/dwave_qbsolv/qbsolv_binding' + ext,
                         './python/globals.cc',
                         './src/solver.cc',
                         './src/dwsolv.cc',
                         './src/util.cc'],
                        include_dirs=['./python', './src', './include', './cmd']
                        )]

if USE_CYTHON:
    extensions = cythonize(extensions, language='c++')

packages = ['dwave_qbsolv']

classifiers = [
    'License :: OSI Approved :: Apache Software License',
    'Operating System :: OS Independent',
    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: 3.6',
    'Programming Language :: Python :: 3.7',
    'Programming Language :: Python :: 3.8',
    'Programming Language :: Python :: 3.9',
    'Programming Language :: Python :: 3.10',
    ]

python_requires = '>=3.6'

setup(
    name='dwave-qbsolv',
    version='0.3.4',
    packages=packages,
    package_dir={'dwave_qbsolv': 'python/dwave_qbsolv'},
    install_requires=['dimod>=0.8.1,<0.11.0'],
    ext_modules=extensions,
    cmdclass={'build_ext': build_ext_compiler_check},
    long_description=open('README.rst').read(),
    classifiers=classifiers,
    license='Apache 2.0',
    python_requires=python_requires
)
