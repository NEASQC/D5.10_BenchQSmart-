//
// Created by margarita on 09/12/22.
//
#ifdef QB_ENABLE_SCIP
#include "Branching.h"
#include "Vardata.h"
#include "Probdata.h"
#include "ConstraintHandler.h"

using NodePair = struct nodePair{
    int u;
    int v;
};

NodePair find_branching_constraint(Scip* scip, SCIP_VAR** fractional_vars, SCIP_Real* fractional_values, int nfractional, int node_number){

    vector<vector<SCIP_Real>> pair_values(node_number, vector<WTYPE>(node_number, 0)); // An array containing sum of fractional

    //Fill in the table
    for(int i = 0; i < nfractional; i++){

        SCIP_Real val = fractional_values[i];

        //Get the corresponding independent set
        Vardata* vardata = dynamic_cast<Vardata*>(SCIPgetObjVardata(scip, fractional_vars[i]));
        N_CONTAINER independent_set = vardata->get_independent_set();

        for(const auto& u : independent_set){
            for(const auto& v: independent_set)
                pair_values[u][v] += val;
        }
    }

    //Find the branching nodes in the graph
    NodePair branching_pair = {-1, 1};
    SCIP_Real value, bestvalue = 0;

    for(int i = 0; i < node_number; i++)
        for(int j = i+1; j < node_number; j++){

            //Measure the "fractionality" of the pair interaction
            value = MIN(pair_values[i][j], 1 - pair_values[i][j]);

            if(value > bestvalue)
                //Check if one node i (or j) is not precisely covered by a subset of variables all covering the variable j (or i)
                //Avoids getting an unfeasible problem or merging already merged nodes
                if(!SCIPisEQ(scip, pair_values[i][j], pair_values[i][i]) && !SCIPisEQ(scip, pair_values[i][j], pair_values[j][j]) )
                {
                    bestvalue = value;
                    branching_pair = {i, j};
                }
        }

    return branching_pair;
}

SCIP_RETCODE Branching::scip_execlp(SCIP *scip, SCIP_BRANCHRULE *branchrule, unsigned int allowaddcons, SCIP_RESULT *result) {

    *result = SCIP_DIDNOTRUN;

    //get the problem data
    Probdata* probdata = dynamic_cast<Probdata*>(SCIPgetObjProbData(scip));

    SCIP_VAR** fractional_vars; //stores fractional variables
    SCIP_Real* fractional_values; //stores values of non-integral variables

    int n_fractional; // number of fractional values with maximal priority

    SCIP_CALL(SCIPgetLPBranchCands(scip, &fractional_vars, NULL, &fractional_values, NULL, &n_fractional, NULL));

    NodePair branching_pair = find_branching_constraint(scip, fractional_vars, fractional_values, n_fractional, probdata->get_node_number());

    SCIP_NODE *childsame, *childdiffer;
    SCIP_CONS *conssame, *consdiffer;

    // create the branch-and-bound tree child nodes of the current node
    SCIP_CALL( SCIPcreateChild(scip, &childsame, 0.0, SCIPgetLocalTransEstimate(scip)) );
    SCIP_CALL( SCIPcreateChild(scip, &childdiffer, 0.0, SCIPgetLocalTransEstimate(scip)) );

    // create corresponding constraints
    SCIP_CALL( createConsSamediff(scip, &conssame, "same", branching_pair.u, branching_pair.v, MERGE, childsame, TRUE) );
    SCIP_CALL( createConsSamediff(scip, &consdiffer, "differ", branching_pair.u, branching_pair.v, SPLIT, childdiffer, TRUE) );

    // add constraints to nodes
    SCIP_CALL( SCIPaddConsNode(scip, childsame, conssame, NULL) );
    SCIP_CALL( SCIPaddConsNode(scip, childdiffer, consdiffer, NULL) );

    // release constraints
    SCIP_CALL( SCIPreleaseCons(scip, &conssame) );
    SCIP_CALL( SCIPreleaseCons(scip, &consdiffer) );

    *result = SCIP_BRANCHED;

    return SCIP_OKAY;


}
#endif // QB_ENABLE_SCIP   