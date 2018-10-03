..  -*- coding: utf-8 -*-

.. _contents:

======
qbsolv
======

.. include:: ../README.rst
  :start-after: index-start-marker
  :end-before: index-end-marker

Example
=======

.. code:: python

    from dwave_qbsolv import QBSolv
    Q = {(0, 0): 1, (1, 1): 1, (0, 1): 1}
    response = QBSolv().sample_qubo(Q)
    print("samples=" + str(list(response.samples())))
    print("energies=" + str(list(response.data_vectors['energy'])))



Documentation
=============

.. only:: html

  :Date: |today|

.. toctree::
  :maxdepth: 1

  intro
  source/index
  installation
  license
  contributing_qbsolv

.. toctree::
  :caption: D-Wave's Ocean Software
  :maxdepth: 1

  ocean
  contributing

.. toctree::
  :caption: D-Wave
  :maxdepth: 1

  dwave
  leap
  sysdocs

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
* :ref:`glossary`
