#!/bin/bash
# usage: stress.sh <version> [runs]
# Run bench.sh on the version first so it is compiled.
V=$1; RUNS=${2:-3}
D=$HOME/labb2-bench; cd $D/$V || exit 1
IN=$D/stress/stress500k.indata; EXP=$D/stress/stress500k.utdata
for r in $(seq $RUNS); do
  s=$(date +%s%N); java Main < $IN > stress.out; e=$(date +%s%N)
  if [ ! -f $EXP ]; then cp stress.out $EXP; res="(saved as expected output)";
  elif cmp -s stress.out $EXP; then res=OK; else res=DIFF; fi
  echo "$V stress run $r: $(( (e-s)/1000000 )) ms, output $res"
done
