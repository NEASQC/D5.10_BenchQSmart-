//
// Created by margarita on 09/12/22.
//

#ifdef QB_ENABLE_SCIP
#include "objscip/objbranchrule.h"

#ifndef QUANTUM_BNP_BRANCHING_H
#define QUANTUM_BNP_BRANCHING_H

#define BRANCHRULE_NAME            "RyanFoster"
#define BRANCHRULE_DESC            "Ryan-Foster branching rule"
#define BRANCHRULE_PRIORITY        50000 //Set high priority to impose the rule
#define BRANCHRULE_MAXDEPTH        -1 //Apply the branching rule at all nodes (independently of depth)
#define BRANCHRULE_MAXBOUNDDIST    1.0 //Default distance from dual bound to primal bound

using namespace scip;

class Branching : public ObjBranchrule{

public:
    Branching(SCIP* scip): ObjBranchrule(scip, BRANCHRULE_NAME, BRANCHRULE_DESC, BRANCHRULE_PRIORITY, BRANCHRULE_MAXDEPTH, BRANCHRULE_MAXBOUNDDIST){};

    /**A branching method implementing Ryan-Foster Rule
     *
     * @param scip
     * @param branchrule
     * @param allowaddcons
     * @param result
     * @return
     */
    SCIP_DECL_BRANCHEXECLP(scip_execlp) override;
};

#endif
#endif //QUANTUM_BNP_BRANCHING_H
