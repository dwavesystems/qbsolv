"""
python setup.py build_ext --inplace
"""

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

setup(name='dwave_qbsolv',
      version='0.1',
      packages=['dwave_qbsolv'],
      requires=['cython',
                'dimod'],
      ext_modules=cythonize([Extension("qbsolv",
                                       ["dwave_qbsolv/qbsolv.pyx",
                                        '../src/main.c',
                                        '../src/solver.c',
                                        '../src/debugs.c',
                                        '../src/dwsolv.c',
                                        '../src/readqubo.c',
                                        '../src/util.c'],
                                       include_dirs=['.', '../src'],
                                       extra_compile_args=['-Ofast']
                                       # extra_compile_args=['-w', '-O0', '-g']
                                       )]))
