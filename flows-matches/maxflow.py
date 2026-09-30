import sys
from collections import deque


def main():
    data = sys.stdin.buffer.read().split()
    vertex_count = int(data[0])
    source = int(data[1])
    sink = int(data[2])
    edge_count = int(data[3])

    # Varje kant lagras med sin bakåtkant på nästa plats, så e ^ 1 ger motsatsen.
    graph = [[] for _ in range(vertex_count + 1)]
    head = []
    residual = []
    capacity = []
    pos = 4
    for _ in range(edge_count):
        u = int(data[pos])
        v = int(data[pos + 1])
        c = int(data[pos + 2])
        pos += 3
        capacity.append(c)
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

    used = []
    for i in range(edge_count):
        e = 2 * i
        flow = capacity[i] - residual[e]
        if flow > 0:
            used.append("%d %d %d" % (head[e ^ 1], head[e], flow))

    out = [str(vertex_count), "%d %d %d" % (source, sink, total_flow), str(len(used))]
    out.extend(used)
    sys.stdout.write("\n".join(out) + "\n")


def path_edges(arrival, head, source, sink):
    """Kanterna på den hittade stigen, baklänges från utloppet."""
    v = sink
    while v != source:
        e = arrival[v]
        yield e
        v = head[e ^ 1]


main()
