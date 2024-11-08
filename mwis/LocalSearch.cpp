//
// Created by margarita on 06/12/22.
//

#include "LocalSearch.h"
#include <algorithm>

#include "../Graph.h"

void LocalSearch::init(N_CONTAINER& IS) {
    active_nodes = IS;
    deactivated_nodes = {};
    tightness = vector<int>(graph->get_node_number(), 0);
    for(const auto& u: active_nodes)
        for(const auto& v: graph->get_neighbors(u))
            tightness[v]++;
    for(const auto& u: graph->get_active_nodes())
        if(!active_nodes.count(u) && tightness[u] == 0)
            free_nodes.insert(u);
    to_maximal();
}

bool LocalSearch::find_swap(N_ID &exit, N_ID & enter1, N_ID & enter2) {
    auto it = active_nodes.begin();
    while (it != active_nodes.end()) {
        vector<N_ID> one_tight_neighbors{};
        N_ID u = *it;
        for (const auto &v: graph->get_neighbors(u))
            if (tightness[v] == 1)
                one_tight_neighbors.push_back(v);

        for (auto v = one_tight_neighbors.begin(); v != one_tight_neighbors.end(); v++) {
            auto neighbors = graph->get_neighbors(*v);
            auto w = v + 1;
            auto v_neighbor = neighbors.begin();
            while (w != one_tight_neighbors.end() && v_neighbor != neighbors.end()) {
                if (graph->get_node_weight(*w) + graph->get_node_weight(*v) >= graph->get_node_weight(u)) {
                    if (*w < *v_neighbor) break;
                    else if (*w == *v_neighbor)  w++;
                    v_neighbor++;
                }
                else w++;
            }
            if (w != one_tight_neighbors.end()) {
                exit = u; enter1 = *v; enter2 = *w;
                return true;
            }
        }
        it = active_nodes.erase(it);
        deactivated_nodes.insert(u);
    }
    return false;
}

void LocalSearch::remove_from_solution(const N_ID &u) {
    if(!active_nodes.count(u))
        throw std::range_error("Can't remove vertex that is not in solution");
    for(const auto& v: graph->get_neighbors(u)) {
        tightness[v]--;
        if(tightness[v] == 0)
            free_nodes.insert(v);
        if(tightness[v] == 1)
        {
            auto it = find_if(deactivated_nodes.begin(), deactivated_nodes.end(), [this, v](const N_ID& u){return graph->has_edge(u, v);});
            if(it != deactivated_nodes.end())
            {
                active_nodes.insert(*it);
                deactivated_nodes.erase(it);
            }
        }
    }
    active_nodes.erase(u);
}

void LocalSearch::add_to_solution(const N_ID &u) {
    if(!free_nodes.count(u))
        throw std::range_error("Can't add vertex that is not free");
    for(const auto& v: graph->get_neighbors(u)) {
        tightness[v]++;
        free_nodes.erase(v);
    }
    free_nodes.erase(u);
    active_nodes.insert(u);
}


void LocalSearch::to_maximal() {
    while(!free_nodes.empty())
    {
        auto it = free_nodes.begin();
        N_ID u = *it;
        add_to_solution(u);
    }
}

void LocalSearch::improve(N_CONTAINER &IS) {
    init(IS);
    N_ID exit, enter1, enter2;
    while (find_swap(exit, enter1, enter2))
    {
        remove_from_solution(exit);
        add_to_solution(enter1);
        add_to_solution(enter2);
        to_maximal();
    }
    IS = deactivated_nodes;
    return;
}