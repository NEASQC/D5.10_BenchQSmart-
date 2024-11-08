//
// Created by margarita on 11/10/22.
//

#include <fstream>
#include <iostream>
#include "Graph.h"
#include <algorithm>

#define LINE_SIZE 255

void Graph::init_empty(const int &size) {
    weights.resize(size, 0);
    node_number = size;
    for(int i = 0; i < size; i++) {
        representors[i] = i;
        active_nodes.insert(i);
    }
}

WTYPE Graph::get_max_weight() const {
    if(! weighted)
        return 1;

    auto it = active_nodes.begin();
    WTYPE max_weight = abs(weights.at(*it));
    it++;

    while (it != active_nodes.end()) {
        max_weight = max(max_weight, abs(weights.at(*it)));
        it++;
    }
    return max_weight;
}

void Graph::add_node_weight(const N_ID& u, const WTYPE& weight){
    N_ID u_rep = representors[u];
    weights[u_rep] += weight;
};


void Graph::add_edge(const N_ID& u, const N_ID& v) {
    if (!active_nodes.count(u) || !active_nodes.count(v))
        throw invalid_argument("One of the nodes isn't active");
    else if (u != v) {
        adj_list[u].insert(v);
        adj_list[v].insert(u);
    }
};
bool Graph::has_edge(const N_ID &u, const N_ID& v) const {
    if (!active_nodes.count(u) || !active_nodes.count(v))
        throw invalid_argument("One of the nodes isn't active");
    if (adj_list.find(u)==adj_list.end())
        return false;
    else return adj_list.at(u).count(v);
};

void Graph::merge_nodes(const N_ID &u, const N_ID& v) {
    N_ID rep_u = representors.at(u);
    N_ID rep_v = representors.at(v);

    if(has_edge(rep_u, rep_v))
        throw "Connected nodes can't be merged";

    active_nodes.erase(rep_v);
    for( auto& p: representors )
        if (p.second == rep_v)
            p.second = rep_u;

    adj_list[rep_u].insert(adj_list[rep_v].begin(), adj_list[rep_v].end());

    //Modify the adjacency list of rep_v's neighbors
    for(const auto& v_neighbor : adj_list[rep_v]){
        adj_list[v_neighbor].insert(rep_u);
        adj_list[v_neighbor].erase(rep_v);
    }

    adj_list.erase(rep_v);
    return;
};

void Graph::split_nodes(const N_ID &u, const N_ID& v){
    N_ID rep_u = representors.at(u);
    N_ID rep_v = representors.at(v);

    add_edge(rep_u, rep_v);
    return;
};

N_CONTAINER Graph::recover_all_merged_to(const N_CONTAINER &set) {
    N_CONTAINER result = set;
    for(int i = 0; i < node_number; i++){
        if (set.count(representors[i]))
            result.insert(i);
    }
    return result;
}

void Graph::init_node_weights(const vector<WTYPE>& new_weights) {
    if (new_weights.size() != node_number)
        throw invalid_argument("The size of the vector with weights doesn't match the number of nodes");
    for(auto i: active_nodes)
        set_node_weight(i, new_weights[i]);
    for(int i = 0; i < node_number; i++)
        if(!active_nodes.count(i))
            add_node_weight(i, new_weights[i]);
};

void Graph::read_dimacs(const string& filename) {
    ifstream file;
    file.open(filename);

    char c, comment[LINE_SIZE];
    N_ID u, v;
    WTYPE weight;
    int n_nodes, n_edges;
    weighted = false;

    while (file)
    {
        file >> c;
        switch (c) {
            case 'c':
                file.getline(comment, LINE_SIZE);
                break;
            case 'p': {
                file >> comment >> n_nodes >> n_edges;
                init_empty(n_nodes);
                edge_number = n_edges;
                break;
            }
            case 'e': {
                file >> u >> v ;
                if (n_edges >= 0) {
                    add_edge(u-1, v-1);
                    n_edges--;
                } else {
                    file.close();
                    cerr << "No more edges to add.\n";
                }
                break;
            }
            case 'n': {
                weighted = true;
                file  >> u >> weight ;
                set_node_weight(u-1, weight);
                break;
            }
            default :
            {
                cerr << "Unknown command. Reading aborted." << std::endl;
                file.close();
                break;
            }
        }
    }
    if(!weighted){
        for(int u = 0; u < n_nodes; u++)
            weights[u] = 1;
    }
    file.close();
};

bool is_independent_set(const Graph& G, const N_CONTAINER& IS){
    for(const auto & u: IS)
        for( const auto & v: IS)
            if (u!=v & G.has_edge(u, v))
                return false;
    return true;
}