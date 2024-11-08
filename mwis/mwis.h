//
// Created by margarita on 06/12/22.
//

#ifndef QUANTUM_BNP_MWIS_H
#define QUANTUM_BNP_MWIS_H

#include "../Graph.h"


/** A greedy heuristic that finds a weighted independent set of weight above some threshold
 *
 * It finds the maximal independent sets with respect to one of three orders and stops when the best_mwis > cutoff.
 * If the required set is not found, orders are modified and the search continues for a fixed amount of steps.
 * To get an approximately optimal  weighted independent set we should set cutoff = INF.
 *
 * @param G the input graph
 * @param best_mwis in input constains the best previously known MWIS, is modified if the function finds a better solution
 * @param best_mwis_value the value of best_mwis
 * @param cutoff
 * @return True if the method finds an independent set of weight > cutoff
 * @note The used orders are specified in the paper [Maximum-Weight Stable Sets and Safe Lower Bounds For Graph Coloring]
 */
bool greedyMWIS(const Graph& G, N_CONTAINER& best_mwis, WTYPE & best_mwis_value, const WTYPE & cutoff);

/** A quantum heuristic based on RQAOA that finds a weighted independent set of weight above some threshold
 *
 * The method defines the Hamiltonian corresponding to the MWIS problem. Independence constraint is enforced with penalties.
 * It uses RAOA to approximate ground states of the resulting Hamiltonian.
 * If the approximate ground state correspond to an infeasible solution the set is modified to satisfy the constraints.
 *
 * @param G the input graph
 * @param best_mwis in input constains the best previously known MWIS, is modified if the function finds a better solution
 * @param best_mwis_value the value of best_mwis
 * @param cutoff
 * @return True if the method finds an independent set of weight > cutoff
 * @note The used orders are specified in the paper [Maximum-Weight Stable Sets and Safe Lower Bounds For Graph Coloring]
 */
bool quantumMWIS(const Graph& G, N_CONTAINER& best_mwis, WTYPE & best_mwis_value, const WTYPE & cutoff);

/** An exact method based on Branching that finds a weighted independent set of weight above some threshold
 *
 * The method stops when it find a set of weight < threshold. If threshold = INF the method finds an exact optimal solution
 *
 * @param G the input graph
 * @param best_mwis in input constains the best previously known MWIS, is modified when the function finds a better solution
 * @param best_mwis_value the value of best_mwis
 * @param cutoff
 * @return True if the method finds an independent set of weight > cutoff
 * @note The method was introduced in the paper [Maximum-Weight Stable Sets and Safe Lower Bounds For Graph Coloring]
 */
//bool sewellMWIS(const Graph& G, N_CONTAINER& best_mwis, WTYPE & best_mwis_value, const WTYPE & cutoff);

#ifdef QB_ENABLE_CPLEX
/** An exact method for MWIS solving the ILP formulation with CPLEX solver
 *
 * @param G the input graph
 * @param best_mwis in input constains the best previously known MWIS, is modified when the function finds a better solution
 * @param best_mwis_value the value of best_mwis
 * @param cutoff
 * @return True if the method finds an independent set of weight > cutoff
 */
bool cplexMWIS(const Graph& G, N_CONTAINER& best_mwis, WTYPE & best_mwis_value, const WTYPE & cutoff) ;
#endif
#endif //QUANTUM_BNP_MWIS_H
