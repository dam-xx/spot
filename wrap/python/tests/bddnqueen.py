# Python translation of the C++ example from the BuDDy distribution.
# (compare with buddy/examples/queen/queen.cxx)

import ltihooks
import sys
from buddy import *

# Build the requirements for all other fields than (i,j) assuming
# that (i,j) has a queen.
def build(i, j):
    a = b = c = d = bddtrue
    
    # No one in the same column.
    for l in side:
        if l != j:
            a &= X[i][j] >> -X[i][l]

    # No one in the same row.
    for k in side:
        if k != i:
            b &= X[i][j] >> -X[k][j]

    # No one in the same up-right diagonal.
    for k in side:
        ll = k - i + j
        if ll >= 0 and ll < N:
            if k != i:
                c &= X[i][j] >> -X[k][ll]

    # No one in the same down-right diagonal.
    for k in side:
        ll = i + j - k
        if ll >= 0 and ll < N:
            if k != i:
                c &= X[i][j] >> -X[k][ll]

    global queen
    queen &= a & b & c & d



# Get the number of queens from the command-line, or default to 8.
if len(sys.argv) > 1:
    N = int(argv[1])
else:
    N = 8

side = range(N)

# Initialize with 100000 nodes, 10000 cache entries and NxN variables.
bdd_init(N * N * 256, 10000)
bdd_setvarnum(N * N)

queen = bddtrue

# Build variable array.
X = [[bdd_ithvar(i*N+j) for j in side] for i in side]

# Place a queen in each row.
for i in side:
    e = bddfalse
    for j in side:
        e |= X[i][j]
    queen &= e

# Build requirements for each variable(field).
for i in side:
    for j in side:
        print "Adding position %d, %d" % (i, j)
        build(i, j)

# Print the results.
print "There are", bdd_satcount(queen), "solutions"
print "one is:"
solution = bdd_satone(queen)
bdd_printset(solution)
print

bdd_done()
