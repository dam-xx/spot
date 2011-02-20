#ifndef ALGOS_HH_
# define ALGOS_HH_

#include <queue>
#include "builder.hh"
#include "ltlast/atomic_prop.hh"
#include "ltlast/unop.hh"
#include "ltlenv/defaultenv.hh"
#include "tgba/tgbaproduct.hh"
#include "tgba/tgbaunion.hh"
#include "tgba/tgbasafracomplement.hh"
#include "tgbaalgos/emptiness.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/ltl2tgba_lacim.hh"
#include "tgbaalgos/randomgraph.hh"
#include "tgbaalgos/scc.hh"
#include "tgbaalgos/setdotdec.hh"
#include "tgbaalgos/cutscc.hh"
#include "misc/random.hh"

tgba_map* build_tgba_map(const spot::ltl::formula* f,
                         spot::bdd_dict* dict,
                         std::list<std::string>& algos);

void free_tgba_map(tgba_map* m);

bool check_intersection(tgba_map* m,
                        spot::emptiness_check_instantiator* inst);

bool check_cross_comparison(tgba_map* m,
                            spot::tgba* model,
                            spot::emptiness_check_instantiator* inst);

bool check_consistency(tgba_map* m,
                       spot::tgba* model);

unsigned tgba_size(const spot::tgba* a);

spot::state_set* project_accepting_states(spot::tgba* prod,
                                          spot::tgba* model);

void delete_state_set(spot::state_set* s);

#endif /* !ALGOS_HH_ */
