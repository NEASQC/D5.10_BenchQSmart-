#include <iostream>
#include "Graph.h"

#include "mwis/mwis.h"


#ifdef QB_ENABLE_SCIP
    #include "coloring/coloring.h"
#endif

/** print the independent set in found by one of methods
 *
 * @param method heuristic or exact approach used to find the independent set
 * @param value
 * @param independent_set
 */
void print_mwis_result(const string& method, const WTYPE& value, const N_CONTAINER& independent_set){
    cout << method << " returned an independent set of value: " << value << endl;
    cout << "The set is: ";
    for(const auto& u: independent_set)
        cout << u << " ";
    cout << "\n" << endl;
    return;
}


//TODO Throw exceptions on invalid input
int main(int argc, char* argv[]) {
    if(argc < 3) {
        cout << "Wrong command arguments. Minimum number of arguments = 3";
        return 1;
    }

    string problem_name(argv[1]);
    string instance_file(argv[2]);

    Graph graph;
    graph.read_dimacs(instance_file);

    if(problem_name == "-MWIS")
    {
        if(argc < 4) {
            cout << "Wrong command arguments for MWIS problem, minimum number of argument = 4 \n";
        return 1;
        }
        string method(argv[3]);
        WTYPE weight = 0;
        WTYPE cutoff = INF;
        N_CONTAINER independent_set;

        if(method == "-greedy") {
            greedyMWIS(graph, independent_set, weight, cutoff);
        }
        if(method == "-quantum") {
            quantumMWIS(graph, independent_set, weight, cutoff);
        }
        #ifdef QB_ENABLE_CPLEX
        if(method == "-cplex") {
            cplexMWIS(graph, independent_set, weight, cutoff);
        }
        #endif
        /*
        if(method == "-sewell") {
            sewellMWIS(graph, independent_set, weight, cutoff);
        }*/

        cout << "MWIS is independent: " <<  is_independent_set(graph, independent_set) << endl;
        print_mwis_result(method, weight, independent_set);
    }
    
    if(problem_name == "-COLORING")
    {
        #ifdef QB_ENABLE_SCIP
        vector<int> colors(graph.get_node_number(), 0);
        coloringBNP(graph, colors);

        cout << "COLORING:";
        for(const auto &u: colors)
            cout << " " << u;
        cout << endl;
        #else 
            std::cout << "WARNING you need to install and activate SCIP to use coloring problem" << endl;  
        #endif

    }

    return 0;
}
