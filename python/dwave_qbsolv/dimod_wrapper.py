import dimod

from qbsolv import run_qbsolv, ENERGY_IMPACT, SOLUTION_DIVERSITY

__all__ = ['QBSolv', 'ENERGY_IMPACT', 'SOLUTION_DIVERSITY']


class QBSolv(dimod.TemplateSampler):

    @dimod.decorators.qubo(1)
    @dimod.decorators.qubo_index_labels(1)
    def sample_qubo(self, Q, n_repeats=50, seed=None, algorithm=None):
        """TODO"""

        if not isinstance(n_repeats, int) or n_repeats <= 0:
            raise ValueError("n_repeats must be a positive integer")

        # pose the QUBO to qbsolv
        samples, energies, counts = run_qbsolv(Q, n_repeats, seed, algorithm=algorithm)

        # load the response
        response = dimod.BinaryResponse()
        response.add_samples_from(samples, energies, ({'count': n} for n in counts))

        return response
