import dimod

from dwave_qbsolv.qbsolv import run_qbsolv, ENERGY_IMPACT, SOLUTION_DIVERSITY

__all__ = ['QBSolv', 'ENERGY_IMPACT', 'SOLUTION_DIVERSITY']


class QBSolv(dimod.TemplateSampler):
    """Wraps the qbsolv C package for python.

    Examples:
        >>> h = {0: -1, 1: 1, 2: -1}
        >>> J = {(0, 1): -1, (1, 2): -1}
        >>> response = QBSolv().sample_ising(h, J)
        >>> list(response.samples())
        '[{0: 1, 1: 1, 2: 1}]'
        >>> list(response.energies())
        '[1.0]'
    """

    @dimod.decorators.qubo(1)
    @dimod.decorators.qubo_index_labels(1)
    def sample_qubo(self, Q, n_repeats=50, seed=None, algorithm=None, **kwargs):
        """Samples low energy states defined by a QUBO using qbsolv.

        Args:
            Q (dict): A dictionary defining the QUBO. Should be of the form
                {(u, v): bias} where u, v are variables and bias is numeric.
            n_repeats (int, optional): Determines the number of times to
                repeat the main loop in qbsolv after determining a better
                sample. Default 50.
            seed (int, optional): Random seed. Default None.
            algorithm (int, optional): Which algorithm to use. Defaut
                None. Algorithms numbers can be imported from module
                under the names ENERGY_IMPACT and SOLUTION_DIVERSITY.

        Returns:
            :obj:`BinaryResponse`

        Examples:
            >>> Q = {(0, 0): 1, (1, 1): 1, (0, 1): 1}
            >>> response = QBSolv().sample_qubo(Q)
            >>> list(response.samples())
            '[{0: 0, 1: 0}]'
            >>> list(response.energies())
            '[0.0]'

        """

        if not isinstance(n_repeats, int) or n_repeats <= 0:
            raise ValueError("n_repeats must be a positive integer")

        # pose the QUBO to qbsolv
        samples, energies, counts = run_qbsolv(Q, n_repeats, seed, algorithm=algorithm, **kwargs)

        # load the response
        response = dimod.BinaryResponse()
        response.add_samples_from(samples, sample_data=({'count': n} for n in counts), Q=Q)

        return response
