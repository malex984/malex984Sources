#!/bin/sh

# env

"$SINGULAR_EXECUTABLE" -teq "$srcdir/ederc.tst"
"$SINGULAR_EXECUTABLE" -teq "$srcdir/syzextra.tst"

