#!/bin/bash
# usage: bench.sh <version-folder-name> <test folders...>
# Compiles labb2/versions/<v> in WSL home, runs the built-in -t tests, then diffs full stdout against .utdata.
SRC=$(cd "$(dirname "$0")/.." && pwd)
V=$1; shift
W=$HOME/labb2-bench/$V
rm -rf $W; mkdir -p $W; cp $SRC/versions/$V/*.java $W; cp -r $SRC/test $SRC/large $W
cd $W && javac -encoding UTF-8 *.java || exit 1
for f in "$@"; do
  java Main -t $f
  for in in $f/*.indata; do
    out=${in%.indata}.utdata
    s=$(date +%s%N); java Main < $in > got.txt; e=$(date +%s%N)
    if diff -q <(tr -d '\r' < $out | sed 's/ *$//') <(sed 's/ *$//' got.txt) >/dev/null; then r=OK; else r=DIFF; fi
    echo "  stdin run $(basename $in): $(( (e-s)/1000000 )) ms wall (incl. JVM start), output $r"
  done
done
