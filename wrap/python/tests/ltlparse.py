import sys
import ltihooks
import spot

e = spot.default_environment.instance()
p = spot.empty_parse_error_list()

l = ['GFa', 'a U (((b)) xor c)', '!(FFx <=> Fx)', 'a \/ a \/ b \/ a \/ a'];

for str1 in l:
    f = spot.parse(str1, p, e, 0)
    if spot.format_parse_errors(spot.get_cout(), str1, p):
        sys.exit(1)
    str2 = str(f)
    spot.destroy(f)
    print str2
    # Try to reparse the stringified formula
    f = spot.parse(str2, p, e)
    if spot.format_parse_errors(spot.get_cout(), str2, p):
        sys.exit(1)
    print f
    spot.destroy(f)

assert spot.atomic_prop.instance_count() == 0
assert spot.binop.instance_count() == 0
assert spot.unop.instance_count() == 0
assert spot.multop.instance_count() == 0
