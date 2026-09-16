# Labb 2 – optimisation log

Each folder in `versions/` is a complete Kattis submission: upload its `Main.java` and `ClosestWords.java`. Input, output and `java Main -t` behave like the original in every version.

## Timings

Measured in WSL (OpenJDK 11), wall-clock time including JVM start (~60 ms).

- **large3 / large4:** the two course test cases in `large/`, a 70,431-word dictionary with 2 and 6 misspelled words.
- **stress:** a Kattis-sized input: 500,000 words and 100 misspelled words.

| Version | Change | large3 | large4 | stress | Stress speed-up vs previous |
|---|---|---|---|---|---|
| v0-original | recursive `partDist` | ~4 min (est.) | ~45 h (est.) | – | – |
| v1-dynprog | DP matrix, filled row by row | 97 ms | 199 ms | 23.3 s | – |
| v2-reuse-matrix | one matrix per misspelled word | 52 ms | 57 ms | 7.6 s | 3.1× |
| v3-prefix | skip rows shared with previous word | 59 ms | 44 ms | 5.2 s | 1.5× |
| v4-precomputed-prefix | compute prefixes once per dictionary | 47 ms | 49 ms | 3.0 s | 1.7× |
| v5-pruning | length-difference skip + row-minimum stop | 46 ms | 32 ms | 1.39 s | 2.2× |

The large3/large4 columns come from the `-t` timer, so they don't include JVM start. At 30–60 ms they are mostly noise; the stress column is the one to compare.

**How the v0 estimates were made.** Running v0 on `large/` would take hours, so I counted the calls instead. `partDist` on words of lengths m and n makes C(m, n) = 1 + C(m−1, n−1) + C(m−1, n) + C(m, n−1) calls. Summed over the dictionary, that is 1.8·10¹¹ calls for large3 and 1.2·10¹⁴ for large4. One pair with 3.4·10⁸ calls took 437 ms after JVM start, which is 1.3 ns per call.

## What each step does and why it helps

**v1 – dynamic programming (biggest gain).** The recursion recomputes the same subproblems an exponential number of times. The matrix computes each of the (n+1)(m+1) cells once. Rows follow the dictionary word and columns the misspelled word. The loop fills one row at a time, left to right, which is the order Java stores a row's `int[]` in memory, so the three cells each step reads sit right next to the one it writes. Example: "dabbbhud" against a 13-letter word takes 1.3·10⁷ recursive calls but 8·13 = 104 matrix cells.

**v2 – no allocation per word pair.** v1 made a new `int[n+1][m+1]` for every one of 500,000 × 100 = 5·10⁷ word pairs. v2 allocates once per misspelled word, sets row 0 and column 0 once, reads the misspelled word as a `char[]`, and looks up `M[i-1]` and `M[i]` once per row instead of once per cell. That makes three changes in one step, so the 3.1× cannot be credited to allocation alone.

**v3 – shared prefixes (theory questions 9–10).** Row i only depends on the first i letters of the dictionary word. The dictionary is sorted, so neighbours often share a prefix: "abbedissa" → "abbedissan" shares 9 letters, and only row 10 needs computing. On the stress dictionary this leaves 21% of the rows. The speed-up was only 1.5× because v3 still compared neighbouring words letter by letter for every misspelled word: 100 × 500,000 comparisons whose result never changes.

**v4 – precompute per dictionary.** The prefix lengths, `char[]` arrays and the longest word length are computed once when the dictionary is read. The matrix is allocated at full height once per misspelled word and never grows. After v4, reading input plus JVM start is ~230 ms of the 3.0 s, so the rest is DP.

**v5 – pruning.** Two cut-offs, both exact:

- **Length difference.** Each insertion or deletion changes the length by 1, so distance ≥ |len(w1) − len(w2)|. If that already exceeds the best distance so far, the word is skipped. Example: best distance 2, misspelled word of 5 letters, dictionary word of 9 letters: difference 4 > 2, skip.
- **Row minimum.** Every cell in row i+1 is at least the smallest value in row i, so once a whole row exceeds the best distance, the final value will too.

Both use strict `>`, because words at exactly the best distance must be printed. After a skip or an early stop, only the rows actually computed are valid, so v5 tracks `validRows` and starts at min(`validRows`, prefix) instead of trusting the prefix alone.

## Tried and rejected

**Tighter row bound (v6, not kept).** This used min over j of M[i][j] + |(n−i) − (m−j)| instead of the plain row minimum. It is a valid bound and stops earlier, but computing it in every cell cost more than it saved: 1.67 s against v5's 1.39 s.

## Correctness checks

- Every version matches `test/` and `large/` under both the built-in `-t` check and a full `diff` against `.utdata`. The built-in check doesn't notice when a word is missing from the output; `diff` does.
- v2–v5 produce byte-identical output to v1 on the stress input.
- v5 matches v1 on 300 random small inputs. These cover words that are prefixes of their neighbours, words with repeated letters, and long misspelled words. A deliberately broken copy (`>=` instead of `>` in the length skip) failed 193 of the 300 cases, which shows the check can catch this kind of bug.

## Course sources (checked in the ADK NotebookLM notebook)

- **Filling a DP matrix row by row:** ovn3, exercise 6b ("Vi kan alltid titta på föregående rad (i − 1), eftersom vi beräknar en rad i taget").
- **What to keep during the computation:** ovn3, exercise 5.
- **Prefix reuse:** not in the lectures or exercises. It comes from the lab's own theory questions 9–10.
- **Allocating once, cache locality, pruning:** not in the course material; the lectures use the RAM model, where every memory access costs the same. Locality is required by the lab text itself.
