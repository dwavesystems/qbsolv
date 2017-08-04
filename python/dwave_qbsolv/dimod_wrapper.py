import dimod

from qbsolv import run_qbsolv

__all__ = ['QBSolv']


class QBSolv(dimod.DiscreteModelSampler):

    @dimod.decorators.qubo(1)
    @dimod.decorators.qubo_index_labels(1)
    def sample_qubo(self, Q, n_repeats=50, seed=None):
        """TODO"""

        if not isinstance(n_repeats, int) or n_repeats <= 0:
            raise ValueError("n_repeats must be a positive integer")

        # pose the QUBO to qbsolv
        samples, energies, counts = run_qbsolv(Q, n_repeats, seed)

        # load the response
        response = dimod.BinaryResponse()
        response.add_samples_from(samples, energies, ({'count': n} for n in counts))

        return response

    @dimod.decorators.ising(1, 2)
    def sample_ising(self, h, J, *args, **kwargs):
        Q, offset = dimod.ising_to_qubo(h, J)
        binary_response = self.sample_qubo(Q, *args, **kwargs)
        return binary_response.as_spin(offset)
