//
// Created by margarita on 12/12/22.
//
#include <iostream>
#ifdef  QB_ENABLE_SCIP
#include "coloring.h"
#include "Probdata.h"
#include "ConstraintHandler.h"
#include "Branching.h"
#include "Pricer.h"
#include "scip/scipdefplugins.h"
#include "Vardata.h"


SCIP_RETCODE coloringBNP(const Graph& graph, vector<int>& colors){
    SCIP * scip = NULL;
    SCIP_CALL( SCIPcreate(&scip) );

    Probdata* probdata = new Probdata(&graph);
    SCIP_CALL( SCIPcreateObjProb(scip, "COLORING", probdata, true));

    SCIP_CALL ( SCIPincludeDefaultPlugins(scip));

    probdata->initialize_cons(scip);
    probdata->initialize_vars(scip);

    ConstraintHandler* conshdlr = new ConstraintHandler(scip);
    SCIP_CALL( SCIPincludeObjConshdlr(scip, conshdlr, true));

    Branching* branching = new Branching(scip);
    SCIP_CALL( SCIPincludeObjBranchrule(scip, branching, true));

    //Event handler for addition of variable
    EventAddedVar* eventhdlr = new EventAddedVar(scip);
    SCIP_CALL( SCIPincludeObjEventhdlr(scip, eventhdlr, false));

    Pricer* pricer = new Pricer(scip, &graph, probdata->get_cons());
    SCIP_CALL( SCIPincludeObjPricer(scip, pricer, true));
    SCIP_CALL ( SCIPactivatePricer(scip, SCIPfindPricer(scip, PRICER_NAME))); //Activates the pricer used in solution, deactivation is automatic

    // set the time limit
    SCIP_CALL( SCIPsetRealParam(scip, "limits/time", TIME_LIMIT) );

    // set verbosity level
    SCIP_CALL( SCIPsetIntParam(scip,"display/verblevel",0));

    // for column generation instances, disable restarts
    SCIP_CALL( SCIPsetIntParam(scip,"presolving/maxrestarts",0) );

    // turn off all separation algorithms
    SCIP_CALL( SCIPsetSeparating(scip, SCIP_PARAMSETTING_OFF, TRUE) );

    // set objective sense
    SCIP_CALL( SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE) );

    // tell SCIP that the objective will be always integral
    SCIP_CALL( SCIPsetObjIntegral(scip) );

    SCIP_CALL( SCIPsolve(scip) );

    SCIP_SOL * scip_solution = SCIPgetBestSol(scip);

    if (! scip_solution )
        throw "Solution not found!";
    else{
        int n_colors = 0;


        //New variables are added only to a transformed problem, therefore probdata doesn't refer to an updated set of variables
        Probdata* trans_probdata = dynamic_cast<Probdata*>(SCIPgetObjProbData(scip));
        vector<SCIP_VAR*> variables = trans_probdata->get_vars();

        //Recover solution
        for(auto& var: variables){
            SCIP_Real value = SCIPgetSolVal(scip, scip_solution, var);
            if (SCIPisEQ(scip, value, 1.0)){
                Vardata* vardata = dynamic_cast<Vardata*>(SCIPgetObjVardata(scip, var));
                for(const auto & u: vardata->get_independent_set())
                    colors[u] = n_colors;
                n_colors++;
            }
        }
 /*       for(int i = 0; i < variables.size(); i++){
            SCIPreleaseVar(scip, &variables[i]);
        }*/
        cout << "Branch & Price found a coloring with " << n_colors << " colors." << endl;
        pricer->print_success_rate();
    }

    SCIPfree(&scip);
    return SCIP_OKAY;
};

#endif // QB_ENABLE_SCIP