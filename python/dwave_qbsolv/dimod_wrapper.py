import dimod

from qbsolv import run_qbsolv

__all__ = ['QBSolv']


class QBSolv(dimod.DiscreteModelSampler):
    def sample_qubo(self, Q, n_repeats=50, seed=None):
        """TODO"""

        samples, energies, counts = run_qbsolv(Q, n_repeats, seed)

        response = dimod.BinaryResponse()
        response.add_samples_from(samples, energies, ({'count': n} for n in counts))

        return response
