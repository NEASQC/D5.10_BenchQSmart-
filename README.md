# Benchmarking suite for smart charging : NEASQC WP 5.10 

This readme correspond to the benchmarking suite for smart charging problems studied in the [NEASQC](https://neasqc.eu/) developped during the PhD work of Margarita Veshchezerova : Quantum algorithms for the [energy management optimization problems](https://theses.hal.science/tel-04105922/) [3]. 

This library is a software implementing a quantum-inspired approach and compares it to classical approaches. 

**This program allows to compute the Maximum Weighted Independed Set in a graph and the Graph Coloring.**

# Basic Installation procedure 
* The library require NLopt (version 2.7.1), a .tar.gz archive of NLopt is inside the library
1) Install Nlopt in nlopt_install folder <p>
  1.1) Uncompress Nlopt library :  <p>
  `tar -xzf nlopt-2.7.1.tar.gz` <p>
  1.2) Move in nlopt-2.7.1 folder : <p>
   `cd nlopt-2.7.1`  <p>
  1.3) Compile nlopt and install it :  <p>
      `cmake .` <p>
      `make -j` <p>
      `cmake --install . --prefix ${PATH_TO_CURRENT_FOLDER}/nlopt_install` <p>
3) Library installation  <p>
  2.1) Get back in the root folder <p> `cd ..` <p>
  2.2)  `cmake . `  <p>
  2.3)  `make -j `  <p>

The binary file QUANTUM_BNP should be created at the root folder of the library.

## The library implement two heuristics for Maximum Weighted Independent Set :
- Heuristic 1 : greedy initialization from [1] enhanced with a local search [2]. 
- Heuristic 2 : firstly optimize the corresponding energy Hamiltonian with the hybrid quantum routine **RQAOA** [4] to further improve the obtained assignment with the local search [2]. 

Remark : Due to the parameter optimization routine the quantum heurisic is probabilistic, so the result may change between executions. If the quantum heuristic is not working very well, it may be due to the suboptimal values of parameters. In this case modify the macros MAX_OPT_TIME in Hamiltonian.cpp (initially set to 5).

To see the link between smart charging problems and the two problems solved by the library we refer to [3]. 

# Additional library installation 
## Optionnaly The library implement two exact methods for Maximum Weighted Independent Set but which might require advance settings :
- Exact 1 : implementation of the SEWELL method introduced in [1].
- Exact 2 : solves the corresponding Integer Linear Program with CPLEX

To activate those liraries you need to activate the corresponding QB_ENABLE_ option of the library and to change the corresponding set environnement line, the corresponding lines are between line 14 and 18 of the CMakeList : <p> <p>
  option(QB_ENABLE_SCIP "Use of SCIP library" OFF)<p>
  set(SCIP_DIR YOU_PATH_TO_SCIP)<p>
  option(QP_ENABLE_CPLEX "Use of CPLEX solver"	OFF)<p>
  set(ENV{CPLEX_ROOT} YOU_PATH_TO_CPLEX)<p>

## Version 
* SCIP (version 8.0.2)
* cplex (version 12.7)



## The graph coloring solution is computed with the **Quantum-assisted Branch & Price** [3].
  The Branch & Price procedure is implemented using SCIP library (version 8.0.0). The branching is done with *Ryan-Foster rule*. 
  The coloring procedure stops either when the optimal coloring is found or after the TIMELIMIT (specified at line 41 of the coloring.cpp file) seconds. 


# Library test
The program receives in input from 2 to 3 arguments.<p>
  `Quantum_BnP -solved_problem -instance_file (optional) -method`

* **solved_problem** indicates if we solve the graph coloring ( *-COLORING* ) or the Maximum Weighted Independent Set problem ( *-MWIS* )
* **instance_file** describes the instance in the DIMACS format (node indexes start at 1):
* **method**: for the MWIS problem specifies which exact or heuristic method should be called. Possible values are *-CPLEX* and *-sewell* (for exact methods) and *-greedy* or *-quantum* for heuristics

The program outputs in the standard output:
* For the MWIS problem - The set that was found by the specified method
* For the graph coloring problem - the color assingment and the RQAOA success rate

# Examples :
## For the *Maximum Independent Set* problem:

 `QUANTUM_BNP -MWIS test_data/graphs_100/GNP_0.5.mwis -quantum` <p>

The expected (probabilistic) output is: <p>

MWIS is independent: 1 <p>
-quantum returned an independent set of value: 9 <p>
The set is: 13 27 32 34 56 63 88 94 99 <p>

## For the *Coloring* problem you need to activate SCIP : <p> 
`QUANTUM_BNP -COLORING test_data/graphs_20/GNP_0.5.mwis` <p>
The expected output is: <p>
Branch & Price found a coloring with 6 colors.  <p>
RQAOA success rate is: 1  <p>
COLORING: 0 3 1 5 2 4 3 5 4 5 1 2 5 1 0 2 4 2 0 2  <p>

The folder test_data contains simple graphs of different node number and density. 
Each folder *graphs_n* in test_data contains graphs with *n* nodes. 
Files *GNP_0.d.mwis* are randomly generated instances with density *d*.


# General graph data
  * p #nodes #edges
  
  The list of the edges:
  * e u v 
  * ...
  * e u v
  
  The list of node weights (if not provided every node is assigned the weight 1)
  * n 1 weight
  * ...
  * n n weight
  * c ... May be used in any line for comments
  * If the line starts with any other character than c, p, e and n the execution is aborted

# AKNOWLEDGEMENTS AND BIBLIOGRAPHY
**This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement No 951821.**

[1] https://www.math.uwaterloo.ca/~bico/papers/safe_color.pdf

[2] https://www.researchgate.net/publication/221131653_Fast_Local_Search_for_the_Maximum_Independent_Set_Problem

[3] Veshchezerova, M. (2022). Quantum algorithms for energy management optimization problems. Theses, Université de Lorraine, December.

[4] Bravyi et al., 2020 Sergey Bravyi, Alexander Kliesch, Robert Koenig, and Eugene Tang. Obstacles to variational quantum optimization from symmetry protection. Phys. Rev. Lett., 125:260505, Dec 2020.
