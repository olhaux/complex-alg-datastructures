#!/bin/bash
# usage: fuzz.sh <version> [cases]  -- compares against v1-dynprog on random small inputs
V=$1; N=${2:-300}; B=$HOME/labb2-bench; F=$B/fuzz; mkdir -p $F
python3 - "$F" "$N" <<'PY'
import random, sys
d, n = sys.argv[1], int(sys.argv[2])
random.seed(7)
for t in range(n):
    alpha = random.choice(["ab", "abc", "abcåäö", "abcdefghijklmnopqrstuvwxyzåäö"])
    words = set()
    for _ in range(random.randint(1, 60)):
        w = "".join(random.choice(alpha) for _ in range(random.randint(1, random.choice([3, 8, 20]))))
        words.add(w)
        if random.random() < 0.3: words.add(w + "".join(random.choice(alpha) for _ in range(random.randint(1, 4))))
        if random.random() < 0.2 and len(w) > 1: words.add(w[:random.randint(1, len(w)-1)])
    words = sorted(words, key=lambda w: w.encode("utf-8"))
    qs = []
    for _ in range(random.randint(1, 8)):
        q = "".join(random.choice(alpha) for _ in range(random.randint(1, random.choice([3, 8, 25]))))
        if q not in words and q not in qs: qs.append(q)
    if not qs: qs = ["zzzzzzzzzzzz"] if "zzzzzzzzzzzz" not in words else ["qqqqqqqq"]
    open(f"{d}/{t}.in", "w", encoding="utf-8").write("\n".join(words) + "\n#\n" + "\n".join(qs) + "\n")
PY
cd $F; bad=0
for t in $(seq 0 $((N-1))); do
  (cd $B/v1-dynprog && java Main < $F/$t.in) > $t.exp
  (cd $B/$V && java Main < $F/$t.in) > $t.got
  cmp -s $t.exp $t.got || { bad=$((bad+1)); echo "MISMATCH case $t"; }
done
echo "$V: $N cases, $bad mismatches"
