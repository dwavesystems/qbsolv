"""
python setup.py build_ext --inplace
"""

from setuptools import setup, find_packages
from setuptools.extension import Extension
from Cython.Build import cythonize


extensions = [Extension("dwave_qbsolv.qbsolv",
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
                        )]

setup(name='dwave_qbsolv',
      version='0.1.2',
      packages=find_packages(),
      install_requires=['dimod>=0.1.1',
                        'cython'],
      ext_modules=cythonize(extensions))
