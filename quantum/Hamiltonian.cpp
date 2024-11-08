//
// Created by margarita on 06/12/22.
//

#include "Hamiltonian.h"
#include "../mwis/LocalSearch.h"
#include "nlopt.hpp"
#include <cmath>
#include <iostream>
#include <chrono>
#include <climits>
#include <numeric>

// Time budget for global parameter optimization routine

#define MAX_OPT_TIME 10

void Hamiltonian::update_common_neighbors() {
    for(const auto& u: active_nodes)
        for(const auto& v: active_nodes)
            if(u < v) {
                common_neighbors[u][v] = neighbors[u];
                common_neighbors[u][v].insert(neighbors[v].begin(), neighbors[v].end());
                common_neighbors[u][v].erase(u);
                common_neighbors[u][v].erase(v);
                common_neighbors[v][u] = common_neighbors[u][v];
            }
    return;
}

void Hamiltonian::set_linear(const N_ID &u, CTYPE weight) {
    if(active_nodes.count(u)) linear[u] = weight;
}

void Hamiltonian::set_quadratic(const N_ID &u, const N_ID &v, CTYPE weight) {
    if(abs(weight) < EPSILON)
    {
        neighbors[u].erase(v);
        neighbors[v].erase(u);
    }
    else
    {
        neighbors[u].insert(v);
        neighbors[v].insert(u);
    }
    quadratic[u*allocated + v] = quadratic[v*allocated + u] = weight;
}

double Hamiltonian::z_mean(const N_ID &u, const Parameters &p) const {
    if (abs(linear[u]) < EPSILON) return 0;
    double zu = sin(p.beta) * sin(p.gamma * linear[u]);
    for(const auto & v: neighbors[u])
        zu *= cos(p.gamma * quadratic[u*allocated + v]);
    return zu;
}

double Hamiltonian::zz_mean(const N_ID &u, const N_ID &v, const Parameters &p) const {
    double zuv = 0;
    if( abs(quadratic[u*allocated + v]) > EPSILON)
    {
        double cu = cos(p.gamma * linear[u]);
        for(const auto & x : neighbors[u])
            if(x != v)
                cu *= cos(p.gamma * quadratic[u*allocated + x]);
        double cv = cos(p.gamma * linear[v]);
        for(const auto & x: neighbors[v])
            if(x != u)
                cv *= cos(p.gamma * quadratic[v*allocated + x]);
        zuv = sin(2*p.beta)*sin(p.gamma * quadratic[u*allocated + v])*(cu + cv);
    }
    double cplus = cos(p.gamma * (linear[u] + linear[v]));
    double cminus = cos(p.gamma * (linear[u] - linear[v]));
    for(const auto & x: common_neighbors[u][v])
    {
        cplus *= cos(p.gamma * (quadratic[u*allocated + x] + quadratic[v*allocated + x]));
        cminus *= cos(p.gamma * (quadratic[u*allocated + x] - quadratic[v*allocated + x]));
    }
    zuv += (sin(p.beta) * sin(p.beta))*(cminus - cplus);
    return zuv/2;
}

double Hamiltonian::qaoa_mean(const Parameters &p) const {
    double mean = 0;
    for(auto it1 = active_nodes.begin(); it1 != active_nodes.end(); it1++) {

        if(!abs(linear[*it1]) < EPSILON)
            mean += linear[*it1] * z_mean(*it1, p);
        for(auto it2 = next(it1); it2 != active_nodes.end(); it2++)
            if(!abs(quadratic[*it1*allocated + *it2]) < EPSILON)
                mean += quadratic[*it1*allocated + *it2] * zz_mean(*it1, *it2, p);
    }
    return mean;
}

void Hamiltonian::remove_node(const N_ID &u) {
    for (const auto &v: neighbors[u])
        neighbors[v].erase(u);

    active_nodes.erase(u);
    actual_node_number--;
}

void Hamiltonian::add_constraint(Constraint c) {
    constraints.push_back(c);
    if(c.v != -1)
        linear[c.v] += c.sigma * linear[c.u];
    for(const auto& w: neighbors[c.u])
        if(w != c.v) {
            if (c.v != -1)
                set_quadratic(c.v, w, quadratic[c.v * allocated + w] + c.sigma * quadratic[c.u * allocated + w]);
            else
                set_linear(w, linear[w] + c.sigma * quadratic[c.u * allocated + w]);
        }
    remove_node(c.u);
    return;
}



