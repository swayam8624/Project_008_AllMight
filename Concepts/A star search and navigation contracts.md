# A* search and navigation contracts

A* is Dijkstra-style relaxation ordered by an estimate of the remaining cost. For node n,

$$
f(n)=g(n)+h(n),
$$

where $g(n)$ is the best path cost currently known from the start and $h(n)$ estimates the cost from n to the goal.

The algorithm is not “put nodes in a priority queue and stop when the goal first appears.” Correctness depends on the edge-cost domain, heuristic assumptions, stale queue entries, and whether improved paths are allowed to reopen nodes.

With nonnegative edge costs and a zero heuristic, A* becomes Dijkstra’s algorithm. That is a useful oracle mode. An admissible heuristic never overestimates the true remaining cost; a consistent heuristic additionally satisfies a triangle-like condition and simplifies closed-set reasoning.

## Relaxation

For edge n → m with cost c,

$$
g_{candidate}=g(n)+c.
$$

If this is smaller than the currently known $g(m)$, update $g(m)$, remember n as m’s parent, and push a new queue item. Priority queues commonly keep stale older entries; when popped, compare the queued score with the latest stored score and discard stale work.

A design that permanently closes a node before the heuristic assumptions justify that decision can return a suboptimal path.

## Navigation is larger than A*

A navigation system also needs a graph construction story. For a navmesh this includes walkable geometry, polygon adjacency, portals, agent radius/height constraints, dynamic obstacle policy and a path-smoothing or funnel stage. A* only solves the discrete shortest-path problem on the graph it is given.

## Executable reference

tools/labs/spatial_index_lab.cpp includes an A* implementation and tests a weighted graph using h = 0, making the expected result equivalent to Dijkstra’s optimum. It also checks an unreachable goal.

The witness does not yet prove navmesh generation, admissible geometric heuristics, funnel smoothing, dynamic replanning, hierarchical navigation or KairoSpatial parity.
