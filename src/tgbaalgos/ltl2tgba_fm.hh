#ifndef SPOT_TGBA_LTL2TGBA_FME_HH
# define SPOT_TGBA_LTL2TGBA_FME_HH

#include "ltlast/formula.hh"
#include "tgba/tgbaexplicit.hh"

namespace spot
{
  /// \brief Build a spot::tgba_explicit* from an LTL formula.
  ///
  /// This is based on the following paper.
  /// \verbatim
  /// @InProceedings{couvreur.99.fm,
  ///   author	  = {Jean-Michel Couvreur},
  ///   title     = {On-the-fly Verification of Temporal Logic},
  ///   pages     = {253--271},
  ///   editor	  = {Jeannette M. Wing and Jim Woodcock and Jim Davies},
  ///   booktitle = {Proceedings of the World Congress on Formal Methods in the
  /// 		     Development of Computing Systems (FM'99)},
  ///   publisher = {Springer-Verlag},
  ///   series	  = {Lecture Notes in Computer Science},
  ///   volume	  = {1708},
  ///   year      = {1999},
  ///   address	  = {Toulouse, France},
  ///   month	  = {September},
  ///   isbn      = {3-540-66587-0}
  /// }
  /// \endverbatim
  tgba_explicit* ltl_to_tgba_fm(const ltl::formula* f, bdd_dict* dict);
}

#endif // SPOT_TGBA_LTL2TGBA_HH
