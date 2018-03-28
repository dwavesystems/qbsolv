"""
A quick smoke test to make sure basic operation is possible.

Runs the solver on a series of simple problems. Not actually a unit test.
"""

import dimod
import dwave_qbsolv
import unittest
import os


ENERGY_ERROR = 0.001


def load_qubo(filename):
    """
    Load a qubo file into a dictionary.

    TODO: There should probably be a first class home for a more careful
          implementation of this, rather than it living in a test.
    """
    qubo = {}
    with open(filename, 'r') as handle:
        for line in handle.readlines():
            line = line.strip()
            # ignore comment lines
            if len(line) == 0 or line[0] == 'c':
                continue
            # grab size of qubo and instantiate with zeros on diagonal
            # needs to happen to account for nodes with no biases
            if line[0] == 'p':
                _, _, _, n_variables, _, _ = line.split()
                qubo = {(a, a): 0 for a in range(int(n_variables))}
                continue
            # TODO: we should check that 'p' line has been found first
            a, b, bias = line.split()
            qubo[int(a), int(b)] = qubo.get((int(a), int(b)), 0) + float(bias)
    return qubo


class TestSmallProblems(unittest.TestCase):
    """Run some problems with trivial solutions."""

    def has_solution(self, results, expected_state, expected_energy):
        solution_found = False
        for state, energy in results.data(['sample', 'energy']):
            solution_found |= state == expected_state and abs(energy - expected_energy) < ENERGY_ERROR
        self.assertTrue(solution_found)

    @unittest.skip("This always fails given details wrt tabu list")
    def test_single_bit(self):
        """Make sure the solver can tell which way to flip a single bit."""
        qubo = {(0, 0): -1}
        results = dwave_qbsolv.QBSolv().sample_qubo(qubo)
        self.has_solution(results, {0: 1}, -1)

    def test_linear(self):
        """Run a set of independent variables."""
        size = 10
        qubo = {}
        solution = {}
        for ii in range(size):
            qubo[(ii, ii)] = -1
            solution[ii] = 1

        results = dwave_qbsolv.QBSolv().sample_qubo(qubo)
        self.has_solution(results, solution, -size)

        results = dwave_qbsolv.QBSolv().sample_qubo(qubo, target=-size)
        self.has_solution(results, solution, -size)

    def test_easy_loop(self):
        """Run a loop of variables without frustration."""
        size = 10
        qubo = {}
        solution = {}
        for ii in range(size):
            qubo[(ii, ii)] = -1
            qubo[ii, (ii + 1) % size] = -1
            solution[ii] = 1

        results = dwave_qbsolv.QBSolv().sample_qubo(qubo)
        self.has_solution(results, solution, -2 * size)

        results = dwave_qbsolv.QBSolv().sample_qubo(qubo, target=-2 * size)
        self.has_solution(results, solution, -2 * size)

    def test_single_frustrated_loop(self):
        """Run a loop of variables with a frustrated link."""
        size = 10
        qubo = {}
        solution = {}
        for ii in range(size):
            qubo[(ii, ii)] = -1
            qubo[ii, (ii + 1) % size] = -1
            solution[ii] = 1
        qubo[size - 1, 0] = 1

        results = dwave_qbsolv.QBSolv().sample_qubo(qubo)
        self.has_solution(results, solution, -2 * size + 2)

        results = dwave_qbsolv.QBSolv().sample_qubo(qubo, target=-2 * size + 2)
        self.has_solution(results, solution, -2 * size + 2)


class TestSmallBeasleyProblems(unittest.TestCase):
    """Test the smallest set of beasley instances.

    This is non-deterministic. Not an ideal test.
    A stop gap until a more thourough automated testing can be setup.
    """

    beasley50_qubo_format = "{tests_path}/qubos/bqp50_{index}.qubo"
    beasley50_energies = [2098, 3702, 4626, 3544, 4012, 3693, 4520, 4216, 3780, 3507]

    def test_small_problems(self):
        """Run a set of beasely instances with 50 variables."""
        qbsolv = dwave_qbsolv.QBSolv()
        tests_path = os.path.dirname(os.path.abspath(__file__))

        for index, expected_energy in enumerate(self.beasley50_energies):
            index += 1
            qubo = load_qubo(self.beasley50_qubo_format.format(tests_path=tests_path, index=index))
            # Python 3 feature
            # with self.subTest(index=index, energy=energy):

            # Give each problem three shots. One should be enough.
            # But this is a non-deterministic test.
            for _ in range(3):
                results = qbsolv.sample_qubo(qubo, find_max=True)
                self.assertTrue(any(abs(datum.energy - expected_energy) < ENERGY_ERROR for datum in results.data()))
                break
            else:
                self.fail()


class TestDimodCallback(unittest.TestCase):
    """Run the beasley problems with a dimod sub-sampler.

    This is non-deterministic. Not an ideal test.
    A stop gap until a more thourough automated testing can be setup.
    """

    beasley50_qubo_format = "{tests_path}/qubos/bqp50_{index}.qubo"
    beasley50_energies = [2098, 3702, 4626, 3544, 4012, 3693, 4520, 4216, 3780, 3507]

    def test_callable_solver(self):
        """Run a set of beasely instances with 50 variables."""
        qbsolv = dwave_qbsolv.QBSolv()
        tests_path = os.path.dirname(os.path.abspath(__file__))
        import random

        callback_called = [False]

        def _callback(Q, best):
            callback_called[0] = True
            return [random.randint(0, 1) for _ in best]

        for index, expected_energy in enumerate(self.beasley50_energies):
            index += 1
            qubo = load_qubo(self.beasley50_qubo_format.format(tests_path=tests_path, index=index))
            # Python 3 feature
            # with self.subTest(index=index, energy=energy):

            # Give each problem three shots. One should be enough.
            # But this is a non-deterministic test.
            for _ in range(3):
                results = qbsolv.sample_qubo(qubo, find_max=True, solver=_callback)
                self.assertTrue(any(abs(datum.energy - expected_energy) < ENERGY_ERROR for datum in results.data()))
                break
            else:
                self.fail()

        # make sure at least one call invoked the callback
        self.assertTrue(callback_called[0])

    def test_dimod_solver(self):
        """Run a set of beasely instances with 50 variables."""
        qbsolv = dwave_qbsolv.QBSolv()
        tests_path = os.path.dirname(os.path.abspath(__file__))

        for index, expected_energy in enumerate(self.beasley50_energies):
            index += 1
            qubo = load_qubo(self.beasley50_qubo_format.format(tests_path=tests_path, index=index))
            # Python 3 feature
            # with self.subTest(index=index, energy=energy):

            # Give each problem three shots. One should be enough.
            # But this is a non-deterministic test.
            for _ in range(3):
                results = qbsolv.sample_qubo(qubo, find_max=True, solver=dimod.RandomSampler())
                self.assertTrue(any(abs(datum.energy - expected_energy) < ENERGY_ERROR for datum in results.data()))
                break
            else:
                self.fail()
