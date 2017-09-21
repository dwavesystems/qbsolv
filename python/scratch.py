from dwave_qbsolv.qbsolv import run_qbsolv

Q = {(0, 0): -1, (1, 1): -1, (0, 1): 1}

print(run_qbsolv(Q, verbosity=3))
