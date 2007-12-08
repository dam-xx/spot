#!/bin/sh

# This file is run by the LRDE autobuilder after a successful compilation.
# It is not meant to be distributed with Spot.

set -e
set -x

rm -rf /lrde/dload/spot/spot-snapshot.tmp
cp -pR doc/spot.html /lrde/dload/spot/spot-snapshot.tmp
chmod -R a+rX /lrde/dload/spot/spot-snapshot.tmp
mv -f spot-*.tar.gz /lrde/dload/spot/spot-snapshot.tar.gz
chmod a+rX /lrde/dload/spot/spot-snapshot.tar.gz
rm -rf /lrde/dload/spot/spot-snapshot.html
mv -f /lrde/dload/spot/spot-snapshot.tmp /lrde/dload/spot/spot-snapshot.html
