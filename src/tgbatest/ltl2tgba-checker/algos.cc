#include "algos.hh"

tgba_map* build_tgba_map(const spot::ltl::formula* f,
                         spot::bdd_dict* dict,
                         std::list<std::string>& algos)
{
  spot::ltl::formula* f_neg;
  f_neg = spot::ltl::unop::instance(spot::ltl::unop::Not, f->clone());
  tgba_map* res = new tgba_map;

  // Function dispatch will be handled by ltl_translator_instantiator.
  std::list<std::string>::iterator it;
  for (it = algos.begin(); it != algos.end(); ++it)
  {
    if (*it == "fm")
    {
      spot::tgba* fm_prop = ltl_to_tgba_fm(f, dict);
      spot::tgba* fm_prop_neg = ltl_to_tgba_fm(f_neg, dict);
      tgba_pair fm_pair(fm_prop, fm_prop_neg);
      (*res)["fm"] = fm_pair;
    }
    else if (*it == "lacim")
    {
      spot::tgba* lacim_prop = ltl_to_tgba_lacim(f, dict);
      spot::tgba* lacim_prop_neg = ltl_to_tgba_lacim(f_neg, dict);
      tgba_pair lacim_pair(lacim_prop, lacim_prop_neg);
      (*res)["lacim"] = lacim_pair;
    }
    else if (*it == "taa")
    {

      // FIXME: when using TAA, there are no acceptance conditions on the
      // resulting TGBA.
      std::cerr << "Do not use TAA for now." << std::endl;
      exit(1);

/*    spot::tgba* taa_prop = ltl_to_taa(f, dict, false);
      spot::tgba* taa_prop_neg = ltl_to_taa(f_neg, dict, false);
      tgba_pair taa_pair(taa_prop, taa_prop_neg);
      (*res)["taa"] = taa_pair;
      */
    }
  }
  f_neg->destroy();
  return res;
}


// Free a tgba_map.
void free_tgba_map(tgba_map* m)
{
  tgba_map::iterator it;
  for (it = m->begin(); it != m->end(); ++it)
  {
    tgba_pair& p = it->second;
    delete p.first;
    delete p.second;
  }
  delete m;
}

// 'Emptiness check for the intersection of two Buchi automata' as described in
// [TauHel02].
// Given a LTL formula p and its negation !p, the assertion:
// L(p) inter L(!p) = empty_set must hold.
// To check the ltl to tgba translator, we compute the Buchi automata Ap and
// A!p.
// If L(Ap inter A!p) is not empty, then either Ap or A!p does not correspond
// to p or !p.
bool check_intersection(tgba_map* m,
                        spot::emptiness_check_instantiator* inst)
{
  bool res = true;
  tgba_map::iterator it1;
  tgba_map::iterator it2;
  for (it1 = m->begin(); it1 != m->end(); ++it1)
    for (it2 = m->begin(); it2 != m->end(); ++it2)
    {
      tgba_pair& p1 = it1->second;
      tgba_pair& p2 = it2->second;
      // Compute Ap inter A!p.
      spot::tgba_product* prod = new spot::tgba_product(p1.first, p2.second);
      spot::emptiness_check* ec = inst->instantiate(prod);
      spot::emptiness_check_result* acc = ec->check();
      bool is_empty = (acc == 0);
      delete ec;
      delete prod;
      delete acc;
      // If the intersection is not empty, one of the translator failed.
      if (!is_empty)
        return false;
    }
  return res;
}

// 'Model checking result cross-comparison test' as described in [TauHel02].
// Given a formula p, the result of model checking must be the same with
// all translation algorithms.
// For each product, compute the set of accepting states in the
// model, this set must be the same for all translation algorithms.
bool check_cross_comparison(tgba_map* m,
                            spot::tgba* model,
                            spot::emptiness_check_instantiator* inst)
{
  bool res = true;
  bool ref_N = true;
  bool ref_P = true;
  bool current_N = true;
  bool current_P = true;
  tgba_map::iterator it = m->begin();

  // Compute the first state of accepting states as a reference.
  tgba_pair& p = it->second;
  spot::tgba_product* prod_P = new spot::tgba_product(p.first, model);
  spot::emptiness_check* ec_P = inst->instantiate(prod_P);
  spot::emptiness_check_result* refe_P = ec_P->check ();
  spot::tgba_product* prod_N = new spot::tgba_product(p.second, model);
  spot::emptiness_check* ec_N = inst->instantiate(prod_N);
  spot::emptiness_check_result* refe_N = ec_N->check ();

  // For now, We just check the nullity or not of the emptiness_check.
  ref_N = (0 == refe_N);
  delete ec_N;
  delete prod_N;
  delete refe_N;
  ref_P = (0 == refe_P);
  delete ec_P;
  delete prod_P;
  delete refe_P;

  it++;

  for (; it != m->end(); it++)
  {
    p = it->second;
    prod_P = new spot::tgba_product(p.first, model);
    ec_P = inst->instantiate(prod_P);
    refe_P = ec_P->check();
    prod_N = new spot::tgba_product(p.second, model);
    ec_N = inst->instantiate(prod_N);
    refe_N = ec_N->check();

    current_N = (0 == refe_N);
    delete ec_N;
    delete prod_N;
    delete refe_N;
    current_P = (0 == refe_P);
    delete ec_P;
    delete prod_P;
    delete refe_P;

    if (current_N != ref_N)
    {
      res = false;
      break;
    }
    if (current_P != ref_P)
    {
      res = false;
      break;
    }

  }

  return res;
}


