# Make sure that interdependencies between the spot and buddy wrappers
# are not problematic.
import ltihooks
import spot
import buddy
e = spot.default_environment.instance()
p = spot.empty_parse_error_list()
f = spot.parse('GFa', p, e)
dict = spot.bdd_dict()
a = spot.ltl_to_tgba(f, dict)
s0 = a.get_init_state()
b = s0.as_bdd()
print b
iter = a.succ_iter(s0)
iter.first()
while not iter.done():
    c = iter.current_condition()
    print c
    b &= c # `&=' is defined only in buddy.  So if this statement works
           # it means buddy can grok spot's objects.
    iter.next()
print b
