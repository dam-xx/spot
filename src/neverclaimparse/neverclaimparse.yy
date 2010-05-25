/* Copyright (C) 2009, 2010 Laboratoire de Recherche et Développement
** de l'Epita (LRDE).
** Copyright (C) 2003, 2004, 2005, 2006 Laboratoire d'Informatique de
** Paris 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
** Université Pierre et Marie Curie.
**
** This file is part of Spot, a model checking library.
**
** Spot is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Spot is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
** or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
** License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Spot; see the file COPYING.  If not, write to the Free
** Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
** 02111-1307, USA.
*/
%language "C++"
%locations
%defines
%expect 0 // No shift/reduce
%expect-rr 0 // No reduce/reduce
%name-prefix "neverclaimyy"
%debug
%error-verbose

%code requires
{
#include <string>
#include "public.hh"
typedef std::pair<std::string*, std::string*> pair;
}

%parse-param {spot::neverclaim_parse_error_list& error_list}
%parse-param {spot::ltl::environment& parse_environment}
%parse-param {spot::tgba_explicit_string*& result}
%union
{
  std::string* str;
  pair* p;
  std::list<pair>* list;
}

%code
{
#include "ltlast/constant.hh"
  /* Unfortunately Bison 2.3 uses the same guards in all parsers :( */
#undef BISON_POSITION_HH
#undef BISON_LOCATION_HH
#include "ltlparse/public.hh"

/* neverclaimparse.hh and parsedecl.hh include each other recursively.
   We must ensure that YYSTYPE is declared (by the above %union)
   before parsedecl.hh uses it. */
#include "parsedecl.hh"
using namespace spot::ltl;
}

%token NEVER "never"
%token SKIP "skip"
%token IF "if"
%token FI "fi"
%token ARROW "->"
%token GOTO "goto"
%token <str> FORMULA
%token <str> IDENT
%type <p> transition
%type <list> transitions

%destructor { delete $$; } <str>
%destructor { delete $$->first; delete $$->second; delete $$; } <p>
%destructor {
  for (std::list<pair>::iterator i = $$->begin();
       i != $$->end(); ++i)
  {
    delete i->first;
    delete i->second;
  }
  delete $$;
  } <list>
%printer { debug_stream() << *$$; } <str>

%%
neverclaim:
  "never" '{' states '}'
;

states:
  /* empty */
  | state states
;

state:
  IDENT ':' "skip"
    {
      result->create_transition(*$1, *$1);
      delete $1;
    }
  | IDENT ':' { delete $1; }
  | IDENT ':' "if" transitions "fi"
    {
      std::list<pair>::iterator it;
      for (it = $4->begin(); it != $4->end(); ++it)
      {
	spot::tgba_explicit::transition* t =
	  result->create_transition(*$1,*it->second);
	spot::ltl::parse_error_list pel;
	spot::ltl::formula* f = spot::ltl::parse(*(it->first), pel);
	result->add_condition(t, f);
      }
      // Free the list
      delete $1;
      for (std::list<pair>::iterator it = $4->begin();
	   it != $4->end(); ++it)
      {
	delete it->first;
	delete it->second;
      }
      delete $4;
    }
;

transitions:
  /* empty */ { $$ = new std::list<pair>; }
  | transition transitions
    {
      $2->push_back(*$1);
      delete $1;
      $$ = $2;
    }
;

transition:
  ':' ':' FORMULA  "->" "goto" IDENT
    {
      $$ = new pair($3, $6);
    }
%%

void
neverclaimyy::parser::error(const location_type& location,
			    const std::string& message)
{
  error_list.push_back(spot::neverclaim_parse_error(location, message));
}

namespace spot
{
  tgba_explicit_string*
  neverclaim_parse(const std::string& name,
		   neverclaim_parse_error_list& error_list,
		   bdd_dict* dict,
		   environment& env,
		   bool debug)
  {
    if (neverclaimyyopen(name))
      {
	error_list.push_back
	  (neverclaim_parse_error(neverclaimyy::location(),
				  std::string("Cannot open file ") + name));
	return 0;
      }
    tgba_explicit_string* result = new tgba_explicit_string(dict);
    neverclaimyy::parser parser(error_list, env, result);
    parser.set_debug_level(debug);
    parser.parse();
    neverclaimyyclose();
    result->merge_transitions();
    return result;
  }
}

// Local Variables:
// mode: c++
// End:
