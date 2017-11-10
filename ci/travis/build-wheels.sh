#!/bin/bash
set -e -x

for PYBIN in /opt/python/*/bin; do
    if [[ ${PYBIN} =~ 26 ]]; then
        # dimod doesn't support 2.6, skip for now
        continue
    fi
    "${PYBIN}/pip" install -r /io/python/requirements.txt
    "${PYBIN}/pip" wheel -e /io/python -w wheelhouse/
done

# Bundle external shared libraries into the wheels
for whl in wheelhouse/*.whl; do
    if ! [[ "$whl" =~ dwave_qbsolv ]]; then
        continue
    fi
    auditwheel repair "$whl" -w /io/wheelhouse/
done

# Install packages and test
for PYBIN in /opt/python/*/bin/; do
    if [[ ${PYBIN} =~ 26 ]]; then
        # dimod doesn't support 2.6, skip for now
        continue
    fi
    "${PYBIN}/pip" install dwave_qbsolv --no-index -f /io/wheelhouse/
    (cd /io/; "${PYBIN}/python" -m unittest tests)
done
