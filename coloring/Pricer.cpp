//
// Created by margarita on 11/12/22.
//
#ifdef QB_ENABLE_SCIP
#include <iostream>
#include "scip/cons_setppc.h"

#include "Pricer.h"
#include "Vardata.h"
#include "ConstraintHandler.h"
#include "Probdata.h"


SCIP_RETCODE Pricer::add_branching_constraints() {
    if(branching_accounted)
        throw "Branching constraint was already integrated";

    local_graph = *initial_graph;

    auto constraint_set = SCIPconshdlrGetConss(conshdlr);
    auto n_conss = SCIPconshdlrGetNConss(conshdlr);

    SCIP_CONS* constraint;

    for(int i = 0; i < n_conss; i++){
        constraint = constraint_set[i];

        if(!SCIPconsIsActive(constraint))
            continue;

        SameDiff* cons_data = (SameDiff *) SCIPconsGetData(constraint);

        if(cons_data->type == MERGE)
            local_graph.merge_nodes(cons_data->u, cons_data->v);
        else if(cons_data->type == SPLIT)
            local_graph.split_nodes(cons_data->u, cons_data->v);
        else
            return SCIP_INVALIDDATA;
    }

    branching_accounted = true;
    return SCIP_OKAY;
}

SCIP_RETCODE Pricer::add_MWIS_variable_to_SCIP(SCIP* scip, const N_CONTAINER& independent_set) {
    SCIP_VAR* var;
    ObjVardata* vardata = new Vardata(independent_set); //TODO when free?
    string varname = "l_" + to_string(counter);
    counter++;

    SCIP_CALL(SCIPcreateObjVar(scip, &var, varname.c_str(), 0.0, 1.0, 1.0, SCIP_VARTYPE_BINARY, false, true, vardata, true));

    //Change the upper bound to lazy as the objective function already implies it
    SCIP_CALL( SCIPchgVarUbLazy(scip, var, 1.0));

    //Add the variable to the formulation, modifies the objective function
    SCIP_CALL( SCIPaddPricedVar(scip, var, 1.0));

    //Add the variable to constraints
    for(const auto& u: independent_set)
        SCIP_CALL( SCIPaddCoefSetppc(scip, covering_constraints[u], var));

    SCIP_CALL( SCIPreleaseVar(scip, &var) );

    return SCIP_OKAY;
}

SCIP_DECL_PRICERINIT(Pricer::scip_init){
    SCIP_EVENTHDLR * eventhdlr = SCIPfindEventhdlr(scip, EVENTHDLR_NAME);
    SCIP_CALL( SCIPcatchEvent(scip, SCIP_EVENTTYPE_VARADDED, eventhdlr, NULL, NULL) );

    for(auto & cons : covering_constraints){
        SCIP_CONS * old_cons = cons;

        //release original constraint
        //SCIP_CALL(SCIPreleaseCons(scip, &cons));

        //get the transformed constraint
        SCIP_CALL(SCIPgetTransformedCons(scip, old_cons, &cons));
    }
    return SCIP_OKAY;
}

SCIP_DECL_PRICERREDCOST(Pricer::scip_redcost){
    if(!branching_accounted)
        add_branching_constraints();

    //Initialize weights from dual values

    WTYPE dual;
    local_graph.set_weights_to_zero();
    for(int u = 0; u < covering_constraints.size(); u++){
        dual = SCIPgetDualsolSetppc(scip, covering_constraints[u]);
        local_graph.add_node_weight(u, dual);
    }

    //Find an improving MWIS if it exists

    bool found = false; // becomes true when heuristic or exact method finds an improving variable
    WTYPE cutoff = 1.0 + SCIPepsilon(scip); // If the independent set has a weight > cutoff then it improves the solution

    N_CONTAINER mwis;
    WTYPE mwis_value = 0;

    //Search for an improving independent se(t
    found = greedyMWIS(local_graph, mwis, mwis_value, cutoff);
    if(!found) {
        found = quantumMWIS(local_graph, mwis, mwis_value, cutoff);
        rqaoa_found += found;
//        cout << "RQAOA found an IS of weight: " << mwis_value << endl;
    }
    if(!found){
        found = cplexMWIS(local_graph, mwis, mwis_value, cutoff);
        exact_found += found;
//        cout << "CPLEX found an IS of weight: " << mwis_value << endl;
    }
    if(found) {
        mwis = local_graph.recover_all_merged_to(mwis);
        if (is_independent_set(*initial_graph, mwis))
            add_MWIS_variable_to_SCIP(scip, mwis);
        else
            cout << "The pricing found an infeasible solution";
    }
    else branching_accounted = false; // As the improving variable was not found, the branching occurs before the next execution

    (*result) = SCIP_SUCCESS;
    return SCIP_OKAY;
}
#endif
