#ifndef SPOT_TGBA_BDDPRINT_HH
# define SPOT_TGBA_BDDPRINT_HH

#include <string>
#include <iostream>
#include "tgbabdddict.hh"
#include <bdd.h>

namespace spot
{

  /// \brief Print a BDD as a list of literals.
  ///
  /// This assumes that \a b is a conjunction of literals.
  /// \param os The output stream.
  /// \param dict The dictionary to use, to lookup variables.
  /// \param b The BDD to print.
  std::ostream& bdd_print_sat(std::ostream& os,
			      const tgba_bdd_dict& dict, bdd b);

  /// \brief Format a BDD as a list of literals.
  ///
  /// This assumes that \a b is a conjunction of literals.
  /// \param dict The dictionary to use, to lookup variables.
  /// \param b The BDD to print.
  /// \return The BDD formated as a string.
  std::string bdd_format_sat(const tgba_bdd_dict& dict, bdd b);

  /// \brief Print a BDD as a list of accepting conditions.
  ///
  /// This is used when saving a TGBA.
  /// \param b The BDD to print.
  /// \return The BDD formated as a string.
  std::ostream& bdd_print_acc(std::ostream& os,
			      const tgba_bdd_dict& dict, bdd b);

  /// \brief Print a BDD as a set.
  /// \param os The output stream.
  /// \param dict The dictionary to use, to lookup variables.
  /// \param b The BDD to print.
  std::ostream& bdd_print_set(std::ostream& os,
			      const tgba_bdd_dict& dict, bdd b);
  /// \brief Format a BDD as a set.
  /// \param dict The dictionary to use, to lookup variables.
  /// \param b The BDD to print.
  /// \return The BDD formated as a string.
  std::string bdd_format_set(const tgba_bdd_dict& dict, bdd b);

  /// \brief Print a BDD as a diagram in dotty format.
  /// \param os The output stream.
  /// \param dict The dictionary to use, to lookup variables.
  /// \param b The BDD to print.
  std::ostream& bdd_print_dot(std::ostream& os,
			      const tgba_bdd_dict& dict, bdd b);

  /// \brief Print a BDD as a table.
  /// \param os The output stream.
  /// \param dict The dictionary to use, to lookup variables.
  /// \param b The BDD to print.
  std::ostream& bdd_print_table(std::ostream& os,
				const tgba_bdd_dict& dict, bdd b);

}

#endif // SPOT_TGBA_BDDPRINT_HH
