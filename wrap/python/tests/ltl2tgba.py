# This is a python translation of the ltl2tgba C++ test program.
# Compare with src/tgbatest/ltl2tgba.test.

import sys
import getopt
import ltihooks
import spot

def usage(prog):
    print "Usage: ", prog, """ [OPTIONS...] formula

Options:
  -a   display the accepting_conditions BDD, not the reachability graph
  -A   same as -a, but as a set
  -d   turn on traces during parsing
  -D   degeneralize the automaton
  -r   display the relation BDD, not the reachability graph
  -R   same as -r, but as a set
  -t   display reachable states in LBTT's format
  -v   display the BDD variables used by the automaton"""
    sys.exit(2)


prog = sys.argv[0]
try:
    opts, args = getopt.getopt(sys.argv[1:], 'aAdDrRtv')
except getopt.GetoptError:
    usage(prog)

exit_code = 0
debug_opt = 0
degeneralize_opt = None
output = 0

for o, a in opts:
    if o == '-a':
        output = 2
    elif o == '-A':
        output = 4
    elif o == '-d':
        debug_opt = 1
    elif o == '-D':
        degeneralize_opt = 1
    elif o == '-r':
        output = 1
    elif o == '-R':
        output = 3
    elif o == '-t':
        output = 6
    elif o == '-v':
        output = 5
    else:
        usage(prog)

if len(args) != 1:
    usage(prog)


cout = spot.get_cout()
cerr = spot.get_cerr()

e = spot.default_environment.instance()
p = spot.empty_parse_error_list()

f = spot.parse(args[0], p, e, debug_opt)
if spot.format_parse_errors(cerr, args[0], p):
    exit_code = 1
    
dict = spot.bdd_dict()

if f:
    concrete = spot.ltl_to_tgba(f, dict)
    spot.destroy(f)
    del f
    a = concrete

    degeneralized = None
    if degeneralize_opt:
        a = degeneralized = spot.tgba_tba_proxy(a)

    if output == 0:
        spot.dotty_reachable(cout, a)
    elif output == 1:
        spot.bdd_print_dot(cout, concrete.get_dict(),
                           concrete.get_core_data().relation)
    elif output == 2:
        spot.bdd_print_dot(cout, concrete.get_dict(),
                           concrete.get_core_data().accepting_conditions)
    elif output == 3:
        spot.bdd_print_set(cout, concrete.get_dict(),
                           concrete.get_core_data().relation)
        print
    elif output == 4:
        spot.bdd_print_set(cout, concrete.get_dict(),
                           concrete.get_core_data().accepting_conditions)
        print
    elif output == 5:
        a.get_dict().dump(cout)
    elif output == 6:
        spot.lbtt_reachable(cout, a)
    else:
        assert "unknown output option"

    # Must delete absolutely all references to an automaton
    # so that the C++ destructor gets called.
    del a

    if degeneralize_opt:
        degeneralized.thisown = 1
        del degeneralized

    concrete.thisown = 1
    del concrete

else:
    exit_code = 1

del dict;

assert spot.atomic_prop.instance_count() == 0
assert spot.unop.instance_count() == 0
assert spot.binop.instance_count() == 0
assert spot.multop.instance_count() == 0

