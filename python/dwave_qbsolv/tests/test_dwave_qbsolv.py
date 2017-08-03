import unittest

import dwave_qbsolv as qbs


class TestWrapper(unittest.TestCase):
    def test_dimod(self):
        n_variables = 50

        Q = {(v, v): 1 for v in range(n_variables)}

        qbs.QBSolv().sample_qubo(Q)
