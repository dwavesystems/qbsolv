import unittest

import itertools

import dwave_qbsolv as qbs

alpha = dict(enumerate('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'))


class TestWrapper(unittest.TestCase):
    def test_dimod_basic_qubo(self):
        n_variables = 50

        # all 0s
        Q = {(v, v): 1 for v in range(n_variables)}
        response = qbs.QBSolv().sample_qubo(Q, seed=10)
        sample = next(iter(response))
        for v in sample:
            self.assertEqual(sample[v], 0)

        # all 1s
        Q = {(u, v): -1 for u, v in itertools.combinations(range(n_variables), 2)}
        response = qbs.QBSolv().sample_qubo(Q, seed=167)
        sample = next(iter(response))
        for v in sample:
            self.assertEqual(sample[v], 1)

    def test_dimod_basic_qubo_alpha(self):
        n_variables = 50

        # all 0s
        Q = {(alpha[v], alpha[v]): 1 for v in range(n_variables)}
        response = qbs.QBSolv().sample_qubo(Q, seed=5125)
        sample = next(iter(response))
        for v in sample:
            self.assertEqual(sample[v], 0)

        # all 1s
        Q = {(alpha[u], alpha[v]): -1 for u, v in itertools.combinations(range(n_variables), 2)}
        response = qbs.QBSolv().sample_qubo(Q, seed=5342)
        sample = next(iter(response))
        for v in sample:
            self.assertEqual(sample[v], 1)

    def test_dimod_basic_ising(self):
        n_variables = 60

        h = {v: -1 for v in range(n_variables)}
        J = {}
        response = qbs.QBSolv().sample_ising(h, J)
        sample = next(iter(response))
        for v in sample:
            self.assertEqual(sample[v], 1)
