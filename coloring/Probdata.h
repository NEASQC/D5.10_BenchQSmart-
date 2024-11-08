//
// Created by margarita on 11/12/22.
//
#ifdef QB_ENABLE_SCIP
#include "../Graph.h"
#ifndef QUANTUM_BNP_PROBDATA_H
#define QUANTUM_BNP_PROBDATA_H

#include "ConstraintHandler.h"
#include "scip/scip.h"
#include "objscip/objprobdata.h"
#include "objscip/objeventhdlr.h"

#define EVENTHDLR_NAME         "Added Variable"
#define EVENTHDLR_DESC         "Event handler for catching added variables"


using namespace scip;

class Probdata : public ObjProbData {
    vector<SCIP_VAR*> variables;
    vector<SCIP_CONS*> covering_constraints;
    Graph* initial_graph;


public:
    Probdata(const Graph* graph);
    ~Probdata() { delete initial_graph; };

    /** Create covering constraint for each node in the initial_graph
     *
     * @param scip
     * @return
     */
    SCIP_RETCODE initialize_cons(Scip* scip);

    /** Add initial variables that assures the existence of a feasible solution
     *
     * Creates and adds initial variables to constrains. Uses DSATUR greedy heuristic to compute colors.
     *
     * @param scip
     * @return
     */
    SCIP_RETCODE initialize_vars(Scip* scip);

    Graph* get_graph() const { return initial_graph; };
    int get_node_number() const { return initial_graph->get_node_number(); };
    int get_nvars() const { return variables.size(); };
    vector<SCIP_VAR*> get_vars() const { return variables; };
    vector<SCIP_CONS*> get_cons() const { return covering_constraints; };

    //TODO
    /** Initialize the transformed problem from the initial problem
     *
     * The method resets variables and constraints to transformed ones
     *
     * @param scip
     * @param objprobdata where transformed problem is stored
     * @param deleteobject If TRUE
     * @return
     */
    SCIP_RETCODE scip_trans(SCIP* scip, ObjProbData** objprobdata, SCIP_Bool* deleteobject) override;

    /** Add a newly created variable to probdata
     *
     * @param var_to_add
     */
    SCIP_RETCODE addVar(Scip* scip, SCIP_VAR* var_to_add) {
        SCIP_CALL( SCIPcaptureVar(scip, var_to_add) );
        variables.push_back(var_to_add);

        return SCIP_OKAY;
    };

    SCIP_RETCODE scip_deltrans(SCIP *scip) override;

    SCIP_RETCODE scip_delorig(SCIP *scip) override;
};

class EventAddedVar : public ObjEventhdlr{

public:
    /** If any method (Pricer or initialization method) creates a variable it is added to the probdata
     *
     * @param scip
     */
    EventAddedVar(SCIP* scip): ObjEventhdlr( scip, EVENTHDLR_NAME, EVENTHDLR_DESC){};
    SCIP_DECL_EVENTEXEC(scip_exec) override;
};

/** Dsatur greedy coloring method
 *
 * See https://www.geeksforgeeks.org/dsatur-algorithm-for-graph-coloring/ for details
 *
 * @param graph
 * @param ncolors
 * @return a vector with colors
 * @note is called to obtain initial set of variables for which a feasble solution exist
 */
vector<int> dstatur(const Graph* graph, int& ncolors);

#endif
#endif //QUANTUM_BNP_PROBDATA_H
