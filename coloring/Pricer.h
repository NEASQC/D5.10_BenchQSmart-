//
// Created by margarita on 11/12/22.
//

#ifndef QUANTUM_BNP_PRICER_H
#define QUANTUM_BNP_PRICER_H

#ifdef QB_ENABLE_SCIP
    #include "scip/scip.h"
    #include "objscip/objpricer.h"
#endif

#include "../mwis/mwis.h"


#define PRICER_NAME "MWIS"
#define PRICER_DESC "Find an independent set with reduced cost > 1"
#define PRICER_PRIORITY 0 // If multiple pricers are defined they are called in decreasing priority order, in our code there is only ine pricer
#define PRICER_DELAY true // Pricer is called if all existing variables have negative reduced cost


class Pricer : public scip::ObjPricer {

    int counter; // Used to name priced variables
    const Graph* initial_graph; //doesn't change during execution

    vector<SCIP_CONS*> covering_constraints; //initial constraints in the formulation assuring that each node is covered
    SCIP_CONSHDLR * conshdlr;

    bool branching_accounted; // Is true if we had prepared the correct local graph
    Graph local_graph; // Graph with merged and split vertices defined by branching constraints

    // Logging information
    int rqaoa_found; // how often qaoa manages to find an improving variable
    int exact_found; // how often the exact method finds an improving variable

private:
    /** Modifies the local Pricer graph at each node of the Branch & Bound tree
     *
     * Constructs a local graph with merged and split nodes from the samediff constraints
     * *
     * @param conshdlr
     * @return
     * @throw TODO an error if called when the branching constraints were already accounted
     */
    SCIP_RETCODE add_branching_constraints();

    /** Create and add a variable corresponding to an imrpoving independent set
    *
    * @param independent_set
    * @return
    */
    SCIP_RETCODE add_MWIS_variable_to_SCIP(SCIP* scip, const N_CONTAINER& independent_set);
public:

    Pricer (SCIP* scip, const Graph* _initial_graph, const vector<SCIP_CONS*>& constraints) :
            counter(0), rqaoa_found(0), exact_found(0),
            initial_graph(_initial_graph),
            covering_constraints(constraints),
            conshdlr(SCIPfindConshdlr(scip, "SameDiff")),
            ObjPricer(scip, PRICER_NAME, PRICER_DESC, PRICER_PRIORITY, PRICER_DELAY) {};

    /** Initialize the Pricer after the problem was transformed
     *
     * Resets constraints to the constraints from the transformed problems
     * Transformed problem is a copy of the initial problem created during the solving process
     * All modifications performed by pricing are applied to the transformed problem
     *
     * @param scip
     * @param pricer
     * @return
     */
    SCIP_DECL_PRICERINIT(scip_init) override;

    /** Find a reduced variable of negative cost
     *
     * @param scip
     * @param pricer
     * @param lowerbound
     * @param stopearly
     * @param result refers to SCIP_SUCCESS if an improving variable is found, otherwise is SCIP_DIDNOTRUN
     * @note if an imrpoving variable is not found, set branching_accounted to false
     * @return
     */
    SCIP_DECL_PRICERREDCOST(scip_redcost) override;

    /** The linear program should never become infeasible after the branching
     *
     * If the method is called something went wrong, abort the execution
     *
     * @param scip
     * @param pricer
     * @param result
     * @throw TODO
     * @return
     */
    SCIP_DECL_PRICERFARKAS(scip_farkas) {
        SCIPABORT();
    }

    /** Print the RQAOA success rate*/
    void print_success_rate(){
        cout << "RQAOA success rate is: " << rqaoa_found / (rqaoa_found + exact_found) << endl;
    }
};








#endif //QUANTUM_BNP_PRICER_H
