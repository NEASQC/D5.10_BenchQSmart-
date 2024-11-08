//
// Created by margarita on 11/12/22.
//
#ifdef QB_ENABLE_SCIP
#include "ConstraintHandler.h"
#include "Probdata.h"
#include "Vardata.h"

struct SCIP_ConsData {
    int u;
    int v;
    ConsType type; // MERGE or SPLIT

    //Supplementary information that allows to avoid unnecessary efficient propagation
    int n_propagated_vars; // How many variables there were when the constraint was propagated the last time
    bool is_propagated; // Indicates if the constraint was already propagated
    SCIP_NODE* node; // the node to which the constraint is sticking
};

SameDiff recover_constraint_characteristic(SCIP_CONS* cons){
    SCIP_ConsData* consdata = SCIPconsGetData(cons);
    return {consdata->u, consdata->v, consdata->type};
}

SCIP_RETCODE ConstraintHandler::scip_active(SCIP *scip, SCIP_CONSHDLR *conshdlr, SCIP_CONS *cons) {

    Probdata *probdata = dynamic_cast<Probdata *>(SCIPgetObjProbData(scip));
    SCIP_CONSDATA *consdata = SCIPconsGetData(cons);

    // Repropagate the constraint if itwas not propagated to all variables
    if (consdata->n_propagated_vars != probdata->get_nvars()) {
        consdata->is_propagated = false;
        SCIP_CALL(SCIPrepropagateNode(scip, consdata->node));
    }
    return SCIP_OKAY;
}

SCIP_RETCODE ConstraintHandler::scip_deactive(SCIP *scip, SCIP_CONSHDLR *conshdlr, SCIP_CONS *cons) {

    Probdata *probdata = dynamic_cast<Probdata *>(SCIPgetObjProbData(scip));
    SCIP_CONSDATA *consdata = SCIPconsGetData(cons);

    consdata->n_propagated_vars = probdata->get_nvars();
    return SCIP_OKAY;
}

SCIP_RETCODE ConstraintHandler::scip_prop(SCIP *scip, SCIP_CONSHDLR *conshdlr, SCIP_CONS **conss, int nconss, int nusefulconss,
                             int nmarkedconss, SCIP_PROPTIMING proptiming, SCIP_RESULT *result) {

    *result = SCIP_DIDNOTFIND; //Method failed finding anything
    Probdata* probdata = dynamic_cast<Probdata*>(SCIPgetObjProbData(scip));
    vector<SCIP_VAR*> variables = probdata->get_vars();

    SCIP_Bool fixed;
    SCIP_Bool infeasible;
    SCIP_Bool cutoff = FALSE;

    int nfixed = 0;

    //Propagate each constraint in conss if it's not propagated
    for(int i = 0; i< nconss; i++){

        SCIP_CONSDATA * consdata = SCIPconsGetData(conss[i]);
        ConsType type = consdata->type;

        if(! consdata->is_propagated){
            for(int v = consdata->n_propagated_vars; v < variables.size(); v++) {

                //If the variable can't have non-zero value we can scip it
                if( SCIPvarGetUbLocal(variables[v]) < 0.5 )
                    continue;

                Vardata *vardata = dynamic_cast<Vardata *> (SCIPgetObjVardata(scip, variables[v]));
                bool u_covered = vardata->covers(consdata->u);
                bool v_covered = vardata->covers(consdata->v);

                if (type == MERGE && u_covered != v_covered || type == SPLIT && u_covered && v_covered){
                    nfixed++;
                    SCIP_CALL( SCIPfixVar(scip, variables[v], 0.0, &infeasible, &fixed));
                    cutoff = cutoff || infeasible;
                }
            }

            //If the problem becomes unsatisfiable because of the constraint conflict the propagation is stopped
            if( cutoff ) {
                *result = SCIP_CUTOFF;
                break;
            }
            //Otherwise the constraint was successfully propagated
            else {
                consdata->is_propagated = true;
                consdata->n_propagated_vars = variables.size();
                if (nfixed > 0)
                    *result = SCIP_REDUCEDDOM;
            }
        }
    }
    return SCIP_OKAY;
}

SCIP_RETCODE ConstraintHandler::scip_trans(SCIP *scip, SCIP_CONSHDLR *conshdlr, SCIP_CONS *sourcecons, SCIP_CONS **targetcons) {

    //Copies the source constraint to a transformed constraint
    SCIP_CONSDATA * sourcedata = SCIPconsGetData(sourcecons);
    SCIP_CONSDATA * targetdata;
    SCIP_CALL (SCIPallocBlockMemory(scip, &targetdata));
    *targetdata = *sourcedata;

    SCIPcreateCons(scip, targetcons, SCIPconsGetName(sourcecons), conshdlr, targetdata,
                   SCIPconsIsInitial(sourcecons), SCIPconsIsSeparated(sourcecons), SCIPconsIsEnforced(sourcecons),
                   SCIPconsIsChecked(sourcecons), SCIPconsIsPropagated(sourcecons), SCIPconsIsLocal(sourcecons),
                   SCIPconsIsModifiable(sourcecons), SCIPconsIsDynamic(sourcecons), SCIPconsIsRemovable(sourcecons),
                   SCIPconsIsStickingAtNode(sourcecons));
}

SCIP_RETCODE ConstraintHandler::scip_delete(SCIP *scip, SCIP_CONSHDLR *conshdlr, SCIP_CONS *cons, SCIP_CONSDATA **consdata) {
    SCIPfreeBlockMemory(scip, consdata);
    return SCIP_OKAY;
}

SCIP_RETCODE createConsSamediff(SCIP* scip, SCIP_CONS** cons, const char* name, int u, int v, ConsType type, SCIP_NODE* node,SCIP_Bool local){
    SCIP_CONSHDLR * handler = SCIPfindConshdlr(scip, CONSHDLR_NAME);
    SCIP_CONSDATA * consdata;

    SCIP_CALL (SCIPallocBlockMemory(scip, &consdata));
    *consdata = {u, v, type, 0, FALSE, node};

    SCIP_CALL( SCIPcreateCons(scip, cons, name, handler, consdata, FALSE, FALSE, FALSE, FALSE, TRUE,
                              local, FALSE, FALSE, FALSE, TRUE) );

    return SCIP_OKAY;
};
 #endif //   QB_ENABLE_SCIP