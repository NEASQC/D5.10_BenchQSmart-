//
// Created by margarita on 10/12/22.
//
#ifdef QB_ENABLE_CPLEX
#include <ilcplex/ilocplex.h>
#include "mwis.h"

ILOSTLBEGIN

bool cplexMWIS(const Graph& G, N_CONTAINER& best_mwis, WTYPE & best_mwis_value, const WTYPE & cutoff)
{
    auto active_in_graph = G.get_active_nodes();
    vector<int> node_id(active_in_graph.begin(), active_in_graph.end());

    IloEnv env;
    IloModel model(env);
    IloCplex cplex(model);

    IloBoolVarArray vars(env, active_in_graph.size());
    IloExpr obj_function(env);

    model.add(vars);

    for(int i = 0; i < node_id.size(); i++) {

        //add one constraint per edge to the model
        for (int j = i + 1; j < node_id.size(); j++)
            if (G.has_edge(node_id[i], node_id[j])) {
                IloRange edge_cons(env, 0, 1);
                edge_cons.setExpr(vars[i] + vars[j]);
                //add constraint
                model.add(edge_cons);
            }

        //add variable to the objective function
        obj_function += G.get_node_weight(node_id[i]) * vars[i];
    }

    IloObjective obj;
    if(G.is_weighted())
        obj = IloMaximize(env, obj_function);
    else
        obj = IloMaximize(env, IloSum(vars));

    model.add(obj);

    //Remove console logging
    cplex.setOut(env.getNullStream());

    cplex.solve();

    //Recover the solution
    N_CONTAINER cplex_IS;
    WTYPE cplex_IS_weight = 0;

    for(int i = 0; i < node_id.size(); i++){
        auto val = cplex.getValue(vars[i]);
        if(val > EPSILON){
            cplex_IS.insert(node_id[i]);
            cplex_IS_weight += G.get_node_weight(node_id[i]);
        }
    }
    if(cplex_IS_weight > best_mwis_value){
        best_mwis_value = cplex_IS_weight;
        best_mwis = cplex_IS;
    }
    env.end();
    return cplex_IS_weight > cutoff;
}
#endif