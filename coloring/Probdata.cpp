//
// Created by margarita on 11/12/22.
//
#ifdef QB_ENABLE_SCIP
#include <scip/cons_setppc.h>
#include "../Graph.h"
#include "Probdata.h"
#include "Vardata.h"
#include "algorithm"




vector<int> dsatur(const Graph* graph, int& n_colors)
{
    n_colors = 0;
    int node_number = graph->get_node_number();
    vector<int> candidate_color(node_number, 0); // All colors smaller than candidate color are assumed to be unavailable for the node

    vector<N_CONTAINER> forbidden_colors(node_number); // Colors greater than candidate_color[u] that are forbidden for the node u

    vector<int> sat_degree(node_number, 0); // Saturation degree - how many neighbors of the node are already colored

    N_CONTAINER uncolored;
    for(N_ID u = 0; u < node_number; u++) uncolored.insert(u);

    //A comparator that allows at each step to select a node with maximal saturated degree, ties are broken in favor of nodes with higher degrees in the graph
    auto comp = [&sat_degree, &graph](N_ID u, N_ID v){
        if (sat_degree[u] < sat_degree[v])
            return true;
        else if (sat_degree[u] == sat_degree[v])
            return graph->get_degree(u) < graph->get_degree(v);
        return false;
    };

    while(!uncolored.empty())
    {
        auto node_to_color = std::max_element(uncolored.begin(), uncolored.end(), comp);
        int used_color = candidate_color[*node_to_color];
        if(used_color > n_colors)
            n_colors = used_color;

        auto neighbors = graph->get_neighbors(*node_to_color);
        for(const auto & u : neighbors)
        {
            if(uncolored.count(u))
            {
                sat_degree[u]++;
                if (candidate_color[u] > used_color)
                    continue;
                if (candidate_color[u] < used_color)
                {
                    forbidden_colors[u].insert(used_color);
                    continue;
                }
                if (candidate_color[u] == used_color)
                    candidate_color[u]++;
                auto it = forbidden_colors[u].begin();
                while(it != forbidden_colors[u].end() && candidate_color[u] == *it)
                {
                    it = forbidden_colors[u].erase(it);
                    candidate_color[u]++;
                }
            }
        }
        uncolored.erase(node_to_color);
    }
    n_colors++;
    return candidate_color;
}

SCIP_RETCODE Probdata::initialize_cons(Scip *scip) {

    string cons_name = "node_";
    for(int u = 0; u < initial_graph->get_node_number(); u++){
        string cons_name = "node_" + to_string(u);
        SCIP_CONS* cover_cons;

        SCIP_CALL( SCIPcreateConsBasicSetcover(scip, &cover_cons, cons_name.c_str(), 0, NULL));
        SCIP_CALL( SCIPsetConsModifiable(scip, cover_cons, TRUE));
        SCIP_CALL( SCIPaddCons(scip, cover_cons));
        covering_constraints.push_back(cover_cons);

        SCIP_CALL( SCIPreleaseCons(scip, &cover_cons));
    }
    return SCIP_OKAY;
}

SCIP_RETCODE Probdata::initialize_vars(Scip *scip) {
    int n_colors;
    vector<int> dsatur_coloring = dsatur(initial_graph, n_colors);

    for(int c = 0; c < n_colors; c++){
        string var_name = "in_var_" + to_string(c);
        SCIP_VAR * var;
        Vardata* vardata = new Vardata(); //TODO when free?

        SCIP_CALL( SCIPcreateObjVar(scip, &var, var_name.c_str(), 0.0, 1.1, 1.0, SCIP_VARTYPE_BINARY, true, true, vardata, true));
        SCIP_CALL( SCIPchgVarUbLazy(scip, var, 1.0)); //The upper bound is already enforced by the objective function

        SCIP_CALL( SCIPaddVar(scip, var));

        //Add the variable to constraints corresponding to nodes covered by it
        N_CONTAINER c_set;
        for(int u = 0; u < initial_graph->get_node_number(); u++){
            if (dsatur_coloring[u] == c) {
                SCIP_CALL(SCIPaddCoefSetppc(scip, covering_constraints[u], var));
                c_set.insert(u);
            }
        }
        vardata->set_independent_set(c_set); //Add the variable data

        variables.push_back(var);

        SCIP_CALL( SCIPreleaseVar(scip, &var));
    }
}
Probdata::Probdata(const Graph* graph): ObjProbData() {
    initial_graph = new Graph(*graph);
};

SCIP_RETCODE Probdata::scip_trans(SCIP *scip, ObjProbData **objprobdata, unsigned int *deleteobject) {
    Probdata* transdata = new Probdata(initial_graph);

    // for each variable push its transformed version to the transformed problem data
    transdata->variables.clear();

   for(auto& orig_var: variables){
        SCIP_VAR * transvar;
        SCIPtransformVar(scip, orig_var, &transvar);
        transdata->variables.push_back(transvar);
        SCIPreleaseVar(scip, &transvar);
    }

    //for each constraint push its transformed version to the transformed problem data
    SCIP_CONS * transcons;
    for (auto& orig_cons : covering_constraints){

        SCIPtransformCons(scip, orig_cons, &transcons);
        transdata->covering_constraints.push_back(transcons);
    }

    *deleteobject = FALSE; //TODO
    *objprobdata = transdata;
    return SCIP_OKAY;
}

SCIP_RETCODE Probdata::scip_deltrans(SCIP *scip) {
    for(auto & var: variables) {
        SCIP_CALL(SCIPreleaseVar(scip, &var));
    }

    return SCIP_OKAY;
}

SCIP_RETCODE Probdata::scip_delorig(SCIP *scip) {
    for(auto & var: variables)
        SCIP_CALL(SCIPreleaseVar(scip, &var));

    return SCIP_OKAY;
}

SCIP_RETCODE EventAddedVar::scip_exec(SCIP *scip, SCIP_EVENTHDLR *eventhdlr, SCIP_EVENT *event, SCIP_EVENTDATA *eventdata) {
    Probdata* probdata = dynamic_cast<Probdata*>(SCIPgetObjProbData(scip));
    SCIP_VAR* var_to_add = SCIPeventGetVar(event);
    probdata->addVar(scip, var_to_add);

    return SCIP_OKAY;
}

#endif //QB_ENABLE_SCIP