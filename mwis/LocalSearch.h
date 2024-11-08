//
// Created by margarita on 06/12/22.
//

#ifndef QUANTUM_BNP_LOCALSEARCH_H
#define QUANTUM_BNP_LOCALSEARCH_H
#include "../Graph.h"

/**
 * This class allows to improve a solution for the independent set problem by performing local modifications.
 * It implements the algorithm introduced in the paper [Fast Local Search for the Maximum Independent Set Problem] by D. Andrade, M. G. C. Resende and R. F. F. Werneck
 */
class LocalSearch {
    const Graph* graph;
    vector<int> tightness;
    N_CONTAINER active_nodes;
    N_CONTAINER free_nodes;
    N_CONTAINER deactivated_nodes;


    /** Initialize the data structures required for a local search
     *
     * @param g
     */
    void init(N_CONTAINER& IS);

    /** Find if there exists a 1-to-2 swap that improves the solution value
     *
     * @param exit stores the id of the node that should leave the solution
     * @param enter1, enter2 store the id of the node that should enter the solution
     * @return True if the method found an improving swap
     */
    bool find_swap(N_ID& exit, N_ID & enter1, N_ID & enter2);
    void remove_from_solution(const N_ID& u);

    /** Add the free node to the solution
     *
     * @param u
     * @throw  TODO
     */
    void add_to_solution(const N_ID& u);

    /** Transform the stored independent set to the maximal one
     *
     * The method adds iteratively free nodes to the solution
     */
    void to_maximal();

public:

    LocalSearch(const Graph *g) : graph(g) {};

    /** Performs a local search from a better independent set around the initial point
     *
     * @param IS the initial solution obtained with a constructive heuristic
     */
    void improve(N_CONTAINER& IS);
};


#endif //QUANTUM_BNP_LOCALSEARCH_H
