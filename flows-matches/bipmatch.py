import sys
from collections import deque


def max_flow(vertex_count, source, sink, edges):
    """Edmonds-Karp. edges är (u, v, kapacitet). Ger totalt flöde och flödet per kant."""
    # Varje kant lagras med sin bakåtkant på nästa plats, så e ^ 1 ger motsatsen.
    graph = [[] for _ in range(vertex_count + 1)]
    head = []
    residual = []
    for u, v, c in edges:
        graph[u].append(len(head))
        head.append(v)
        residual.append(c)
        graph[v].append(len(head))
        head.append(u)
        residual.append(0)

    total_flow = 0
    while True:
        # Breddenförstsökning ger kortaste stigen i restflödesgrafen.
        arrival = [-1] * (vertex_count + 1)
        arrival[source] = -2
        queue = deque([source])
        while queue and arrival[sink] == -1:
            u = queue.popleft()
            for e in graph[u]:
                v = head[e]
                if residual[e] > 0 and arrival[v] == -1:
                    arrival[v] = e
                    queue.append(v)
        if arrival[sink] == -1:
            break

        bottleneck = min(residual[e] for e in path_edges(arrival, head, source, sink))
        for e in path_edges(arrival, head, source, sink):
            residual[e] -= bottleneck
            residual[e ^ 1] += bottleneck
        total_flow += bottleneck

    flows = [c - residual[2 * i] for i, (u, v, c) in enumerate(edges)]
    return total_flow, flows


def path_edges(arrival, head, source, sink):
    """Kanterna på den hittade stigen, baklänges från utloppet."""
    v = sink
    while v != source:
        e = arrival[v]
        yield e
        v = head[e ^ 1]


def main():
    data = sys.stdin.buffer.read().split()
    x_count = int(data[0])
    y_count = int(data[1])
    edge_count = int(data[2])
    pairs = [(int(data[3 + 2 * i]), int(data[4 + 2 * i])) for i in range(edge_count)]

    # Flödesgrafen: matchningens hörn plus källa och utlopp.
    source = x_count + y_count + 1
    sink = x_count + y_count + 2
    edges = [(source, x, 1) for x in range(1, x_count + 1)]
    edges += [(x, y, 1) for x, y in pairs]
    edges += [(y, sink, 1) for y in range(x_count + 1, x_count + y_count + 1)]

    total_flow, flows = max_flow(sink, source, sink, edges)

    # Mättade kanter mellan X och Y bildar matchningen.
    matching = ["%d %d" % (x, y)
                for (x, y), f in zip(pairs, flows[x_count:x_count + edge_count]) if f > 0]

    out = ["%d %d" % (x_count, y_count), str(len(matching))]
    out.extend(matching)
    sys.stdout.write("\n".join(out) + "\n")


main()