// 'Model checking result consistency check' as described in [TauHel02].
// Given a formula p, Ap is the resulting Buchi automaton with a given
// translation algorithm.
// Compute the product of the model with Ap and the product
// of the model with A!p.
// For each product, compute the sets Sp and S!p of accepting states in
// the model.  At the end, all states of the model must be either in
// Ap or A!p, i.e Sp U S!p = S (all the states of the model).
bool check_consistency(tgba_map* m,
                       spot::tgba* model)
{
  tgba_map::iterator it;
  for (it = m->begin(); it != m->end(); ++it)
  {
    tgba_pair& p = it->second;
    spot::tgba_product* prod = new spot::tgba_product(p.first, model);
    spot::tgba_product* prod_neg = new spot::tgba_product(p.second, model);
    spot::state_set* acc_states = project_accepting_states(prod, model);
    spot::state_set* acc_states_neg = project_accepting_states(prod_neg, model);
    /*
    // Commented: code to color nodes according to their set.
    std::vector<spot::state_set*> v_set;
    v_set.push_back(acc_states);
    v_set.push_back(acc_states_neg);
    spot::set_dotty_decorator dec(&v_set);
    spot::dotty_reachable(std::cout, model, &dec);
    exit(1);
    */
    spot::state_set* states_union = new spot::state_set;
    // Compute the union Sp U S!p.
    set_union(acc_states->begin(), acc_states->end(),
              acc_states_neg->begin(), acc_states->end(),
              std::inserter(*states_union, states_union->begin()));
    unsigned union_size = states_union->size();
    delete prod;
    delete prod_neg;
    delete_state_set(acc_states);
    delete_state_set(acc_states_neg);
    delete states_union;
    // We check is Sp U S!p = S by simply comparing the sizes.
    if (union_size != tgba_size(model))
      return false;
  }
  return true;
}


// Compute the size of a TGBA (number of state), through a
// breadth-first traversal.
unsigned tgba_size(const spot::tgba* a)
{
  spot::state_set seen;
  std::queue<spot::state*> tovisit;
  // Perform breadth-first traversal.
  spot::state* init = a->get_init_state();
  tovisit.push(init);
  seen.insert(init);
  unsigned count = 0;
  // While there are still states to visit.
  while (!tovisit.empty())
  {
    ++count;
    spot::state* cur = tovisit.front();
    tovisit.pop();
    spot::tgba_succ_iterator* sit = a->succ_iter(cur);
    for (sit->first(); !sit->done(); sit->next())
    {
      spot::state* dst = sit->current_state();
      // Is it a new state ?
      if (seen.find(dst) == seen.end())
      {
        // Yes, register the successor for later processing.
        tovisit.push(dst);
        seen.insert(dst);
      }
      else
        // No, free dst.
        delete dst;
    }
    delete sit;
  }
  spot::state_set::iterator it2;
  // Free visited states.
  for (it2 = seen.begin(); it2 != seen.end(); it2++)
    delete *it2;
  return count;
}

// Return a set containing all the accepting states in model.
// For each accepting SCC in the product, the states are projected in model.
spot::state_set* project_accepting_states(spot::tgba* prod,
                                    spot::tgba* model)
{
  spot::state_set* res = new spot::state_set;
  spot::scc_map scc_prod (prod);
  scc_prod.build_map();
  typedef std::list<const spot::state*> state_list;
  for (unsigned i = 0; i < scc_prod.scc_count(); ++i)
  {
    if (scc_prod.accepting(i))
    {
      const state_list& slist = scc_prod.states_of(i);
      state_list::const_iterator it;
      for (it = slist.begin(); it != slist.end(); ++it)
      {
        spot::state* proj = prod->project_state(*it, model);
        assert(proj);
        if (res->find(proj) == res->end())
          res->insert(proj);
        else
          delete proj;
      }
    }
  }
  return res;
}

void delete_state_set(spot::state_set* s)
{
  spot::state_set::iterator it;
  for (it = s->begin(); it != s->end(); ++it)
    delete *it;
  delete s;
}
