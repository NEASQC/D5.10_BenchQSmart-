//
// Created by margarita on 09/12/22.
//
#include "mwis.h"
#include "LocalSearch.h"
#include <functional>
#include <list>
#include <iostream>

//Number of different permutations of the order
#define N_ORDERS 5


/** Find a Maximal Independent Set with respect to a given order
 *
  * @param graph
  * @param priority the order of nodes
  * @param dynamic true if the order is modified after a vertex is added to a solution
  * @param best_curr_is stores the best known independent set
  * @param best_curr_weight stores the weight of the best indepedent set
  */
void maximalIS(const Graph& graph, unordered_map<int, WTYPE>& priority, bool dynamic, N_CONTAINER& best_curr_is, WTYPE& best_curr_weight)
{
    N_CONTAINER active_nodes = graph.get_active_nodes();
    N_CONTAINER IS;

    //Add active node with the highest priority
    while (!active_nodes.empty())
    {
        //Find node with the highest priority
        auto it = max_element(active_nodes.begin(), active_nodes.end(),
                              [&priority](auto u, auto v) {return priority[u] < priority[v];});
        N_ID u = *it;

        //Eliminate the neighbors and modify priorities if necessary
        for(const auto & v: graph.get_neighbors(u)) {
            if(dynamic)
                for(const auto & v_neighbor : graph.get_neighbors(v)){
                    if(active_nodes.count(v_neighbor))
                        priority[v_neighbor] += graph.get_node_weight(v_neighbor);
                }
            active_nodes.erase(v);
        }
        IS.insert(u);
        active_nodes.erase(u);
    }

    // Improve the greedy solution with local search
    LocalSearch ls(&graph);
    ls.improve(IS);

    //If the maximal independent set is better than the best previously known set - modify it
    WTYPE maximal_is_weight = graph.get_nodeset_weight(IS);
    if(maximal_is_weight > best_curr_weight){
        best_curr_is = IS;
        best_curr_weight = maximal_is_weight;
    }
}


bool greedyMWIS(const Graph& graph, N_CONTAINER& IS, WTYPE& IS_weight, const WTYPE& cutoff){
    unordered_map<int, WTYPE> weight_priority;
    for (const auto & u: graph.get_active_nodes()){
        weight_priority[u] = graph.get_node_weight(u);
    }

    unordered_map<int, WTYPE> surplus_priority = weight_priority;
    for (const auto & u : graph.get_active_nodes()){
        for(const auto & v : graph.get_neighbors(u))
            surplus_priority[v] -= graph.get_node_weight(u);
    }

    unordered_map<int, WTYPE> dynamic_surplus_priority = surplus_priority;

    WTYPE factor = graph.get_node_number() * graph.get_max_weight();

    //Iterate over different orders and first elements to get the best independent set

    for(int i = 0; i < N_ORDERS; i++){

        //Find is maximal sets for any of orders improves the current best known independent set
        maximalIS(graph, weight_priority, false, IS, IS_weight);
        maximalIS(graph, surplus_priority, false, IS, IS_weight);
        maximalIS(graph, dynamic_surplus_priority, true, IS, IS_weight);

        if(IS_weight > cutoff) break;

        //Move the top element to the end of the list
        auto it_w = max_element(weight_priority.begin(), weight_priority.end(), [](auto u, auto v){ return u.first < v.first; });
        if(it_w!=weight_priority.end())
            it_w->second -= factor;
        

        auto it_s = max_element(surplus_priority.begin(), surplus_priority.end(), [](auto u, auto v){ return u.first < v.first; });
        if(it_s!=weight_priority.end())
            it_s->second -= factor;

        dynamic_surplus_priority = surplus_priority;
    }
    return IS_weight > cutoff;
}