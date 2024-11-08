//
// Created by margarita on 12/12/22.
//

#ifndef QUANTUM_BNP_COLORING_H
#define QUANTUM_BNP_COLORING_H
#include "../Graph.h"
#ifdef QB_ENABLE_SCIP
    #include "scip/scip.h"
#endif

#define TIME_LIMIT 50000 // The upper bound on the Branch & Price runtime

/** Color the nodes of the graph with Branch & Price
 *
 * @param graph
 * @param colors vector to store the colors of nodes
 * @return
 */
SCIP_RETCODE coloringBNP(const Graph& graph, vector<int>& colors);

#endif //QUANTUM_BNP_COLORING_H
