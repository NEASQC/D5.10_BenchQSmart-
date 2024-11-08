//
// Created by margarita on 11/12/22.
//

#ifndef QUANTUM_BNP_CONSTRAINTHANDLER_H
#define QUANTUM_BNP_CONSTRAINTHANDLER_H

#ifdef QB_ENABLE_SCIP

#include "scip/scip.h"
#include "objscip/objconshdlr.h"


#define CONSHDLR_NAME          "SameDiff"
#define CONSHDLR_DESC          "stores the local branching decisions"
#define CONSHDLR_SEPAPRIORITY         0 // priority of the constraint handler for separation (never called)
#define CONSHDLR_ENFOPRIORITY         0 // priority of the constraint handler for constraint enforcing (goes after the set covering)
#define CONSHDLR_CHECKPRIORITY  9999999 // priority of the constraint handler for checking feasibility (the check is trivial)
#define CONSHDLR_SEPAFREQ             -1 // frequency for spearation (constraint not used)
#define CONSHDLR_PROPFREQ             1 // frequency for propagating domains
#define CONSHDLR_EAGERFREQ            1 // frequency for using all instead of only the useful constraints in separation
#define CONSHDLD_MAXPREROUND          1 // maximal number of presolving rounds the constraint handler participates in
#define CONSHDLR_DELAYSEPA        FALSE // should separation method be delayed,
#define CONSHDLR_DELAYPROP        FALSE // should propagation method be delayed, if other propagators found reductions?
#define CONSHDLR_NEEDSCONS         TRUE // should the constraint handler be skipped, if no constraints are available?
#define CONSHDLR_PROP_TIMING       SCIP_PROPTIMING_BEFORELP //Propagate the constraint before the relaxation is solved
#define CONSHDLR_PRESOLTIMING      SCIP_PRESOLTIMING_EXHAUSTIVE

using namespace scip;

enum ConsType
{
    SPLIT = 0,                               //corresponding nodes should be in different independent sets
    MERGE   = 1                              //corresponding nodes should be in the same independent set
};

using SameDiff = struct SameDiff {
    int u;
    int v;
    ConsType type;
};

class ConstraintHandler : public ObjConshdlr{

public:
    ConstraintHandler(Scip* scip) : ObjConshdlr(scip, CONSHDLR_NAME, CONSHDLR_DESC,
                                                CONSHDLR_SEPAPRIORITY, CONSHDLR_ENFOPRIORITY, CONSHDLR_CHECKPRIORITY,
                                                CONSHDLR_SEPAFREQ, CONSHDLR_PROPFREQ, CONSHDLR_EAGERFREQ,
                                                CONSHDLD_MAXPREROUND,
                                                CONSHDLR_DELAYSEPA, CONSHDLR_DELAYPROP,
                                                CONSHDLR_NEEDSCONS,
                                                CONSHDLR_PROP_TIMING, CONSHDLR_PRESOLTIMING) {};

    /** Activated the constraint
     *
     * if the constraint is not already propagated to all variables repropagates it
     *
     * @param scip
     * @param conshdlr
     * @param cons
     * @return
     */
    virtual SCIP_DECL_CONSACTIVE(scip_active) override;

    /** Activate the constraint
     *
     * If it is not propagated to all existing variables - launch the propagation
     *
     * @param scip
     * @param conshdlr
     * @param cons
     * @return
     */
    virtual SCIP_DECL_CONSDEACTIVE(scip_deactive) override;

    /** Domain propagation method
     *
     * @param scip
     * @param conshdlr
     * @param conss
     * @param nconss
     * @param nusefulconss
     * @param nmarkedconss
     * @param proptiming
     * @param result store the result of propagation, which may be the domain reduction (SCIP_REDUCEDDOM), the insatisfability (SCIP_CUTOFF)
     * @return
     */
    virtual SCIP_DECL_CONSPROP(scip_prop) override;

    /**Transform constraint data into data from transformed problem
     *
     * @param scip
     * @param conshdlr
     * @param sourcecons
     * @param targetcons
     * @return
     */
    virtual SCIP_DECL_CONSTRANS(scip_trans) override;

    //Methods are nor used in for the Handler, however, the definition is needed as they are virtual
    virtual SCIP_DECL_CONSDELETE(scip_delete) override;
    virtual SCIP_DECL_CONSCHECK(scip_check) override {  *result = SCIP_FEASIBLE; return SCIP_OKAY;};
    virtual SCIP_DECL_CONSENFOLP(scip_enfolp) override { *result = SCIP_FEASIBLE; return SCIP_OKAY;};
    virtual SCIP_DECL_CONSENFOPS(scip_enfops) override { *result = SCIP_FEASIBLE; return SCIP_OKAY;};
    virtual SCIP_DECL_CONSLOCK(scip_lock) override { return SCIP_OKAY;};
    virtual SCIP_DECL_CONSSEPASOL(scip_sepasol) override { return SCIP_OKAY;};

};

SameDiff recover_constraint_characteristic(SCIP_CONS* cons);


/** Creates a new SameDiff constraint
 *
 * @param scip
 * @param cons pointer to add constraint
 * @param name
 * @param u first node
 * @param V second node
 * @param type MERGE of SPLIT
 * @param node node that stores the constraint
 * @param local
 * @return
 */
SCIP_RETCODE createConsSamediff(SCIP* scip, SCIP_CONS** cons, const char* name, int u, int v, ConsType type, SCIP_NODE* node,SCIP_Bool local);
#endif //QB_ENABLE_SCIP
#endif //QUANTUM_BNP_CONSTRAINTHANDLER_H
