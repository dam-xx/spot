#! /bin/bash

NIPS_COMPILER=~/lrde/vmssg/nips_c/CodeGen
NIPS_ASSEMBLER=~/lrde/vmssg/nips_vm/nips_asm.pl
THISDIR=`pwd`

if [ ! -f "$NIPS_COMPILER" ]; then
    echo "You have to specify the path of your NIPS compiler (CodeGen)"
    exit 3
fi

if [ ! -f "$NIPS_ASSEMBLER" ]; then
    echo "You have to specify the path of your NIPS assembler (nips_asm.pl)"
    exit 3
fi

if [ $# -ne 1 ]; then
    echo "usage : $0 promela_model"
    exit 1
fi

FILE="$(cd `dirname $1`; pwd)/`basename $1`"
TMP_FILE="/tmp/`basename $1`"

cpp "$FILE" | sed 's/^#.*$//' > "$TMP_FILE"

cd `dirname $NIPS_COMPILER`
./`basename $NIPS_COMPILER` "$TMP_FILE"

cd `dirname $NIPS_ASSEMBLER`
./`basename $NIPS_ASSEMBLER` "$TMP_FILE.s"

mv "$TMP_FILE.b" "$FILE.b"

echo "$FILE".b