void Hamiltonian::optimize_parameters(Parameters& p, bool& in_neighborhood) const {

    // Compute how much time takes the optimization

    auto start = std::chrono::system_clock::now();

    //The objective function
    auto f = [](const vector<double> &x, vector<double>&, void* f_data){
        auto instance = (Hamiltonian* ) f_data;
        return instance->qaoa_mean({x[0], x[1]});
    };

    //The parameter vector
    vector<double> opt_x(2);

    //The optimum value
    double opt_val;

    // Set up the optimizer
    nlopt::opt local_optimizer(nlopt::algorithm::LN_BOBYQA, 2);

    //Termination condition
    local_optimizer.set_xtol_abs(0.0001);
    local_optimizer.set_ftol_rel(0.01);

    //If the initial point is not specified find it with a global search

    if(!in_neighborhood) {
        //Optimization routines
        nlopt::opt global_optimizer(nlopt::algorithm::GD_MLSL, 2);
        global_optimizer.set_local_optimizer(local_optimizer);

        //Set the objective
        global_optimizer.set_min_objective(f, (void *) this);

        //Bounds
        global_optimizer.set_population(actual_node_number);
        global_optimizer.set_lower_bounds(0);

        //Termination condition
        global_optimizer.set_upper_bounds(2* M_PI);
        global_optimizer.set_xtol_abs(0.001);
        global_optimizer.set_maxtime(MAX_OPT_TIME);

        global_optimizer.optimize(opt_x, opt_val);
    }
    else
    {
        opt_x[0] = p.beta;
        opt_x[1] = p.gamma;
    }

    //Search around the initialization point with a local method

    //Set bounds
    local_optimizer.set_lower_bounds(0);
    local_optimizer.set_upper_bounds(2* M_PI);

    //Strengthen the termination  condition
    local_optimizer.set_ftol_rel(0.001);
    local_optimizer.set_maxtime(MAX_OPT_TIME);

    local_optimizer.set_min_objective(f, (void *) this);
    local_optimizer.optimize(opt_x, opt_val);

    auto elapsed_seconds = std::chrono::system_clock::now()-start;
    p.beta = opt_x[0];
    p.gamma = opt_x[1];
    in_neighborhood = true;
//    cout << "Optimal parameters: " << opt_x[0] << " " << opt_x[1] << endl;
}

Constraint Hamiltonian::find_max_correlation(const Parameters &p) {
    Constraint output = {1, *active_nodes.begin(), -1};
    double max_abs_corr_value = 0;
    for(auto const & u: active_nodes)
    {
        double val = z_mean(u, p);
        if ( abs(val) > max_abs_corr_value)
        {
            max_abs_corr_value = abs(val);
            int sigma = val < 0 ? -1 : 1;
            output = {sigma, u, -1};
        }
        for(auto const &v : active_nodes)
            if(u < v)
            {
                double val = zz_mean(u, v, p);
                if (abs(val) > max_abs_corr_value)
                {
                    max_abs_corr_value = abs(val);
                    int sigma = val < 0 ? -1 : 1;
                    output = {sigma, u, v};
                }
            }
    }
//    cout << "Maximum correlation: " << max_abs_corr_value << "; ";
    return output;
}

bool next_vector(vector<int>& proper_vector)
{
    for (int i = proper_vector.size()-1; i >=0; i--)
        if(proper_vector[i]==-1)
        {
            proper_vector[i] = 1;
            return true;
        }
        else
            proper_vector[i] = -1;
    return false;
};

void Hamiltonian::solve_by_brute_force() {

    vector<int> best_vector(actual_node_number, -1);
    vector<int> proper_vector = best_vector;

    vector<int> node_id(active_nodes.begin(), active_nodes.end());

    // TODO throw an exception if the size is too large
    CTYPE min_val = INT_MAX;
    do
    {
        CTYPE value = 0;
        for(int i = 0; i < actual_node_number; i++) {
            value += linear[node_id[i]] * proper_vector[i];
            for(int j = i+1; j < actual_node_number; j++)
                value += quadratic[node_id[i] * allocated + node_id[j]] * proper_vector[i] * proper_vector[j];
        }
        if(value < min_val) {
            min_val = value;
            best_vector = proper_vector;
        }
    }
    while(next_vector(proper_vector));

    for(int i = 0; i < actual_node_number; i++)
        constraints.push_back({best_vector[i], node_id[i], -1});
    active_nodes.clear();
    return;
}


