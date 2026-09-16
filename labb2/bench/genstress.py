# Builds a Kattis-sized stress test: ~500k-word dictionary, 100 misspelled words.
import random, os
random.seed(2350)
src = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ordlista.utf8")
base = [w for w in open(src, encoding="utf-8").read().split("\n") if w]
suffixes = ["", "na", "en", "er", "et", "s", "ar", "or", "ande", "ade", "t", "erna", "arna", "ens", "ets", "ning"]
words = set()
for w in base:
    for s in suffixes:
        if len(w + s) < 40:
            words.add(w + s)
words = sorted(random.sample(sorted(words), 500000), key=lambda w: w.encode("utf-8"))
alpha = "abcdefghijklmnopqrstuvwxyzåäö"
wset = set(words)
queries = []
while len(queries) < 100:
    w = list(random.choice(words))
    for _ in range(random.randint(1, 4)):
        op = random.randrange(3); p = random.randrange(len(w) + (op == 1))
        if op == 0 and len(w) > 1: del w[min(p, len(w)-1)]
        elif op == 1: w.insert(p, random.choice(alpha))
        else: w[min(p, len(w)-1)] = random.choice(alpha)
    q = "".join(w)
    if q and q not in wset and len(q) < 40 and q not in queries:
        queries.append(q)
d = os.path.expanduser("~/labb2-bench/stress"); os.makedirs(d, exist_ok=True)
with open(d + "/stress500k.indata", "w", encoding="utf-8") as f:
    f.write("\n".join(words) + "\n#\n" + "\n".join(queries) + "\n")
print(len(words), "words;", len(queries), "queries; max len", max(map(len, words)), "; avg query len", sum(map(len, queries))/100)
