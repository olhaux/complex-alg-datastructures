import sys


def tokens():
    """Läser indata lat, en rad i taget, så svarta lådans utdata kan läsas efteråt."""
    for line in sys.stdin:
        for tok in line.split():
            yield tok


def main():
    tok = tokens()
    x_count = int(next(tok))
    y_count = int(next(tok))
    edge_count = int(next(tok))
    edges = [(int(next(tok)), int(next(tok))) for _ in range(edge_count)]

    # Flödesgrafen: matchningens hörn plus källa och utlopp.
    source = x_count + y_count + 1
    sink = x_count + y_count + 2

    out = [str(sink), "%d %d" % (source, sink), str(x_count + edge_count + y_count)]
    for x in range(1, x_count + 1):
        out.append("%d %d 1" % (source, x))
    for x, y in edges:
        out.append("%d %d 1" % (x, y))
    for y in range(x_count + 1, x_count + y_count + 1):
        out.append("%d %d 1" % (y, sink))
    sys.stdout.write("\n".join(out) + "\n")
    sys.stdout.flush()

    # Svaret från flödesproblemet.
    next(tok)
    next(tok)
    next(tok)
    next(tok)
    flow_edge_count = int(next(tok))

    matching = []
    for _ in range(flow_edge_count):
        u = int(next(tok))
        v = int(next(tok))
        f = int(next(tok))
        # Bara mättade kanter mellan X och Y hör till matchningen.
        if f > 0 and 1 <= u <= x_count and x_count < v <= x_count + y_count:
            matching.append("%d %d" % (u, v))

    sys.stdout.write("%d %d\n%d\n" % (x_count, y_count, len(matching)))
    sys.stdout.write("\n".join(matching) + ("\n" if matching else ""))


main()
