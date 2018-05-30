import random
import itertools

import networkx as nx

from dwave_qbsolv import QBSolv
from dwave.system.samplers import DWaveSampler
from dwave.system.composites import FixedEmbeddingComposite
import minorminer

# define (sub)problem size
solver_limit = 3
qubo_size = 4

# find embedding of subproblem-sized complete graph to the QPU
G = nx.complete_graph(solver_limit)
system = DWaveSampler()
embedding = minorminer.find_embedding(G.edges, system.edgelist)

# solve a random problem
Q = {t: random.uniform(-1, 1) for t in itertools.product(range(qubo_size), repeat=2)}
response = QBSolv().sample_qubo(Q, solver=FixedEmbeddingComposite(system, embedding), solver_limit=solver_limit)
print("Q=" + str(Q))
print("samples=" + str(list(response.samples())))
print("energies=" + str(list(response.data_vectors['energy'])))
