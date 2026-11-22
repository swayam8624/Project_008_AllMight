# Floyd–Warshall all-pairs shortest paths

Floyd–Warshall computes shortest-path distances between every ordered pair of vertices using dynamic programming over which intermediate vertices are allowed.

Let

$$
D^{(k)}[i,j]
$$

be the best distance from $i$ to $j$ using only intermediate vertices from the first $k$ allowed vertices. When vertex $k$ becomes available, an optimal path either does not use it or uses it to split the path into $i\rightarrow k$ and $k\rightarrow j$:

$$
D^{(k)}[i,j]
=
\min\left(
D^{(k-1)}[i,j],
D^{(k-1)}[i,k]+D^{(k-1)}[k,j]
\right).
$$

The in-place triple loop is valid because the recurrence can be ordered by outermost intermediate vertex k.

Time complexity is $O(V^3)$ and dense storage is $O(V^2)$. That makes it unsuitable for huge sparse navigation graphs but excellent as a small-graph oracle for testing Dijkstra/A* implementations.

Negative edges are allowed when no relevant negative cycle exists. After completion, a negative diagonal distance $D[i,i]<0$ indicates a reachable negative cycle under the standard formulation.

## Existing executable evidence

tools/labs/phase5_algorithm_workload_lab.cpp already implements Floyd–Warshall and checks its source-row result against Dijkstra on a nonnegative test graph. The earlier repo audit missed it because the concept was absent from the canonical concept graph, not because the code was absent.

Related: [[A star search and navigation contracts]].
