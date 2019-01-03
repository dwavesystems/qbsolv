import unittest
import time
import itertools

import dwave_qbsolv as qbs
import dimod

import random

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

        for sample, energy in response.data(['sample', 'energy']):
            self.assertEqual(dimod.qubo_energy(sample, Q), energy)

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

    def test_energy_calculation(self):
        n_variables = 15

        h = {alpha[v]: random.uniform(-2, 2) for v in range(n_variables)}
        J = {(alpha[u], alpha[v]): random.uniform(-1, 1)
             for u, v in itertools.combinations(range(n_variables), 2) if random.random() > .98}

        response = qbs.QBSolv().sample_ising(h, J)

        for sample, energy in response.data(['sample', 'energy']):
            self.assertAlmostEqual(dimod.ising_energy(sample, h, J), energy)

    # these tests are hard to automate for CI because different systems have different
    # speeds
    # def test_timeout_parameter(self):
    #     # set up a problem we hope will take a long time
    #     Q = {edge: random.uniform(-1, 1) for edge in itertools.combinations_with_replacement(range(500), 2)}

    #     timeout = 1 # in seconds

    #     t = time.time()

    #     response = qbs.QBSolv().sample_qubo(Q, timeout=timeout)  # seconds to millisconds
        
    #     # let's be generous and give it 3x the timeout
    #     self.assertLessEqual(time.time() - t, 3*timeout)
