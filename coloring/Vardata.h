//
// Created by margarita on 11/12/22.
//

#ifndef QUANTUM_BNP_VARDATA_H
#define QUANTUM_BNP_VARDATA_H

#ifdef QB_ENABLE_SCIP
#include "scip/scip.h"
#include "objscip/objvardata.h"


#include "../Graph.h"
using namespace scip;

/** Stores the data related to each variable in formulation
 *
 */
class Vardata : public ObjVardata{
    N_CONTAINER independent_set;
public:
    Vardata(): independent_set(), ObjVardata() {};
    Vardata(const N_CONTAINER& IS): independent_set(IS), ObjVardata() {};
    void set_independent_set(const N_CONTAINER& IS) { independent_set = IS; };

    N_CONTAINER get_independent_set() const { return independent_set; };

    /** Check if an independent set covers the input node
     *
     * @param u
     * @return
     */
    bool covers(int u) const { return independent_set.count(u); };
};
#endif // QB_ENABLE_SCIP
#endif //QUANTUM_BNP_VARDATA_H
