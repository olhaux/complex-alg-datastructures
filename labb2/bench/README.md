# Benchmark scripts

Run from WSL (Java lives there). Each script compiles and runs in `~/labb2-bench/`, outside the Windows filesystem.

```
bash bench/bench.sh v5-pruning test large   # built-in -t tests + full-output diff against .utdata
python3 bench/genstress.py                   # once: 500k-word, 100-query stress input
bash bench/stress.sh v1-dynprog 1            # once: v1's output becomes the expected stress output
bash bench/stress.sh v5-pruning 3            # time a version on the stress input
bash bench/fuzz.sh v5-pruning 300            # compare against v1 on 300 random small inputs
```

`stress.sh` and `fuzz.sh` need the version (and `v1-dynprog`) compiled by `bench.sh` first.
