# QBSolv python bindings

## Install Instructions

Compatible with Python 2 and 3

```
pip install -r python/requirements.txt
pip install cython==0.27
python setup.py install
```

## Example Usage

```python
>>> from dwave_qbsolv import QBSolv
>>> h = {0: -1, 1: 1, 2: -1}
>>> J = {(0, 1): -1, (1, 2): -1}
>>> response = QBSolv().sample_ising(h, J)
>>> list(response.samples())
'[{0: 1, 1: 1, 2: 1}]'
>>> list(response.data_vectors['energy'])
'[-3.0]'
```

```python
>>> from dwave_qbsolv import QBSolv
>>> Q = {(0, 0): 1, (1, 1): 1, (0, 1): 1}
>>> response = QBSolv().sample_qubo(Q)
>>> list(response.samples())
'[{0: 0, 1: 0}]'
>>> list(response.data_vectors['energy'])
'[0.0]'
```
