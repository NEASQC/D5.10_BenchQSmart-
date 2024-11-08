//
// Created by margarita on 06/12/22.
//

#ifndef QUANTUM_BNP_HAMILTONIAN_H
#define QUANTUM_BNP_HAMILTONIAN_H

#include "../Graph.h"
#include "../mwis/mwis.h"
#include <list>

// Threshold for the brute-force solution

#define BF_LIMIT 12

using CTYPE = int;

using Parameters =  struct Parameters
{
    double beta;
    double gamma;
};


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

using Constraint = struct Constraint{
    //The value -1 or +1 that should take the variable Z_u (or the pair Z_u Z_v)
    int sigma;

    //The first node in the constraint
    N_ID u;

    //The second node in the constraint, if v = -1 than the constraint involves only the node u (Z_u = sigma), otherwise Z_uZ_v = sigma
    N_ID v;
};

class Hamiltonian {

    //Inital size of the instance (before RQAOA)
    int allocated;

    //Size of the instance at the current step of RQAOA
    int actual_node_number;

    //Nodes not eliminated at the current step of RQAOA
    N_CONTAINER active_nodes;

    //Constraints added by RQAOA
    list<Constraint> constraints;

    //For each active node stores a list of other nodes connected by a non-zero interaction term J_i,j in the Ising Hamiltonian
    vector<N_CONTAINER> neighbors;

    //Service structure - for each pair of active nodes stores the union of their neighbors, accelerate the computation of mean values.
    vector<vector<N_CONTAINER>> common_neighbors;

    //Linear coefficients of the Ising model
    vector<CTYPE> linear;

    //Quadratic coefficients of the Ising model
    vector<CTYPE> quadratic;

    /** After a variable is removed and the instance is modified - updates the list of common neighbors
     *
     */
    void update_common_neighbors();

public:
    Hamiltonian(int n): linear(n, 0), quadratic(n*n, 0), actual_node_number(n), allocated(n), neighbors(n), common_neighbors(n, vector<N_CONTAINER>(n)) {
        for(int i = 0; i < n; i++)
            active_nodes.insert(i);
    };

    /** Compute the mean value of the operator Z_u
     *
     * @param u id of the node
     * @param p parameters of QAOA_1 circuit
     * @return the mean value <b, g|Z_u| b, g>
     */
    double z_mean(const N_ID& u, const Parameters& p) const;
    /** Compute the mean value of the operator Z_u,v
     *
     * @param u id of the node
     * @param p parameters of QAOA_1 circuit
     * @return the mean value <b, g|Z_u,v| b, g>
     */
    double zz_mean(const N_ID& u, const N_ID& v, const Parameters& p) const;

    /** Compute the mean energy of the Hamiltonian at the point p the with an analytical function
     *
     * @param p
     * @return the mean energy in QAOA_1(p.beta, p.gamma) state
     */
    double qaoa_mean(const Parameters& p) const;

    /** Modifies the linear coefficient of the node u
     *
     * @param u
     * @param weight
     */
    void set_linear(const N_ID& u, CTYPE weight);

    /** Modifies the quadratic coefficient of the term Z_uZ_v
     *
     * @param u
     * @param v
     * @param weight
     */
    void set_quadratic(const N_ID& u, const N_ID& v, CTYPE weight);


    void remove_node(const N_ID&  u);

    /** Add a constraint, modify the Hamiltonian, and remove a variable
     *
     * @param c
     */
    void add_constraint(Constraint c);

    /** Optimize parameters of QAOA for the Hamiltonian
     *
     * @param p
     * @param in_neighborhood True if we search for an optimum in the neighborhood of p
     * @throw TODO an exception when the optimizer fails (see nlopt::opt::optimize)
     * @note If the initial point is not provided finds it with a multistart global search. Then use BOBYQA method for the local search
     */
    void optimize_parameters(Parameters& p, bool& in_neighborhood) const;

    Constraint find_max_correlation(const Parameters& p);

    /** Compute the exact ground state of the Hamiltonian with a brute-force approach
     *
     * If the number of active nodes is larger than BF_limit throw an exception
     * @throw TODO if the instance is too large
     */
    void solve_by_brute_force();

    /** Compute the approximate ground state of the Hamiltonian with RQAOA
     *
     * @return an approximate solution x \in {-1, 1}^n
     * @note the algorithm was introduced in the paper [Obstacles to State Preparation and Variational Optimization from Symmetry Protection] by S/ Bravyi, A. Kliesch, R. Koenig, E. Tang
     */
    vector<int> rqaoa();
};

#endif //QUANTUM_BNP_HAMILTONIAN_H
