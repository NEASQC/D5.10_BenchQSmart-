//
// Created by margarita on 11/10/22.
//

#ifndef QUANTUM_BNP_GRAPH_H
#define QUANTUM_BNP_GRAPH_H

#include <vector>
#include <set>
#include <unordered_map>
#include <stdexcept>

using namespace std;

//Type of the unique node identifier
using N_ID = int;

//Type of node weights
using WTYPE = double;

//The largest value of weight
#define INF numeric_limits<WTYPE>::max()

//If a WTYPE variable has a value lower than EPSILON we assume it to be zero
#define EPSILON 10*numeric_limits<WTYPE>::epsilon()

//Type of node lists
using N_CONTAINER = set<N_ID>;

//Graph without loops
class Graph {
private:
    bool weighted;
    int node_number;

    int edge_number;
    vector<WTYPE> weights; //TODO I chainged the type, see what crushes
    unordered_map<N_ID, N_CONTAINER> adj_list;

    // The indexes of nodes that were not merged to others
    N_CONTAINER active_nodes;

    // If representator[u] != u than the node u was merged to representator[u]
    unordered_map<N_ID, N_ID> representors;

    /** Create an empty graph
     *
     * @param size number of nodes
     * @note nodes are indexed from 0 to size, each node is its own representor
     */
    void init_empty(const int& size);
public:
    bool is_weighted() const { return weighted; };
    int get_node_number() const { return node_number; };
    int get_edge_number() const { return edge_number; };
    int get_degree(const N_ID& u) const { if (!active_nodes.count(u)) throw invalid_argument("The node isn't active"); else return adj_list.at(u).size(); };
    N_CONTAINER get_neighbors(const N_ID& u) const { const std::set<int> emptyReturn; return (adj_list.find(u) != adj_list.end()) ? adj_list.at(u) : emptyReturn;}
    N_CONTAINER get_active_nodes() const { return  active_nodes; };

    WTYPE get_node_weight(const N_ID& u) const { if (!active_nodes.count(u)) throw invalid_argument("The node isn't active"); else return weights.at(u); } ;
    WTYPE get_nodeset_weight(const N_CONTAINER& set) const { WTYPE res = 0; for(const auto& u: set) res += get_node_weight(u); return res; }

    /**Computes the largest absolute weight of an active node in the graph
     *
     * @return
     */
    WTYPE get_max_weight() const;
    /** Return the difference between the node weight and the cumulate weight of its neighbors
     *
     * @param u
     * @return
     */
    WTYPE get_surplus(const N_ID& u) const { auto w = get_node_weight(u); for(const auto & v: adj_list.at(u)) w -= get_node_weight(v); return w; };

    void set_weights_to_zero() {fill(weights.begin(), weights.end(), 0); }

    /** Set the new weight to an active node
     *
     * @param u
     * @param weight
     * @throw invalid_argument if the node u isn't active
     */
    void set_node_weight(const N_ID& u, const WTYPE& weight) { if (!active_nodes.count(u)) throw invalid_argument("The node isn't active"); else weights[u] = weight; };

    /** Add the weight of an inactive node to the weight of its representor
     *
     * @param u
     * @param weight
     * @note used in the pricing procedure
     */
    void add_node_weight(const N_ID& u, const WTYPE& weight);

    void add_edge(const N_ID& u, const N_ID& v);
    bool has_edge(const N_ID &u, const N_ID& v) const;

    /** Merge the nodes u and v
     *
     * u and v get the same representor. The method merges all nodes previously merged to v (v included) to the representor of u.
     * The number of active nodes decreases
     *
     * @param u
     * @param v
     * @return
     * @throw TODO an exception when nodes rep_u and rep_v are connected
     */
    void merge_nodes(const N_ID &u, const N_ID& v);

    /** Add an edge between the representors of u and v
     *
     * @param u
     * @param v
     * @return
     */
    void split_nodes(const N_ID &u, const N_ID& v);

    /** Initialize the weights of active nodes from a vector
     *
     * @param new_weights
     * @note is used in the Pricing step
     */
    void init_node_weights(const vector<WTYPE>& new_weights);

    /** Initialize the edge lists end the node weights from a file
     *
     * @param filename
     * @throw TODO (and exception if the filename is invalid or the file doesn't respect the DIMACS format)
     */
    void read_dimacs(const string& filename);

    /** Find all nodes that are merged to nodes in the input container
     *
     * @param set
     * @return
     */
    N_CONTAINER recover_all_merged_to(const N_CONTAINER& set);
};

bool is_independent_set(const Graph& G, const N_CONTAINER& IS);

#endif //QUANTUM_BNP_GRAPH_H