vector<int> Hamiltonian::rqaoa() {
    bool params_are_initialized = false;
    Parameters p;

    //Eliminate nodes until the problem becomes sufficiently small for the brute-force method
    while (actual_node_number > BF_LIMIT){
        update_common_neighbors();
        optimize_parameters(p, params_are_initialized);
        Constraint c = find_max_correlation(p);
        //       cout << c.sigma << " " << c.v << " " << c.u;
        add_constraint(c);
    }
    //Solve the small problem with brute-force groundstate search
    solve_by_brute_force();

    vector<int> result(allocated, -1);
    while(!constraints.empty())
    {
        Constraint c = constraints.back();
        c.v == -1 ? result[c.u] = c.sigma : result[c.u] = c.sigma * result[c.v];
        constraints.pop_back();
    }
    return result;
}

/**Initialize the instance of Hamiltonian whose ground state encodes the Maximum Weighted Independent Set
 *
 * @param graph
 * @param node_id stores the association between variables in hamiltonian and nodes in the graph
 * @return
 */
Hamiltonian get_MWIS_Hamiltonian(const Graph& graph, vector<int>& node_id)
{
    //Recover active nodes
    N_CONTAINER active_in_graph = graph.get_active_nodes();

    //Remove the nodes of zero weight from consideration
    auto it = active_in_graph.begin();
    while(it != active_in_graph.end())
    {
        if (abs(graph.get_node_weight(*it)) < EPSILON)
            it = active_in_graph.erase(it);
        else
            it++;
    }

    int n_vars = active_in_graph.size(); // Number of variables in the Hamiltonian

    //Associate to each active node of non-zero weight a variable index in the Hamiltonian

    node_id = vector<int>(active_in_graph.begin(), active_in_graph.end());
    Hamiltonian h(n_vars);

    //Penalty
    WTYPE max_weight = graph.get_max_weight();
    CTYPE lambda = 1 + ceil(max_weight);

    //Transform double weights to integer ones for weighted graphs to ensure period [0, 2pi] in parameter values
    int scale = 1;

    if(graph.is_weighted())
        scale = n_vars / max_weight;

    lambda *= scale;

    vector<CTYPE> integer_weights;
    for(const auto& u: active_in_graph) {
        //Minus sign as we transform maximization to minimization
        WTYPE scaled_coeff = -round(2 * scale * graph.get_node_weight(u));
        for(const auto& v: active_in_graph){
            if(graph.has_edge(u, v))
                scaled_coeff += lambda;
        }
        integer_weights.push_back(scaled_coeff);
    }

    //Divide all coefficients by the greatest common divisor to reinsure that the period is not a subspace of [0, 2pi]
    int divisor = lambda;
    for(const auto& coeff: integer_weights)
        divisor = gcd(divisor, coeff);
    for(auto& coeff : integer_weights)
        coeff /= divisor;
    lambda /= divisor;

    //Initialize the Hamiltonian
    for(int i = 0; i < n_vars; i++)
    h.set_linear(i, integer_weights[i]);

    for(int i = 0; i < n_vars; i++)
        for(int j = i+1; j < n_vars; j++)
        {
            if (graph.has_edge(node_id[i], node_id[j]))
                h.set_quadratic(i, j, lambda);
        }

    return h;
}


bool quantumMWIS(const Graph& graph, N_CONTAINER& IS, WTYPE& IS_weight, const WTYPE& cutoff)
{
    //Initialize the problem
    vector<int> node_id;
    Hamiltonian h = get_MWIS_Hamiltonian(graph, node_id);

    //Find the ground state
    auto approx_ground_state = h.rqaoa();

    //Recover the solution
    N_CONTAINER rqaoa_is;
    for(int i = 0; i < node_id.size(); i++)
        if(approx_ground_state[i] == 1)
            rqaoa_is.insert(node_id[i]);

    if(! is_independent_set(graph, rqaoa_is)){
        cout << "RQAOA OBTAINED UNFEASIBLE SOLUTION." << endl;
        return false;
    }

//Improve the solution found by RQAOA with local search
    LocalSearch ls(&graph);
    ls.improve(rqaoa_is);

    WTYPE rqaoa_weight = 0;
    for (const auto& u: rqaoa_is)
        rqaoa_weight += graph.get_node_weight(u);

    if(rqaoa_weight > IS_weight){
        IS_weight = rqaoa_weight;
        IS = rqaoa_is;
    }
    return IS_weight > cutoff;
}
