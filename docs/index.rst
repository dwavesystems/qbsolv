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
  Source <https://github.com/dwavesystems/qbsolv>

.. toctree::
  :caption: Ocean Software
  :maxdepth: 1

  Ocean Home <https://ocean.dwavesys.com/>
  Ocean Documentation <https://docs.ocean.dwavesys.com>
  Ocean Glossary <https://docs.ocean.dwavesys.com/en/latest/glossary.html>

.. toctree::
  :caption: D-Wave
  :maxdepth: 1

  D-Wave <https://www.dwavesys.com>
  Leap <https://cloud.dwavesys.com/leap/>
  D-Wave System Documentation <https://docs.dwavesys.com/docs/latest/index.html>

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
* :ref:`glossary`
