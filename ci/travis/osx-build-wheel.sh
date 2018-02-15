#!/bin/bash
set -e -x

python --version
pip install delocate
pip install -r ./python/requirements.txt
pip install cython==0.27
pip wheel -e ./ -w raw-wheelhouse/

# Bundle external shared libraries into the wheels
for whl in raw-wheelhouse/*.whl; do
    if ! [[ "$whl" =~ dwave_qbsolv ]]; then
        continue
    fi
    delocate-wheel "$whl" -w wheelhouse/
done

# Install packages and test
pip install dwave_qbsolv --no-index -f wheelhouse/
python -m unittest tests
