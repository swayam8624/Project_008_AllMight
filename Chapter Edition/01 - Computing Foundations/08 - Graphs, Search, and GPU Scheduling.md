# 08 — Graphs, Search, and GPU Scheduling

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/07 - Typed Interfaces, Identity, and Data Structures|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/09 - GPU Memory, Divergence, Atomics, and Ordering|Next chapter]]

Engine relationships can be represented as graphs, but edge meaning determines the right traversal or ordering proof. This chapter connects reachability and shortest paths to render dependencies, spatial ordering, memory scheduling, and GPU work distribution.

**By the end:** Select graph algorithms from edge assumptions, preserve dependency order, and connect spatial or render problems to their graph representation.

**Recall question:** Which property of the edges makes breadth-first search or Dijkstra’s algorithm valid for a particular path problem?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 10594–12837; source-span SHA256 51d3a3a77518ed51cea9635faa799be90d1b2eb3f6e74838c668e8f5784265ea. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## From an asset map to a correct navigable world — Phase 5, M028–M030, Parts 446–500

The editor's asset system now has logical identity, a measured memory budget and containers chosen for concrete operations. A connected simulation or game world needs more. Scene nodes form hierarchies; render passes form dependencies; paths connect navigation regions; physics bodies form contact graphs; a renderer sorts spatial keys, computes allocation offsets and caches repeated results. Each system can be understood as a graph or a sequence of transformations, but the correct algorithm depends on edge meaning, input assumptions and what must remain true after every update. Our next operation is therefore not "learn graph algorithms" as a dictionary entry. It is **take one world query, derive the data model, choose the algorithm and prove when its result is valid**.

The original Phase 5 assigns M028 to Parts 446–467 (graph structure and traversal, paths and spanning trees), M029 to 468–486 (searching, sorting, Morton ordering and prefix scans) and M030 to 487–500 (sliding windows, dynamic programming, randomized algorithms, caches and sparse-set/generation contracts). All 55 original titles in this interval have exact entries in the [[Supplementary/Foundations#Phase 5 algorithms and data-structure workload contracts — Parts 401–500|contract ledger]]. The [[tools/labs/phase5_algorithm_workload_lab.cpp|algorithm lab]] independently tests graph, scan, index and cache invariants and compares some shortest-path results to a distinct all-pairs oracle on deterministic randomly generated graphs. The lab does not implement every named algorithm; the narrative explains the mechanisms, their requirements and a path to additional proofs.

### A graph is a data model before it is an algorithm

A graph consists of vertices and edges, but the semantics of an edge must be declared. In a scene hierarchy, a parent → child edge expresses a dependency or ownership relation. In a navigation graph, an edge may represent traversable movement and carry a nonnegative distance or cost. In a physics contact graph, an undirected edge may connect bodies currently constrained by one another. In a renderer's pass graph, edges specify ordering and resource hazards. The same graph data structure cannot make all those meanings interchangeable.

An **adjacency matrix** for V vertices stores a V×V relation; its Θ(V²) memory cost makes edge existence lookup O(1) by index, while enumerating neighbors generally scans V positions. A dense weighted matrix can store a weight or sentinel for every ordered pair. An **adjacency list** stores the neighbors of each vertex and uses Θ(V+E) storage under a typical representation, allowing neighbor traversal proportional to degree. For sparse navigation or dependency graphs, the list usually avoids storing millions of absent edges. For dense small graphs, a bit matrix may have excellent locality and support word-level operations. These comparisons assume particular representations and exclude allocator overhead, indexing features and mutability.

A **directed** edge (u,v) does not automatically imply (v,u); an undirected edge does, or is represented by a pair of directed adjacency entries. A **weighted** graph associates a cost with an edge; "weight" is not automatically a physical length. It might represent travel time, CPU scheduling cost, probability-derived penalty or energy. Combining different units in one shortest-path objective without conversion is a modeling error. Negative weights can make sense in some abstract optimization graphs but are not automatically meaningful for physical movement distances.

### A queue discovers reachability one edge at a time

Suppose a unit moves through a grid in which every allowed step costs one. **Breadth-first search (BFS)** is appropriate. It begins at the source, marks its distance zero and places it in a FIFO queue. When a vertex at distance d is removed, all previously unseen neighbors are assigned distance d+1 and enqueued. Because all edges have equal unit cost and vertices are discovered in nondecreasing depth, the first discovery of each vertex yields its minimum hop count. The proof depends on queue order and equal edge costs; replacing the queue with an arbitrary stack breaks that property.

The Phase 5 laboratory stores a small directed graph with edges 0→1, 0→2, 1→2, 1→3 and 2→3. BFS from 0 gives hop distances [0,1,1,2] even though some weighted path lengths differ. This is the point: BFS minimizes edge count, not an arbitrary sum of edge weights.

**Depth-first search (DFS)** instead explores one branch as far as possible before returning. It can be written recursively, using call frames as the implicit stack, or iteratively with an explicit stack. A recursive implementation can overflow the program stack for a very long graph chain; an explicit stack lets the algorithm account for memory, order and limits. Both commonly run O(V+E) time for adjacency-list graphs when vertices/edges are processed in bounded ways.

Traversal colors or states help detect cycles. In a directed graph, an edge to a vertex currently on the active recursion stack is a back edge and indicates a directed cycle. An edge to an already completely processed vertex does not necessarily indicate a cycle. In an undirected graph, revisiting the immediate parent through the reverse edge is ordinary and must not be mistaken for a cycle; other visited-neighbor relationships may reveal one. Multigraphs and self-loops require stating how duplicate edges are represented.

### A render graph is an ordering proof, not simply a list of passes

A render graph often needs a **topological order**: every declared dependency A→B must place A before B. A directed graph has a topological ordering if and only if it is acyclic. One way to find that order is **Kahn's algorithm**. Count incoming edges for every vertex, enqueue vertices with indegree zero, remove a ready vertex, append it to the order, decrement its outgoing neighbors' indegrees, and enqueue any that become zero. If fewer than V vertices are emitted, a directed cycle remains.

The Phase 5 lab implements this algorithm and tests both a valid DAG and an added edge that creates a cycle. The source confirms the actual property: cycle detection is not based on whether the input "looks like" a sensible list of passes. Every new edge changes the dependence relation. A missing edge can be worse than a detected cycle because the graph may schedule unsafe resource access in an order the developer never intended.

A **strongly connected component (SCC)** is a maximal vertex set in which every vertex can reach every other through directed paths. Condensing each SCC into one supervertex produces an acyclic component graph. Algorithms such as Kosaraju or Tarjan find SCCs in O(V+E) time for adjacency-list graphs under standard assumptions. SCCs are useful for diagnosing cyclic modules, scene dependencies or feedback groups. They do not automatically make cyclic tasks executable: cycles may require a fixed-point solver, explicit delays, previous-frame state or a contract that rejects them. A topological sorter is allowed to reject cycles even when an application could intentionally solve a feedback system through a different algorithm.

### Shortest paths: the edge assumptions determine the proof

Now assign nonnegative weights to the same navigation graph: 0→1 costs 2, 0→2 costs 5, 1→2 costs 1, 1→3 costs 10, and 2→3 costs 1. The best path from 0 to 3 has weight 4 along 0→1→2→3, not the fewest-edge route 0→2→3, whose weight is 6.

**Dijkstra's algorithm** keeps the smallest discovered tentative distance to each vertex, repeatedly extracts the currently smallest tentative distance, and relaxes outgoing edges. If edge weight w(u,v) is nonnegative, then once a reachable vertex is extracted with the smallest unsettled distance, no detour through a not-yet-extracted vertex can later produce a shorter path to it. This is the critical invariant behind Dijkstra's correctness. Negative weights invalidate that argument.

A relaxation computes

\[
d(v)\leftarrow\min(d(v),\,d(u)+w(u,v)).
\]

The sum must be representable. An integer implementation should guard overflow; a floating implementation should specify how NaNs, infinities and negative values are handled. The lab uses a bounded integer infinity sentinel, checks positive-distance addition before performing it, rejects negative edge weights and ignores stale heap entries. With a binary heap and adjacency lists, an implementation that inserts an improved entry rather than decreasing an existing heap key typically has a bound on the order of O((V+E)log(V+E)) in a straightforward analysis, with variations from data structure and representation. Stating simply O(E log V) without identifying the queue and update strategy can hide assumptions.

**Bellman–Ford** relaxes all edges repeatedly. After at most V−1 rounds, every shortest *simple* path in a graph without reachable negative cycles has been considered, because such a path contains no more than V−1 edges. One more improving relaxation indicates a reachable negative cycle under the usual finite-distance interpretation. This algorithm allows negative edge weights, unlike Dijkstra, but costs O(VE) in a straightforward implementation. The lab checks one negative-edge graph and detects an intentionally created reachable negative cycle.

**Floyd–Warshall** computes all-pairs shortest paths by gradually allowing more intermediate vertices. Let D_k(i,j) be the shortest route from i to j whose intermediate vertices are drawn from the first k candidate vertices. The recurrence is

\[
D_k(i,j)=\min\!\left(D_{k-1}(i,j),
D_{k-1}(i,k)+D_{k-1}(k,j)\right).
\]

The algorithm follows from whether an optimal path uses intermediate k or not. A direct implementation costs O(V³) time and O(V²) storage, making it attractive as a small-graph *reference oracle* even when it is too expensive for a massive world graph. Negative cycles require explicit detection and interpretation. The Phase 5 lab uses bounded nonnegative generated weights and compares each Dijkstra row against Floyd–Warshall on 48 deterministic random six-vertex graphs. This is valuable differential testing, not proof for arbitrary graph size, negative weights or overflow.

### A* is Dijkstra informed by an admissible lower bound

In a large navigation world, we often want the shortest path to one target rather than the distances to all vertices. **A*** maintains the actual best-known cost g(n) from the start and a heuristic estimate h(n) of the remaining cost to the goal, ordering exploration by

\[
f(n)=g(n)+h(n).
\]

An **admissible heuristic** never overestimates the true remaining optimal cost h*(n), so h(n) ≤ h*(n). A **consistent heuristic** satisfies h(u) ≤ w(u,v)+h(v) for each edge and h(goal)=0, under the conventional nonnegative route-cost model. Consistency is a triangle inequality on heuristic potential and implies that f values behave nondecreasingly along improvements in a way that permits particularly simple closed-set handling. Admissibility and consistency are related but not synonymous.

A zero heuristic reduces A* to Dijkstra-like best-first expansion. A geometric Euclidean-distance heuristic is admissible only if actual path cost is guaranteed at least the corresponding straight-line travel cost in compatible units. If edge weights represent fuel cost, danger or arbitrary game-design penalties, Euclidean distance might not be a valid lower bound. A heuristic that "looks reasonable" can overestimate and sacrifice optimality.

The laboratory's A* allows improved entries to be re-enqueued and skips stale entries; it tests an explicitly supplied admissible set of estimates on the known four-vertex graph, producing 4. The code does not *verify admissibility* for a caller's arbitrary heuristic. It is a precondition whose source must be justified by the application and tested, not an automatically true property of a vector of numbers. A **greedy best-first search** might order solely by h(n), ignoring g(n); that can reach the goal quickly but is not guaranteed to minimize total path cost. A* is not simply greedy best-first with a fashionable name: the accumulated cost and heuristic bound are central.

For dynamic worlds, changed costs, temporary obstacles and agent-specific constraints require further thought. A previously optimal path can become invalid after an edge changes. Incremental planners and replanning strategies are separate systems; ordinary A* invocation does not update itself. The navigation component must also map continuous collision geometry to a graph whose edges represent traversable space.

### Minimum spanning trees solve a different global problem

A **minimum spanning tree (MST)** connects all vertices in a weighted undirected graph with no cycles and minimum total selected edge weight. It does not minimize the distance between every pair of vertices. The MST's objective is the *sum of selected edge weights*, not shortest paths from a source.

Kruskal's algorithm sorts edges by increasing weight and adds each edge only when it connects two distinct current components. A disjoint-set union detects whether adding an edge would close a cycle. The **cut property** justifies the greedy choice: under the standard MST conditions, a minimum-weight edge crossing a cut respecting the current partial forest is safe to add (with tie caveats concerning uniqueness). The Phase 5 lab computes a known four-vertex MST cost 1+2+3=6 and rejects a disconnected input rather than silently claiming a tree of all vertices.

Prim's algorithm grows a tree outward from the current connected set, repeatedly choosing the smallest edge crossing from the current tree into an unvisited vertex. It can use a priority queue and runs under different complexity bounds depending on adjacency representation and heap choice. Both find an MST for suitable connected undirected weighted graphs; neither is equivalent to Dijkstra or Bellman–Ford. A physics-island graph can use DSU connectivity without ever needing an MST, while an offline network-connection problem may require the MST objective. The abstract question determines the algorithm.

### Searching and sorting: the order of keys is itself a contract

Return to the editor's asset records. Finding one key in an unsorted vector requires **linear search**, O(n) comparisons in the worst case. Once a sequence is sorted according to a consistent comparator, **binary search** can discard half the remaining search interval per comparison, taking O(log n) comparisons for random-access iterators. But sorting the input costs work, and insertion or deletion may require moving data to maintain sorted order. A binary search on an unsorted sequence can silently return nonsense; the sorted-range precondition is not an optional hint.

**Lower bound** finds the first position at which the key could be inserted without violating order. For duplicates, it identifies the start of the equivalent range under the comparator, and upper bound identifies its end. These operations support asset lookups and compact grouping. **Exponential search** probes increasing distances (1,2,4,8,...) until the candidate range is bounded and then uses binary search; it can be useful when a target is close to the beginning of an ordered sequence with suitable random access. It is not a universal replacement for lower_bound.

Sorting algorithms have different contracts. **Selection sort** repeatedly selects the minimum of the remaining suffix, doing Θ(n²) comparisons even on favorable input in its straightforward form; it can minimize swaps in some contexts but is not a general high-performance choice. **Insertion sort** shifts elements into a sorted prefix, running O(n²) worst case but efficiently handling very small or nearly sorted arrays. **Bubble sort** repeatedly swaps adjacent out-of-order pairs; it also has quadratic worst-case behavior and offers little benefit in a production engine beyond illustrating local inversions and stability conditions.

**Merge sort** divides the input, sorts each half and merges them, giving Θ(n log n) comparisons under the standard implementation with O(n) auxiliary array storage; it can be stable by choosing the earlier equivalent element first. **Quicksort** partitions around a pivot and recursively sorts the partitions. Its average performance is typically good under appropriate pivot and input assumptions, but poor pivots can produce Θ(n²) time; its partition scheme and duplicate-key handling are part of correctness. **Heapsort** offers O(n log n) worst-case time with a typical in-place array implementation but does not preserve equal-key order.

**Introsort** begins with a fast quicksort-like strategy and switches to a worst-case-protected method such as heapsort when recursion depth suggests pathological behavior, often using insertion sort for small partitions. C++ standard-library std::sort has a complexity guarantee, but no standard mandate that every implementation use one exact textbook introsort variant. We can compare outputs and contracts without claiming the library's internal code path until the specific implementation is inspected.

**Counting sort** uses the bounded integer key range K to count occurrences and can run O(n+K) when its representation and output strategy suit the input. **Radix sort** processes keys digit by digit using a stable digit sort (for least-significant-digit variants) and has cost dependent on number of passes and digit range, not only n. They can outperform comparison sorting when keys are representable compact integers, but large key ranges, memory allocation, record movement and handling signed/floating keys introduce real tradeoffs.

A **stable sort** preserves the relative order of elements considered equal by its comparator. This matters when a renderer sorts by depth but wants earlier submission order among equal keys, or when multiple stable sorts establish a compound ordering. Stable does not mean deterministic across arbitrary parallel implementations with ambiguous floating-point keys: the comparator must satisfy a strict weak ordering and the execution must define any remaining tie policy. NaNs can violate the expected ordering model if floating comparisons are used without a consistent total-order policy.

### Morton keys turn 2D proximity into a sortable integer—within limits

Spatial grids often classify objects into cells with integer coordinates. **Morton ordering**, also called Z-order, interleaves coordinate bits so that points sharing high-order spatial bits tend to share prefixes of the key. For 16-bit x and y, put x bit i at key bit 2i and y bit i at bit 2i+1. The Phase 5 lab explicitly expands those bits and verifies Morton(1,2)=9. Sorting by these keys can improve locality of cells and serve spatial partitioning or LBVH construction.

However, Morton order is not a monotonic Euclidean distance ordering. Nearby points can cross a large key discontinuity, and distant points can share some prefixes. Coordinates must be quantized and clamped or rejected according to a declared domain, and out-of-range input can alias if high bits are discarded without checking. Sorting keys also costs time and memory movement. The correct question is whether spatially coherent traversal or BVH build cost improves for the target distribution.

### Prefix scans turn counts into a proof of nonoverlapping output

One of the most important bridges from CPU algorithms to GPU execution is the **prefix sum**, or scan. Suppose a GPU or CPU culling pass identifies per-cell counts c=[3,0,4,2]. An **exclusive scan** produces output offsets o=[0,3,3,7] with total 9. Cell i owns the half-open interval [o_i,o_i+c_i), yielding [0,3), [3,3), [3,7), [7,9). Every range is disjoint, and together they fill [0,9). This is more than a speed trick: it is an allocation and indexing correctness proof.

Formally,

\[
o_i=\sum_{j<i}c_j,\qquad
\mathrm{total}=\sum_{j=0}^{n-1}c_j.
\]

An **inclusive scan** instead outputs the sum through the current element, e.g. [3,3,7,9]. The two definitions differ by an index shift and identity element. Confusing them can make GPU scatter writes overlap or start one element too far into a buffer. The lab tests both the expected exclusive offsets and total and rejects unsigned-sum overflow before it can wrap. An empty input has total zero and no offsets.

A serial scan takes O(n) work and O(n) output space. A parallel scan can use an up-sweep/down-sweep tree, work-efficient block-local methods or hierarchical block aggregation, balancing total **work** against **span** and synchronization. A naive parallel algorithm may perform extra work or require a global barrier not available within one GPU workgroup. On GPUs, multi-dispatch scans and decoupled-lookback approaches involve device-specific synchronization and forward-progress assumptions; a portable CPU vector loop does not prove their correctness. This is exactly where Phase 4's task-dependency and memory-order reasoning becomes necessary.

### Windows, recursion, dynamic programming and greedy proofs are algorithmic choices

Many common engine operations are not named data structures at all; they are patterns for arranging computation. A **two-pointer algorithm** maintains two indexes with an invariant, such as advancing from both ends of a sorted collision-pair list until a target relationship is found. A **sliding window** maintains a contiguous active interval while extending its right boundary and removing expired left elements. If each pointer only moves forward O(n) times, the total work can be linear, despite a loop nested inside another loop. The proof depends on monotonic motion and the exact window predicate; arbitrary negative weights or nonlinear update rules can invalidate an easy window argument.

**Divide and conquer** solves a problem by splitting it into subproblems, solving them and combining results. For the recurrence T(n)=2T(n/2)+Θ(n), the recursion tree has log₂ n levels and O(n) work at each level, giving Θ(n log n), assuming balanced splits and the stated combination cost. That reasoning is the concrete mechanism behind common merge-sort complexity. A broad Master-Theorem phrase is less useful than constructing the recursion tree and checking assumptions.

**Dynamic programming (DP)** applies when subproblems overlap and the result can be composed from smaller optimal subproblem results. A DP state must contain enough information to make the recurrence valid. **Memoization** evaluates states on demand and caches their answers, often via recursion; **tabulation** fills a table in an order satisfying dependencies. They can compute the same recurrence but differ in reachability, recursion overhead, storage and traversal order. The Phase 5 lab finds the minimum number of coins to make total six from denominations {1,3,4}: two coins of value three. A naive greedy choice 4+1+1 uses three coins, showing that a locally largest denomination is not always globally optimal.

A **greedy algorithm** makes a locally preferred choice and commits without reconsidering it. It requires a correctness argument such as the MST cut property, an exchange argument or a matroid-like structure. "It worked on these examples" is not a proof of greediness. The counterexample above deliberately breaks a tempting greedy rule while dynamic programming remains correct.

### Randomization needs a probability proof as well as a seeded test

Randomized algorithms use random choices to affect expected cost or selection behavior. A reproducible pseudo-random generator with a fixed seed is useful for debugging and deterministic research reproduction, but it is not cryptographic entropy and does not make every sampling transformation unbiased.

**Reservoir sampling** can select one uniformly from a stream of unknown length without retaining all elements. Initially keep the first. At the i-th observed item (one-based), replace the reservoir with probability 1/i. By induction, an earlier element remains chosen after step i with probability (1/(i−1))×((i−1)/i)=1/i, while the newest element is selected with probability 1/i. Every item thus has equal probability when the stream ends. The proof assumes the random choice really implements the intended distribution.

**Fisher–Yates shuffle** visits positions and swaps with a uniformly selected eligible position, producing a uniformly random permutation under ideal uniform independent random choices. If the random-index procedure is biased, the permutation can be biased. The Phase 5 lab invokes std::shuffle with a seeded generator and checks that it preserves the input multiset; that does not empirically certify an exact uniform distribution. A rigorous implementation audit must also check index distribution, generator assumptions and seed handling.

### An LRU cache and a sparse set reconnect algorithms to engine ownership

The streaming system may keep only K active mesh descriptions in a memory cache. A **Least Recently Used (LRU)** cache evicts the entry that has not been accessed for the longest time under the cache's declared observation model. An ordered linked list can record recency, and a hash table can map keys to list iterators. On a hit, splice the existing node to the front; on insertion when full, remove the tail and erase its index entry. With appropriate hashing and container guarantees, lookup and recency adjustment are expected O(1). The laboratory exercises capacity two: access key one before inserting key three, then verify key two is evicted.

LRU is a cache policy, not a proof of optimal eviction. A sequential scan of many one-use resources can evict genuinely useful entries; LFU, segmented LRU, clock or application-defined priority may suit different workloads. Caches also need concurrency, invalidation, stale value policy and versioning. Returning an owned snapshot avoids dangling references but can still return stale data unless updates invalidate or version the cache.

A **sparse set** is valuable for entity IDs drawn from a sparse universe when most active components should be iterated densely. A sparse array maps an entity index to a dense position, while a dense array stores the active entity IDs and component values. Lookup checks bounds and verifies that the dense entry at the candidate position really belongs to the queried ID; otherwise an old sparse position can falsely report membership after swap-remove. Removing one entity often swaps the final dense element into the hole and updates its sparse index. This keeps iteration dense but invalidates positional references, explaining the need for logical handles.

**Generational-handle tables** add a generation check to physical slots so that a stale ID cannot silently refer to a newly created entity occupying the old slot. This continues M015/M018 rather than duplicating it into an unrelated "data structures" lesson. The API must still define generation wrap, instance/pool identity, concurrent readers, serialization and resource retirement. The current Phase 5 lab does not replace the existing tested M015/M018 generational registry; instead it relies on that earlier evidence and concentrates new tests on lookup, graph, scan and cache contracts.

### The final choice is an experiment grounded in an operation distribution

We can finally choose a structure without name recognition. The decision begins with the workload: how many assets exist, what fraction are inserted/deleted each frame, how frequently are exact lookups versus ordered range queries performed, how dense are graph edges, and what are the lifetime and synchronization requirements? We then identify correctness invariants and complexity bounds, build a representative input generator, measure relevant costs and validate against an independent oracle.

For exact name lookup with frequent mutation and no ordered iteration, a hash table might be appropriate; for ordered traversal and predictable worst-case comparisons, a balanced tree; for fixed small integer IDs, a bitset or direct-index table; for sparse components with dense iteration, a sparse set; for immutable spatial ordering, a sorted Morton-key array; for small, dense all-pairs routes, Floyd–Warshall; for large nonnegative weighted paths, Dijkstra or justified A*; for unweighted hop counts, BFS. Each choice includes memory footprint, cache behavior, invalidation and update cost, not just one asymptotic number.

A proper benchmark should vary n, load factor, hit/miss ratio, insertion/deletion rate, key distribution, graph density and spatial locality. Hash tables must be tested with collisions and tombstones; trees with adversarial insertion/removal order; graph algorithms with disconnected vertices, cycles and invalid weights; scans with zero counts, empty input and overflow; randomized methods with deterministic reference seeds and statistical checks; all algorithms with appropriate sanitizer and property tests. One medium-sized random workload is not enough to establish a production engine's requirements.

**Phase 5 authored content is now connected end to end, Parts 401–500.** The first half derived the cost and invariants of arrays, queues, hashing, trees, heaps and disjoint sets from the asset catalog. The second half used the same world to explain traversal, pathfinding, sorting, spatial keys, scans, DP and caching. The runnable lab checks selected algorithms, including deterministic randomized differential comparison between independently implemented shortest-path approaches. It is not a claim to have implemented every tree variant, every GPU scan or the current KairoSpatial classes. Those remaining validation requirements are recorded as such.

**Research/source comparison:** MIT OCW [6.006 algorithms lectures, 2020](https://ocw.mit.edu/courses/6-006-introduction-to-algorithms-spring-2020/video_galleries/lecture-videos/) explicitly includes BFS, DFS, Dijkstra, Bellman–Ford, heaps, hashing, dynamic arrays and DP. The older [2011 6.006 lectures](https://ocw.mit.edu/courses/6-006-introduction-to-algorithms-fall-2011/video_galleries/lecture-videos/) cover open addressing, table doubling and additional classical algorithm variants. We verified the course inventories, not complete captions. The supplied *Game Engine Architecture* and existing engine narrative motivate sparse sets, resource managers, scene graphs and navigable worlds. No claim about currently shipping KairoSpatial/AStar, BVHBuilder or NavigationGraph code is derived merely from those sources.

## An API is a promise about an operation

A programmer encounters an unfamiliar function:

```cpp
TextureHandle load_texture(std::string_view path);
```

The declaration gives useful information, but it leaves essential questions unanswered. Does the function finish the upload before returning? Is the path retained? Does a failed load return a distinguished handle, throw, or terminate? Which thread may call it? Who eventually releases the texture? Can a returned handle become invalid after hot reload?

An **application programming interface**, or **API**, is the exposed set of operations and the contracts that make those operations usable. Its spelling is only one part of the interface. A **contract** states what a caller must provide, what the operation promises, and what happens when the operation cannot deliver its normal result.

Start documentation with a caller's situation. For a texture loader, the caller wants a texture that can safely participate in a future draw. Explain whether the operation is synchronous or asynchronous, and define what “ready” means before introducing implementation classes. A CPU image decoded into memory, an upload submitted to a GPU queue, and a texture safe for a particular shader read are three distinguishable states.

### State the contract at the boundary

Use this order when teaching or documenting an operation:

1. Describe the intended result in one ordinary sentence.
2. Define the meaning, units, and validity of each input.
3. Identify ownership and the lifetime of any borrowed data.
4. Explain the successful result and the point at which it becomes usable.
5. Explain failure, cancellation, and partial side effects.
6. State thread, reentrancy, and synchronization requirements. Reentrant means another call is permitted before the earlier call finishes, for example through a callback; thread-safe means the stated concurrent uses are supported. Neither word supplies an unspecified ownership rule.
7. Give a small valid example and a changed-input failure trace.

These are parallel parts of the same contract, not a demand for seven boilerplate headings on every trivial accessor. A short function can have a short contract. A deferred operation involving files, memory, workers, GPU submission, and callbacks usually needs more explanation.

Consider the path argument. A `std::string_view` is a borrowed view of characters. If the loader stores the view for a worker that runs later, the caller's characters must still exist when that worker reads them. An API can instead copy the path into owned storage before returning. Its parameter type alone does not tell us which behavior it implements. A useful description says exactly which object retains the characters and for how long.

Failure guarantees are equally concrete. If decoding fails, does the cache remain unchanged? If decoding succeeds but GPU allocation fails, are temporary CPU bytes released? If uploading succeeds but [[Concepts/Interface contracts and publication|publication]] fails, who retires the GPU resource? “Returns an error” is incomplete when the operation can leave a half-published entry behind.

### Publication is the moment other code may rely on a result

**Publication** means making a result visible to its intended consumers under the required synchronization and validity rules. In a simple single-threaded loader, publication might be inserting a fully constructed texture into a resource table. In an asynchronous loader, it may require a completion message, a state transition, and a rule for when GPU consumers are allowed to read the uploaded data.

For a teaching implementation, imagine the states `Absent`, `Loading`, `Ready`, and `Failed`. A request changes `Absent` to `Loading`. A worker decodes the source image. The renderer's upload path creates the GPU resource. Only after the chosen readiness protocol has been established does the resource manager publish `Ready`. The exact readiness protocol depends on the backend; a Vulkan implementation must specify its execution and memory dependencies rather than relying on the word “upload.”

Cancellation introduces a race worth tracing. Request A starts for scene generation 12. The user closes the scene, destroying generation 12, and opens generation 13. A's worker finishes. If its completion callback blindly publishes into the new scene, cancellation has failed even though every allocation was released correctly. The completion path must validate the request's destination identity or carry an ownership mechanism that makes the obsolete destination unavailable. Cancellation is a state transition whose winner must be defined; it is not merely a Boolean checked at the beginning of a long job.

```mermaid
flowchart LR
    R[Request with destination identity] --> W[Worker creates private result]
    W --> V[Validate destination and readiness]
    V -->|still valid| P[Publish Ready]
    V -->|obsolete or failed| C[Release or retire private result]
```

This same pattern appears in scene loading, shader compilation, pathfinding, image readback, and editor thumbnail generation. The payload changes, but the questions remain: who owns the unfinished result, who may publish it, and what makes its destination still valid?

### A buffer name, an API object, and its storage are separate

A returned integer can name several different stages of a resource's life. In OpenGL, glGenBuffers reserves names; the first glBindBuffer creates the corresponding object state. glCreateBuffers instead creates initialized objects directly. An initialized buffer begins with a zero-sized store. A separate storage operation establishes its byte capacity. Direct state access means naming the object in the command rather than first selecting it through a binding target. These are distinct API promises, not observations of a particular driver's physical allocation strategy.

For an OpenGL 4.5 context with loaded entry points, this fragment expresses two transitions:

```cpp
GLuint buffer = 0;
glCreateBuffers(1, &buffer);  // Initialized object; byte capacity is still zero.
glNamedBufferStorage(buffer, 256, nullptr, 0); // Define a 256-byte store.
```

The null data pointer does not initialize those bytes to zero; their contents are undefined. Immutable storage prevents replacing the store through the ordinary reallocation commands, not every permitted change to its contents. Allowed writes depend on flags and operations. These contracts are specified in [OpenGL 4.5 Core, sections 6.1–6.2](https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf).

For a wrapper design, track identity, storage extent, initialized ranges, and pending users separately. A nonzero handle is not proof that a vertex fetch has a valid range or initialized data. This also explains why the source lecture's phrase about creating everything on the GPU should not become a general allocation guarantee. The source is an explanatory recording; this context-dependent fragment was checked against the specification, not executed in a native OpenGL context. Its API-dependent calls stay beside the explanation, while the ownership and range rules connect to [[Concepts/Object identity and generation handles]] and [[Concepts/Matrix storage indexing and strides]].

### Destroying a C++ wrapper may not finish external work

[[Concepts/RAII and resource handles|RAII]] ties resource-management actions to object lifetime. A wrapper's destructor can reliably perform the action its design promises. It cannot make already-submitted asynchronous consumers disappear by magic.

Suppose the CPU submits a draw that refers to texture T. The owning wrapper then reaches the end of its C++ scope. Immediate destruction of T's backend allocation would be wrong if the GPU may still read it. A wrapper can instead hand T to a retirement mechanism that releases it after the chosen completion condition. That mechanism needs its own owner, and the engine's shutdown order must let the retirement system finish before destroying the device.

The language lifetime rule and the external completion rule meet here. C++ specifies when objects begin and end their lifetimes; the graphics API specifies the conditions for valid resource use. Treating either layer as a replacement for the other leaves a hole in the design. The [C++ lifetime clauses](https://eel.is/c++draft/basic.life) describe the language side. The [Khronos synchronization examples](https://docs.vulkan.org/guide/latest/synchronization_examples.html) give concrete dependency patterns for the API side.

### Error types should preserve the caller's decision

A useful error result distinguishes failures that lead to different actions. A missing source file might prompt the editor to offer a replacement path. An unsupported pixel format might lead to a conversion or a clear rejection. A device-lost error requires a different recovery path from a bad asset name.

Avoid inventing a large error taxonomy before the program has meaningful recovery behavior. Start with the distinctions the caller can act on, and keep diagnostic detail separate from the stable category. Logging an error does not communicate it to a caller that must decide whether to retry, substitute a fallback, or abort a transaction.

For partial operations, state what remains valid. A scene loader can build a candidate scene privately and publish it only after validation succeeds. If a referenced material is missing, it may reject the candidate while leaving the active scene intact. Alternatively, it may explicitly permit a partially loaded scene with fallback materials. Both policies can be coherent. The API must say which world the caller has after failure.

## A complete handle contract in a small program

The following constructed laboratory makes identity observable without returning a borrowed component address. `Handle` is meaningful only inside the particular `Values` instance that issued it; it is not a persistent asset ID or a globally unique name. Calls are single-threaded, or externally synchronized. Returning an `optional<int>` copies a small value, so a later vector relocation cannot invalidate the returned result.

A slot is either live, on an intrusive free list, or retired. Destroying a live value advances its generation before reuse. The generation never wraps: a slot at the maximum generation is retired when destroyed. The sample's normal tests exercise stale-handle rejection, invalid indices, reuse and independent replacement identity. Reaching the 64-bit retirement limit is not dynamically tested by this small program.

```cpp
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

struct Handle {
    std::uint32_t index;
    std::uint64_t generation;
};

class Values {
    static constexpr auto none = std::numeric_limits<std::uint32_t>::max();
    struct Slot {
        std::uint64_t generation = 1;
        std::optional<int> value;
        std::uint32_t next = none;
    };
    std::vector<Slot> slots;
    std::uint32_t free_head = none;
public:
    bool alive(Handle h) const noexcept {
        return h.index < slots.size() && slots[h.index].value.has_value()
            && slots[h.index].generation == h.generation;
    }
    Handle create(int value) {
        if (free_head != none) {
            const auto index = free_head;
            Slot& slot = slots[index];
            const auto next = slot.next;
            slot.value = value;
            slot.next = none;
            free_head = next;
            return {index, slot.generation};
        }
        if (slots.size() >= none)
            throw std::length_error("Handle index space exhausted");
        const auto index = static_cast<std::uint32_t>(slots.size());
        slots.push_back(Slot{1, value, none});
        return {index, 1};
    }
    bool destroy(Handle h) noexcept {
        if (!alive(h)) return false;
        Slot& slot = slots[h.index];
        slot.value.reset();
        if (slot.generation == std::numeric_limits<std::uint64_t>::max()) {
            slot.next = none; // Retire this slot; never wrap its generation.
        } else {
            ++slot.generation;
            slot.next = free_head;
            free_head = h.index;
        }
        return true;
    }
    std::optional<int> get(Handle h) const noexcept {
        return alive(h) ? slots[h.index].value : std::nullopt;
    }
};

int main() {
    Values values;
    const Handle first = values.create(42);
    assert(values.get(first) == 42);
    assert(values.destroy(first));
    assert(!values.get(first));
    const Handle replacement = values.create(7);
    assert(first.index == replacement.index);
    assert(first.generation != replacement.generation);
    assert(!values.alive(first));
    assert(!values.destroy(first));
    assert(values.get(replacement) == 7);
    const Handle invalid{std::numeric_limits<std::uint32_t>::max(), 1};
    assert(!values.get(invalid));
    assert(values.destroy(replacement));
    assert(!values.alive(replacement));
}
```

The short-circuit order in `alive` matters: it checks the index before reading the slot. Destroying a stale handle leaves its replacement intact. The free list is stored inside vacant slots, so destruction does not need to allocate a separate free-list entry. Creation can allocate when growing the vector and can throw; no new handle is published until that insertion succeeds. This example deliberately does not address cross-instance handles, concurrent access, serialization or GPU completion.

**Executed check:** this exact body compiled as C++20 with `-Wall -Wextra -Werror -fsanitize=address,undefined` and completed its assertions with no sanitizer diagnostics. That validates the exercised laboratory, not an unrelated ECS or renderer implementation.

## The build graph becomes another interface

### A capability exists only where the complete resource contract permits it

Kohi's uniform-buffer fix is a useful portability example: a desired combination of host-visible and device-local memory was not available on every viewer's device. The episode relaxes one path's requirements and conditionally prefers that combination on another. This is an implementation-specific fix, not a rule that frequently changed buffers must never use device-local memory. [Kohi uniform-buffer correction](https://www.youtube.com/watch?v=aD6fZ7tQSAY)

Host-visible memory permits CPU mapping. Device-local identifies memory local to the device; a type may have both properties. Host-coherent concerns cache visibility, not whether GPU work has finished. A descriptor layout declares binding types and counts; a descriptor pool provides storage for allocated sets.

A memory type's property flags must meet the allocation's requirements, and that type must also be included in the particular resource's `memoryTypeBits`. A device-wide Boolean saying “some type has these properties” is not sufficient to establish compatibility for every resource. Separate required properties from preferences, and define what happens when no acceptable type exists. CPU mapping, noncoherent flush/invalidate operations and GPU completion are separate contracts. [Vulkan memory-type selection](https://docs.vulkan.org/spec/latest/chapters/memory.html)

Descriptors add a different lifetime and update contract. The historical crash-fix episode moves ordinary descriptor updates before binding and allocates separate uniform regions for overlapping frames. A feature being supported by hardware does not itself enable its use: the relevant feature, descriptor-layout/pool configuration and binding rules must agree. Update-after-bind does not mean arbitrary concurrent updates to every in-use descriptor are safe. Keep the ordinary update-before-use path correct before adding optional relaxed behavior. [Kohi descriptor correction](https://www.youtube.com/watch?v=U4QvUeUL37Y), [Khronos descriptor binding rules](https://docs.vulkan.org/spec/latest/chapters/descriptorsets.html)

The frame's descriptor, uniform byte range, command buffer and completion record must refer to the same reuse policy. An acquired swapchain image index and a CPU frame-slot index are different identifiers; matching them by accident on one configuration is not a general design. A per-image or per-frame region is reusable when the work using that region has completed, not merely when a modulo counter repeats. The engine chapter develops ownership across frames; this is the API-level condition it must preserve.

A region offset must also obey the API's applicable alignment limits. For record size $b$ and required alignment $a>0$, a padded stride is $s=a\lceil b/a\rceil$. A 160-byte record with a queried 256-byte offset alignment uses offsets $0,256,512$ for three regions, not $0,160,320$. This is an illustrative limit value, not a universal Vulkan constant. The actual offset/range rules depend on the binding operation and resource type.

### Packaging removes an editor dependency, not every runtime dependency

The Java export episode demonstrates packaging a project as a runnable JAR and launching it outside its IDE. A JAR is an archive used for Java classes and resources. The configured entry point tells the runtime which application to start. Exporting does not make the program independent of a compatible Java runtime, native libraries, expected resources, permissions or platform behavior. The captions explicitly say the presenter did not test Linux there; an extracted claim saying Windows was untested was wrong. [Source export episode](https://www.youtube.com/watch?v=velhSXA2CnQ)

The transferable test is to launch the packaged application from a clean directory or machine with only its declared runtime requirements. Record which assets are bundled, which paths are resolved relative to a working directory, and how startup failures become visible without an IDE console. A successful export and a successful launch are distinct observations. A historical announcement of a future Android port likewise establishes intent, not a completed port or a recommendation to use that period's toolchain today.


## Make a shader record an explicit binary contract

The CPU writes bytes; the shader interprets them using offsets, strides, scalar widths and layout rules. A [[Concepts/Shader buffer ABI|shader ABI]], or application binary interface, is that agreement at the byte boundary. A C++ field name does not communicate its intended offset to a GPU. The same is true of a C++ `bool`: its representation is not an automatic portable shader representation. Prefer an explicit integer flag and verify the complete packed record.

Alignment is the set of byte addresses at which a value may begin. Size is how much storage represents the value. Array stride is the distance from the start of one element to the next. These quantities can differ. Under ordinary extended-alignment `std140` rules, a `vec3` begins at a 16-byte-aligned offset but its three float components occupy 12 bytes. A following scalar can occupy the fourth slot when the layout rules permit it. Treating every `vec3` as an unconditional 16-byte payload gives wrong predictions for some legal structures. Inspect compiler-generated member offsets and array strides, and assert the corresponding CPU offsets.

For the GLSL block `float exposure; vec3 color; float roughness; vec2 uvScale;`, the ordinary member offsets are $0,16,28,32$ and the block's rounded structure extent is 48 bytes. This is a layout calculation, not proof that an arbitrary C++ type has that representation. Arrays of floats have stride 16 under `std140` and stride 4 under ordinary base-alignment `std430`. Vulkan's permitted uniform-block layouts depend on enabled features; feature promotion to a core API version does not make every optional feature automatically enabled. [Khronos shader memory layout guide](https://docs.vulkan.org/guide/latest/shader_memory_layout.html)

Texture descriptor indexing changes how a shader finds an image; it does not remove resource lifetime or synchronization obligations. A descriptor is a binding record. An index into a table is an identifier. Neither means the allocation remains alive until a pending draw finishes. Track generation, ownership and completion independently. The material-facing interpretation remains in [[Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport#Ask a surface where incoming energy can go|Ask a surface where incoming energy can go]].

## Let many invocations cooperate without inventing a global clock

A GPU dispatch launches many invocations: individual executions of a kernel. A workgroup is a set of invocations that can use a common shared-memory region and workgroup synchronization. A subgroup is a smaller execution collective exposed by an API. NVIDIA often calls its hardware collective a warp, while other vendors use wave terminology. Width and supported operations must be queried or constrained; 32 lanes is an example, not a portable constant.

SIMT means single instruction, multiple threads. Many logical invocations execute instructions through grouped hardware. If lanes disagree on a branch, hardware may execute different paths with different active-lane masks. This is divergence. Its cost depends on the actual paths, reconvergence, compiler lowering and hardware. Removing a useful branch can increase work, so measure the kernel rather than memorizing that every branch is expensive.

Latency is how long an individual operation takes. Throughput is how much work completes per unit time. A GPU can hide a memory wait by issuing ready work from another resident group. Occupancy measures the fraction of a relevant hardware residency limit occupied by active work; it is not a direct measure of useful work completed. Registers, shared memory, group size and implementation-specific allocation granularity constrain residency. More occupancy can help a latency-bound kernel and still lose to a lower-occupancy kernel that performs less work.

Coalescing combines suitable nearby lane accesses into fewer memory transactions. Shared-memory bank conflicts arise when concurrent accesses need conflicting service from the same hardware bank. These are different mechanisms. A padded transpose tile can remove a bank conflict while leaving global access unchanged. Texture caches and unified physical memory likewise do not remove the need to reason about access patterns.

### Scan turns local counts into nonoverlapping output regions

Suppose four visible objects emit $c=(2,0,3,1)$ records. The [[Concepts/Prefix scan and stream compaction|exclusive prefix sum]] is $o=(0,2,2,5)$. Object $i$ owns the interval $[o_i,o_i+c_i)$. These intervals do not overlap; their total output size is six. This explains why scan appears in visibility compaction, mesh generation, radix sorting and variable-length expansion. The mathematics is an allocation proof, not just a trick for adding numbers.

For large arrays, first scan blocks locally, scan their totals, then add each block's offset. If all workgroups cannot synchronize within the dispatch, use separate dispatch phases with explicit dependencies. A dispatch boundary by itself is not a universal memory-visibility guarantee. The consumer must wait for the relevant producer writes in the API's execution and memory model. In Vulkan, stage and access scopes identify those producer and consumer operations. [Khronos synchronization specification](https://docs.vulkan.org/spec/latest/chapters/synchronization.html)

```mermaid
flowchart LR
  A["Counts: 2, 0, 3, 1"] --> B["Exclusive scan: 0, 2, 2, 5"] --> C["Owned ranges: two, zero, three, one"] --> D["Scatter six records"]
  classDef allocation fill:#fff1dc,stroke:#a46a24,color:#55340f;
  class A,B,C,D allocation;
```

Atomicity, ordering and rendezvous answer different questions. An atomic addition prevents that counter update from being torn or lost. Memory ordering governs visibility and order of related operations. A workgroup barrier makes the participating invocations meet at a synchronization point and establishes the specified memory effects. An atomic counter does not by itself establish that unrelated payload writes are ready for a reader. A workgroup barrier does not make another workgroup finish.

Consider a final partially filled group in a tiled matrix multiply. Some lanes represent out-of-range matrix elements. They should write neutral zero values into their assigned shared slots and still participate in every required group barrier. Returning before a barrier that the other lanes reach can violate the synchronization contract. After consuming a shared tile, another barrier may be needed before any lane overwrites it for the next iteration. The first barrier protects consumers from incomplete loads; the second protects slower consumers from premature reuse.

### Load a partial matrix tile with two different bounds

The supplied tiled multiplication excerpt uses a placeholder `valid` for both input loads. That placeholder is not a sufficient complete bounds rule. Let the left matrix have shape $M\times K$ and the right matrix shape $K\times N$. For tile width $T$, row $r$, column $c$, inner-dimension base $b$, and local coordinates $(l_x,l_y)$, the two legal-load predicates are

$$
\text{left valid}:\quad r<M\ \land\ b+l_x<K,
\qquad
\text{right valid}:\quad b+l_y<K\ \land\ c<N.
$$

A thread whose output column is out of range can still need to load a left-hand element for another thread's valid output. Similarly, an out-of-range output row does not by itself make its right-hand load unnecessary. Mask the two loads separately, write zero for each missing element, and let every participating thread reach both barriers. Finally store only when $r<M$ and $c<N$. A zero-padded last tile makes the full inner tile loop valid without reading past either input.

```metal
// Illustrative tile body, with T x T participating threads.
// lhsTile and rhsTile are threadgroup arrays declared by the kernel.
for (uint base = 0; base < K; base += T) {
    lhsTile[local.y][local.x] =
        (row < M && base + local.x < K)
        ? lhs[row * K + base + local.x] : 0.0f;
    rhsTile[local.y][local.x] =
        (base + local.y < K && column < N)
        ? rhs[(base + local.y) * N + column] : 0.0f;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    for (uint j = 0; j < T; ++j)
        sum += lhsTile[local.y][j] * rhsTile[j][local.x];
    threadgroup_barrier(mem_flags::mem_threadgroup);
}
if (row < M && column < N)
    output[row * N + column] = sum;
```

This is a kernel fragment with explicit assumptions, not a standalone compiled Metal program. CPU emulation of its load masks and arithmetic is recorded separately; that check does not establish Metal compilation, GPU synchronization or device behavior.

The following supplied GPU lessons retain implementation-specific examples as source observations. Their Kairo and Maveb claims describe the supplied snapshot; this integration has not established that they match today's repositories or tested their kernels on a GPU.


Implementation-specific passages retain observations from the supplied Kairo/Maveb snapshot. They are not a fresh audit of those repositories. Source exercises and illustrative code are teaching material; only explicitly recorded checks have been executed here.

**Material representation has several layers**

A healthy engine should distinguish at least these domains:

```text
authoring/interchange material
        ↓
persistent engine artifact
        ↓
runtime renderer-neutral material
        ↓
GPU material record
        ↓
shader representation
```

These do **not** need to have identical C++ structs.

For Kairo today:

```text
glTF material
    ↓
GltfMaterialData
    ↓
MakeGltfPBRMaterial()
    ↓
PBRMaterial
    ↓
MaterialUniform + descriptors
    ↓
GLSL MaterialData
```

That is already the correct broad layering.

The danger begins when one layer's ABI accidentally leaks into another.

---

**GPU data layout is an ABI**

Suppose C++ contains:

```cpp
struct Material
{
    Vec3 baseColor;
    float roughness;
    float metallic;
};
```

and GLSL says:

```glsl
layout(std140) uniform Material
{
    vec3 baseColor;
    float roughness;
    float metallic;
};
```

You cannot assume:

```text
same field names
→ same bytes.
```

CPU and GPU must agree on:

$$
\text{offset},
$$

$$
\text{alignment},
$$

$$
\text{stride},
$$

$$
\text{matrix order},
$$

$$
\text{scalar size}.
$$

This is a binary ABI contract.

---

**Alignment versus size**

These are different concepts.

A scalar `float` has size:

$$
4
$$

bytes.

Its alignment is typically:

$$
4.
$$

A `vec4` has:

$$
16\text{-byte size}
$$

and:

$$
16\text{-byte alignment}.
$$

A `vec3` contains 12 bytes of useful numeric data but frequently carries 16-byte alignment requirements in GPU interface layouts.

That mismatch is where many CPU/GPU struct bugs begin.

---

**std140**

`std140` is the traditional extended-alignment layout used especially for uniform blocks.

The broad rules to internalize are:

| Type | Typical std140 base alignment |
|---|---:|
| `float` / `int` | 4 |
| `vec2` | 8 |
| `vec3` | 16 |
| `vec4` | 16 |
| arrays | element alignment/stride rounded to vec4-like 16-byte boundaries |
| structs | maximum member alignment, rounded to a 16-byte boundary |
| matrices | treated essentially as aligned vector arrays |

Vulkan documentation refers to std140 as **extended alignment**. [docs.vulkan.org](https://docs.vulkan.org/guide/latest/shader_memory_layout.html?utm_source=chatgpt.com)

The primary characteristic is conservative 16-byte padding for aggregates such as arrays and structures.

---

**Example std140 struct**

Consider:

```glsl
layout(std140) uniform Example
{
    float exposure;
    vec3  color;
    float roughness;
    vec2  uvScale;
};
```

Start offset:

$$
0.
$$

`exposure`:

$$
\text{offset}=0,\quad\text{size}=4.
$$

Next `vec3` requires:

$$
16\text{-byte alignment}.
$$

So we jump:

$$
4\rightarrow16.
$$

`color` begins at:

$$
16.
$$

This immediately introduces 12 bytes between the first float and the vector.

Depending on exact member-layout rules and representation, relying on packing around `vec3` is notoriously error-prone.

A much safer CPU-side shape is:

```cpp
struct alignas(16) MaterialGPU
{
    std::array<float, 4> colorAndExposure;
    std::array<float, 4> params;
};
```

which exactly mirrors:

```glsl
vec4 colorAndExposure;
vec4 params;
```

---

**Why Kairo's current material block is smart**

Kairo shader:

```glsl
layout(set = 0, binding = 7) uniform MaterialData
{
    vec4 baseColorFactor;
    vec4 emissiveAndNormalScale;
    vec4 factors;
    vec4 flags;
} material;
```

That is:

$$
4\times16=64\text{ bytes}.
$$

CPU:

```cpp
struct MaterialUniform
{
    std::array<float, 16> Values{};
};
```

also:

$$
16\times4=64\text{ bytes}.
$$

There are no:

- `vec3` packing questions,
- bool representation questions,
- irregular offsets,
- nested arrays.

This is deliberately boring, which is excellent for an ABI.

---

**Kairo's actual 64-byte mapping**

Current packing is essentially:

$$
\texttt{Values[0..3]}
=
(\text{baseRGB},\alpha).
$$

$$
\texttt{Values[4..7]}
=
(\text{emissiveRGB},\text{normalScale}).
$$

$$
\texttt{Values[8..11]}
=
(\text{metallic},
\text{roughness},
\text{AO},
\text{alphaCutoff}).
$$

$$
\texttt{Values[12..15]}
=
(\text{alphaMode},
\text{doubleSided},
\text{unused},\text{unused}).
$$

That maps exactly onto four shader `vec4`s.

This is a very robust GPU-data layout.

---

**Why GPU-side booleans are awkward**

A CPU `bool` may occupy:

$$
1\text{ byte}.
$$

A shader boolean in an interface block may obey entirely different representation/alignment requirements.

Therefore sending:

```cpp
bool doubleSided;
```

directly into a raw GPU buffer is risky.

Kairo instead converts:

```cpp
DoubleSided
```

into:

$$
0.0\text{ or }1.0
$$

inside a float slot.

This is simple and portable.

For packed material tables, integers/bitfields are also common.

---

**std430**

`std430` is Vulkan/GLSL's **base alignment** layout.

It relaxes some std140 aggregate padding.

The biggest practical difference is arrays and structures of scalar/small-vector values.

For example, under std140:

```glsl
float values[4];
```

commonly has:

$$
16\text{-byte stride}.
$$

Thus four floats may consume:

$$
64\text{ bytes}.
$$

Under std430:

$$
\text{stride}=4
$$

is possible, so the same array uses:

$$
16\text{ bytes}.
$$

Vulkan documents std430 as base alignment, and Vulkan 1.2's `uniformBufferStandardLayout` feature allows std430-style layout for uniform buffers as well; storage buffers commonly use it already. [docs.vulkan.org](https://docs.vulkan.org/guide/latest/shader_memory_layout.html?utm_source=chatgpt.com)

---

**Why std430 matters for large material arrays**

Suppose you want:

$$
100,000
$$

materials.

Each material contains many scalars.

If every scalar/array element is unnecessarily promoted to 16-byte stride, memory waste becomes substantial.

For huge structured material tables, std430 is much more appropriate.

A modern material SSBO could look like:

```glsl
layout(std430, set = 1, binding = 0)
readonly buffer MaterialTable
{
    MaterialGPU materials[];
};
```

Then each draw carries:

$$
\texttt{materialIndex}
$$

instead of rebinding one UBO/descriptor set per material.

That is a major architectural improvement toward GPU-driven rendering.

---

**Example production material GPU record**

A scalable representation might be:

```cpp
struct alignas(16) MaterialGPU
{
    // 16 bytes
    float BaseColorR;
    float BaseColorG;
    float BaseColorB;
    float BaseColorA;

    // 16 bytes
    float EmissiveR;
    float EmissiveG;
    float EmissiveB;
    float NormalScale;

    // 16 bytes
    float Metallic;
    float Roughness;
    float AO;
    float AlphaCutoff;

    // 16 bytes
    uint32_t BaseColorTexture;
    uint32_t NormalTexture;
    uint32_t MetallicRoughnessTexture;
    uint32_t EmissiveTexture;

    // 16 bytes
    uint32_t OcclusionTexture;
    uint32_t Flags;
    uint32_t TexCoordMask;
    uint32_t Reserved;
};
```

Total:

$$
80\text{ bytes}.
$$

The key change is that textures are no longer Vulkan handles inside the struct.

They are **indices into a resource table**.

That is the bridge to bindless rendering.

---

**Padding is part of the format**

Suppose CPU believes:

```text
roughness offset = 12
```

but shader believes:

```text
roughness offset = 16.
```

Then the shader does not get a subtly inaccurate roughness.

It reads **different bytes**.

A float bit pattern from some unrelated field may become:

$$
-3.4\times10^{20}
$$

or:

$$
NaN.
$$

This can then propagate into:

$$
D_{\text{GGX}}
$$

and the entire framebuffer.

So GPU layout mismatches are not “serialization bugs.”

They are shader execution corruption.

---

**Compile-time layout defense**

For explicit CPU GPU structures, use assertions:

```cpp
struct alignas(16) MaterialGPU
{
    float BaseColor[4];
    float EmissiveNormal[4];
    float Factors[4];
    float Flags[4];
};

static_assert(
    sizeof(MaterialGPU) == 64);

static_assert(
    alignof(MaterialGPU) == 16);

static_assert(
    offsetof(MaterialGPU, BaseColor) == 0);

static_assert(
    offsetof(MaterialGPU, EmissiveNormal) == 16);

static_assert(
    offsetof(MaterialGPU, Factors) == 32);

static_assert(
    offsetof(MaterialGPU, Flags) == 48);
```

That turns a silent rendering bug into a compile-time failure.

Even better, generate CPU/shader layouts from one schema where possible.

---

**Why `std::array<float,16>` is robust but not expressive**

Kairo currently avoids ABI mistakes with:

```cpp
std::array<float,16>
```

which is good.

But:

```cpp
uniform.Values[10]
```

does not explain itself.

The long-term better pattern is to retain both:

$$
\text{explicit semantic field names}
$$

and:

$$
\text{layout guarantees}.
$$

For example:

```cpp
struct alignas(16) MaterialGPU
{
    Vec4f BaseColorFactor;
    Vec4f EmissiveNormalScale;
    Vec4f Factors;
    Vec4f Flags;
};
```

provided `Vec4f` is explicitly guaranteed to be 16 bytes/aligned as needed.

Then shader and CPU become mechanically readable.

---

**Bindless: what problem are we solving?**

Current Kairo has approximately:

```text
draw 0
→ bind descriptor set for material 0
→ draw

draw 1
→ bind descriptor set for material 1
→ draw

draw 2
→ bind descriptor set for material 2
→ draw
```

Every set contains several texture descriptors plus material UBO state.

This is perfectly reasonable for a compact renderer.

But at very large scene scales, explicit per-material rebinding becomes expensive and complicates GPU-driven submission.

Bindless-style rendering changes the model to:

```text
one large texture/resource table
+
material contains integer indices
+
shader dynamically chooses resources
```

---

**Bindless conceptual model**

Suppose:

```glsl
layout(set = 0, binding = 0)
uniform sampler2D textures[];
```

Material record:

```glsl
struct MaterialGPU
{
    uint baseColorTexture;
    uint normalTexture;
    uint mrTexture;
};
```

Then:

```glsl
MaterialGPU m =
    materials[materialIndex];

vec4 base =
    texture(
        textures[
            nonuniformEXT(
                m.baseColorTexture)],
        uv);
```

Now thousands of materials can coexist without rebinding a different descriptor set for every draw.

---

**Why `nonuniformEXT` exists**

Imagine one GPU wave processes fragments from different materials.

Lane 0 chooses texture:

$$
17
$$

while lane 1 chooses:

$$
42.
$$

The descriptor index is not uniform across the wave.

Vulkan descriptor indexing explicitly provides support for non-uniform dynamic indexing of descriptor arrays, along with runtime arrays, partially bound arrays and update-after-bind behavior. [docs.vulkan.org](https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_descriptor_indexing.html?utm_source=chatgpt.com)

The shader/feature declarations must tell the implementation that indexing can differ across invocations.

This is not merely syntax.

It changes assumptions the compiler may make about descriptor selection.

---

**Descriptor indexing capabilities**

A Vulkan bindless-like design may rely on capabilities such as:

$$
\texttt{runtimeDescriptorArray},
$$

$$
\texttt{descriptorBindingPartiallyBound},
$$

$$
\texttt{descriptorBindingVariableDescriptorCount},
$$

$$
\texttt{shaderSampledImageArrayNonUniformIndexing},
$$

and possibly:

$$
\texttt{updateAfterBind}.
$$

These features must be queried and enabled; they are not magical properties of declaring `sampler2D[]`. [docs.vulkan.org](https://docs.vulkan.org/guide/latest/extensions/VK_EXT_descriptor_indexing.html?utm_source=chatgpt.com)

---

**Partially bound arrays**

Suppose your shader declares capacity for:

$$
100,000
$$

textures.

But currently only:

$$
11,000
$$

exist.

Without suitable descriptor-indexing behavior, descriptor validity requirements can force every declared slot to be populated.

With partially bound descriptors, unused dynamically unreferenced entries can remain invalid/unpopulated under the extension's rules. [docs.vulkan.org](https://docs.vulkan.org/spec/latest/chapters/descriptorsets.html?utm_source=chatgpt.com)

That makes large resource tables practical.

---

**Update-after-bind**

Ordinarily, descriptor contents are treated as stable while pending GPU work may use them.

Update-after-bind relaxes some of those rules.

This is useful for streaming:

```text
frame running
    ↓
new texture finishes loading
    ↓
descriptor table slot updated
```

without rebuilding every material.

But this flexibility introduces strict lifetime/synchronization responsibilities. Vulkan's descriptor-indexing documentation emphasizes that descriptor updates and resource lifetime still need precise correctness rules. [docs.vulkan.org](https://docs.vulkan.org/tutorial/latest/Building_a_Simple_Engine/Advanced_Topics/Descriptor_Indexing_UpdateAfterBind.html?utm_source=chatgpt.com)

“Bindless” does not mean “lifetime-less.”

---

**A bindless handle is not ownership**

Suppose material stores:

$$
\text{textureIndex}=317.
$$

If texture 317 is destroyed while a command buffer still references that index, the integer remains 317.

It has not become safe.

A scalable renderer therefore needs some policy around:

- descriptor-slot lifetime,
- deferred destruction,
- frame fences,
- generation counters,
- residency,
- streaming.

The hard part of bindless architecture is often resource lifetime, not shader syntax.

---

**Descriptor indexing and GPU-driven rendering fit together**

From Pass 2:

GPU-driven rendering wants the GPU to generate draw commands.

If each generated draw requires the CPU to bind a unique material descriptor set, we lose much of the point.

Instead:

```text
draw command
contains materialID
        ↓
shader
loads MaterialGPU[materialID]
        ↓
MaterialGPU
contains texture indices
        ↓
descriptor table
```

Now the entire rendering decision can stay on GPU-visible data.

This is why:

$$
\boxed{
\text{bindless resources}
+
\text{material tables}
+
\text{indirect draws}
}
$$

form such a common modern architecture.

---



### + M129

Phase 22 begins by changing the question again.

In Phase 21 we asked:

$$
\boxed{\text{How do I construct a correct Monte Carlo estimator?}}
$$

Now the question is:

$$
\boxed{\text{How do I make millions of copies of useful work execute efficiently on throughput hardware?}}
$$


The curriculum's own repository anchors are unusually useful here because Maveb already contains a real Metal Gaussian pipeline with **Blelloch scan, subgroup operations, threadgroup barriers, radix histograms, prefix sums and stable key/value scatter**. So this pass is not merely explaining textbook GPU algorithms—we can reverse-engineer code you already have.

---

### GPU EXECUTION ARCHITECTURE

**CPU and GPU optimization start from different goals**

A modern high-performance CPU is strongly optimized for **low latency on relatively few instruction streams**.

It invests substantial silicon in things such as sophisticated branch prediction, speculative/out-of-order execution, large caches, powerful individual cores and mechanisms for extracting instruction-level parallelism from comparatively narrow serial programs.

A GPU spends vastly more of its resources on arithmetic throughput.

Instead of saying:

> Make one thread finish this operation as soon as physically possible.

the GPU philosophy is closer to:

> Keep an enormous number of arithmetic lanes busy, and when some work stalls, immediately run other work.

The distinction is:

$$
\boxed{
\text{CPU}\approx\text{latency-oriented}
}
$$

versus:

$$
\boxed{
\text{GPU}\approx\text{throughput-oriented}.
}
$$

Neither statement is absolute. CPUs have SIMD and many cores; GPUs have caches and sophisticated schedulers. It is the architectural emphasis that differs.

---

**A simple latency versus throughput example**

Imagine a memory operation takes notionally:

$$
400
$$

cycles before the requested data is ready.

A CPU might try to avoid waiting by using caches, prefetchers, out-of-order execution and other independent instructions from the same few threads.

A GPU instead may have many groups of threads ready.

Warp A issues a memory load and stalls.

Rather than waiting 400 cycles, the scheduler can issue:

$$
B,\ C,\ D,\ E,\dots
$$

provided those groups are ready.

Hence:

$$
\boxed{
\text{GPU latency hiding is primarily achieved by having other independent execution groups resident}.
}
$$

This is the origin of the occupancy discussion later.

---

**Compute unit / streaming multiprocessor concept**

Different vendors use different terminology.

NVIDIA commonly talks about an SM, or streaming multiprocessor.

AMD commonly uses compute-unit/wave terminology.

Apple exposes Metal execution abstractions rather than requiring you to reason in exact vendor-internal units.

Generically, call it a **compute unit**.

A compute unit contains some combination of execution lanes, warp/wave scheduling hardware, a large register file, fast shared/threadgroup memory, load/store hardware, special-function units and caches/interfaces to the wider GPU memory system.

Many workgroups can potentially reside on one compute unit simultaneously.

The central hierarchy is:

```text id="5kw7e9"
GPU
    compute units
        resident workgroups
            subgroups / warps / waves
                lanes / invocations
```

These levels are not interchangeable.

---

**SIMT**

GPU shader programming exposes what appears to be many scalar threads:

```cpp id="e937zu"
kernel void Compute(uint id)
{
    output[id] =
        input[id] * 2.0f;
}
```

Each invocation appears independent.

But hardware typically groups several invocations and issues the same instruction across their lanes.

This is **Single Instruction, Multiple Threads**:

$$
\boxed{\text{SIMT}.}
$$

Each lane has its own logical registers and IDs, but the subgroup often shares instruction issue.

---

**SIMT versus CPU SIMD**

This distinction matters.

Your `KairoSIMD` implementation explicitly executes:

$$
8
$$

floats with AVX2:

```cpp id="qpohrx"
_mm256_add_ps(...)
```

and:

$$
16
$$

with AVX-512:

```cpp id="6vk2sf"
_mm512_add_ps(...)
```

There the programmer explicitly says:

$$
\boxed{\text{perform one vector operation across N lanes}.}
$$

On the GPU, you write something that appears scalar:

```metal id="2o38s4"
kernel void add(
    device const float* a,
    device const float* b,
    device float* out,
    uint id [[thread_position_in_grid]])
{
    out[id] =
        a[id] + b[id];
}
```

Hardware groups many such invocations.

So:

$$
\boxed{
\text{SIMD: vector lanes explicit in the programming model}
}
$$

while:

$$
\boxed{
\text{SIMT: scalar-looking invocations grouped by execution hardware}.
}
$$

This distinction explains divergence.

---

**Warp, wavefront and subgroup**

The hardware execution group has different names.

Conceptually:

$$
\boxed{
W=\{\text{lane}_0,\dots,\text{lane}_{S-1}\}.
}
$$

The width:

$$
S
$$

is hardware/platform dependent.

Do not write an engine assuming:

$$
S=32
$$

unless your target guarantees it.

In cross-platform shader terminology, **subgroup** is the safest conceptual name.

Metal calls corresponding operations `simdgroup` operations.

Vulkan has subgroup operations.

CUDA historically talks about warps.

AMD literature commonly talks about wavefronts.

---

**Maveb already exposes this hierarchy explicitly**

Your Gaussian shader takes:

```metal id="z8wxwy"
uint lane
    [[thread_index_in_simdgroup]],

uint simdGroup
    [[simdgroup_index_in_threadgroup]],

uint simdWidth
    [[threads_per_simdgroup]],

uint threadsPerGroup
    [[threads_per_threadgroup]]
```

That means the shader can distinguish:

$$
\text{global invocation},
$$

$$
\text{threadgroup-local invocation},
$$

$$
\text{subgroup},
$$

$$
\text{lane within subgroup}.
$$

That is the complete execution hierarchy M127 is trying to teach.

---

**Invocation IDs**

For a one-dimensional dispatch, global index is conceptually:

$$
\boxed{
i=
gG+l
}
$$

where:

$$
g=\text{workgroup index},
$$

$$
G=\text{threads per workgroup},
$$

$$
l=\text{local thread index}.
$$

GPU APIs usually provide this already as a global invocation ID.

In Metal:

```metal id="0x8pi8"
uint index
    [[thread_position_in_grid]]
```

is exactly that.

---

**KairoGPU's vector kernel is the canonical 1D mapping**

Current KairoGPU:

```metal id="v6nvzv"
kernel void kairo_vector_multiply(
    device const float* lhs,
    device const float* rhs,
    device float* output,
    constant uint& count,
    uint index [[thread_position_in_grid]])
{
    if (index < count)
        output[index] =
            lhs[index] * rhs[index];
}
```

The mapping is perfect:

$$
\boxed{
\text{one invocation}
\leftrightarrow
\text{one array element}.
}
$$

No invocation communicates with another.

This is the easiest class of GPU workload.

---

**Workgroups / threadgroups**

Invocations are organized into workgroups.

For one workgroup:

$$
T=
T_xT_yT_z.
$$

All invocations in the group can generally:

$$
\text{share threadgroup/shared memory}
$$

and:

$$
\text{synchronize at workgroup barriers}.
$$

Different workgroups must be considered independent during one dispatch.

There is no ordinary barrier inside a kernel that says:

> Wait until every workgroup on the entire GPU reaches this point.

That restriction drives many multi-dispatch algorithms.

---

**Why workgroups exist**

Suppose 256 threads need to collaboratively reduce 256 values.

If those threads belong to one workgroup, they can place values into fast threadgroup storage:

```metal id="et2xom"
threadgroup float scratch[256];
```

then synchronize:

```metal id="yx8mk8"
threadgroup_barrier(
    mem_flags::mem_threadgroup);
```

and collaboratively operate on the array.

Without workgroup locality, every intermediate exchange would need slower device memory plus more complicated synchronization.

---

**Dispatch grids**

A dispatch consists of many workgroups or a requested grid of threads.

For a 1D array of $N$ elements with workgroup width $G$, conventional APIs dispatch:

$$
\boxed{
N_g=
\left\lceil
\frac NG
\right\rceil
}
$$

groups.

For example:

$$
N=1000,
\qquad
G=256.
$$

Then:

$$
N_g=
\lceil3.90625\rceil
=
4.
$$

Total threads:

$$
4\times256=1024.
$$

Therefore 24 invocations are outside valid input range and must perform:

```cpp id="9wj2yb"
if (id >= count)
    return;
```

This is called **over-dispatch**.

---

**Metal's `dispatchThreads`**

Metal also provides a dispatch interface where the requested logical thread-grid size is explicit:

```objc id="d2ts06"
dispatchThreads(
    MTLSizeMake(count, 1, 1),
    threadsPerThreadgroup);
```

KairoGPU uses exactly this for vector addition and multiplication.

It still has:

```metal id="ccmw1t"
if (index < count)
```

which is harmless defensive correctness.

---

**Workgroup scheduling**

A workgroup is assigned to some compute unit.

Multiple workgroups may be resident on that unit.

Within them, the hardware schedules subgroups.

The essential scheduling object is usually the subgroup/warp/wave rather than an individual invocation.

Conceptually:

```text id="r4h73i"
cycle:
    choose ready warp
    issue instruction

later:
    warp stalls on memory
    choose another ready warp
```

This explains why GPU performance can remain excellent despite high individual memory latency.

---

**Latency hiding**

Imagine each subgroup performs:

```text id="6d38i1"
load
wait
multiply
store
```

If only one subgroup is resident, memory latency directly stalls the execution unit.

If 16 are resident:

```text id="bnfz56"
warp 0 → load, stalls
warp 1 → execute
warp 2 → execute
...
```

By the time the scheduler returns to warp 0, its load may be ready.

Thus:

$$
\boxed{
\text{latency hiding requires enough independent ready work}.
}
$$

This is one major role of occupancy.

---

**GPU occupancy**

Occupancy is roughly:

$$
\boxed{
O=
\frac{
\text{resident active subgroups}
}{
\text{hardware maximum resident subgroups}
}.
}
$$

A workgroup consumes finite per-compute-unit resources.

The number of simultaneously resident groups is constrained by the minimum of several limits:

$$
N_{\text{resident groups}}
\le
\min
\left(
N_{\text{thread limit}},
N_{\text{register limit}},
N_{\text{shared-memory limit}},
N_{\text{group limit}}
\right).
$$

Conceptually:

$$
N_{\text{reg}}
=
\left\lfloor
\frac{
R_{\text{CU}}
}{
R_{\text{thread}}
T_{\text{group}}
}
\right\rfloor.
$$

And:

$$
N_{\text{shared}}
=
\left\lfloor
\frac{
S_{\text{CU}}
}{
S_{\text{group}}
}
\right\rfloor.
$$

Whichever is smallest can become the occupancy bottleneck.

---

**Occupancy example**

Imagine a hypothetical compute unit with:

$$
65536
$$

register units available.

A kernel uses:

$$
R_t=64
$$

registers/thread.

Workgroup:

$$
T=256.
$$

Registers/group:

$$
64\times256=16384.
$$

Register limit allows:

$$
65536/16384=4
$$

resident groups.

If the compiler increases register usage to:

$$
128
$$

per thread:

$$
128\times256=32768,
$$

so only:

$$
2
$$

groups fit.

Without changing one line of dispatch code, occupancy has potentially halved.

---

**Occupancy is not synonymous with performance**

A kernel at:

$$
100\%
$$

occupancy is not automatically faster than one at:

$$
50\%.
$$

Suppose lowering register usage to increase occupancy forces values to spill into memory.

You gained resident warps but added expensive memory traffic.

Or a compute-heavy kernel may already have enough instruction-level parallelism to hide latency with fewer warps.

Thus:

$$
\boxed{
\text{occupancy is a constraint/diagnostic, not a score to maximize blindly}.
}
$$

M132 will return to this tradeoff in even more detail.

---

**Registers**

Every invocation has logical private variables:

```metal id="t9jsa8"
float sum;
float3 position;
float4 color;
```

The compiler tries to place these in registers.

Registers are extremely fast.

But the physical register file is finite.

That means a deceptively innocent shader variable has a system-level cost:

$$
\boxed{
\text{registers/thread}
\times
\text{resident threads}
\le
\text{register-file capacity}.
}
$$

---

**Register pressure**

Register pressure increases from long-lived values, large unrolled loops, large temporary expressions, complex material/path states and local arrays whose indexing cannot be optimized well.

Consider:

```cpp id="g2gruk"
float a0, a1, a2, a3;
float b0, b1, b2, b3;
float c0, c1, c2, c3;
```

If all remain live simultaneously, compiler must retain them.

If some can be recomputed or have shorter live ranges, registers can be released/reused.

Shader optimization is therefore partly **lifetime engineering**.

---

**Register spilling**

When register demand exceeds what the compiler/hardware can comfortably allocate, some values may spill into a thread-private address space that is physically backed by lower-level memory/cache.

Instead of:

```text id="i9imdl"
register → ALU
```

you can get:

```text id="x9gdnc"
load spill
ALU
store spill
```

This is enormously slower.

Hence a kernel can become slower when you “optimize” it by aggressively unrolling everything.

---

**Shared / threadgroup memory**

Threadgroup memory is fast on-chip storage shared by all invocations of one workgroup.

KairoGPU's tiled matrix multiplication uses:

```metal id="mmlugb"
threadgroup float lhs_tile[16][16];
threadgroup float rhs_tile[16][16];
```

That is the canonical shared-memory use case.

Why?

Without tiling, computing:

$$
C_{ij}
=
\sum_{k=0}^{K-1}
A_{ik}B_{kj}
$$

requires every output thread to repeatedly fetch rows and columns from device memory.

With a:

$$
16\times16
$$

tile, each workgroup cooperatively loads chunks of $A$ and $B$ once, then reuses those values many times.

---

**KairoGPU matrix multiplication**

Current kernel:

```metal id="tun1bn"
const uint row =
    group.y * 16 + local.y;

const uint column =
    group.x * 16 + local.x;

float sum = 0.0f;

for (uint base = 0;
     base < K;
     base += 16)
{
    lhs_tile[local.y][local.x] =
        valid
        ? lhs[row * K +
              base + local.x]
        : 0.0f;

    rhs_tile[local.y][local.x] =
        valid
        ? rhs[
            (base + local.y) *
            columns +
            column]
        : 0.0f;

    threadgroup_barrier(
        mem_flags::mem_threadgroup);

    for (uint offset = 0;
         offset < 16;
         ++offset)
    {
        sum +=
            lhs_tile[local.y][offset] *
            rhs_tile[offset][local.x];
    }

    threadgroup_barrier(
        mem_flags::mem_threadgroup);
}
```

Each loaded tile element is reused by many threads.

That increases arithmetic intensity dramatically.

---

**Why there are two barriers**

The first:

```metal id="lzwp5h"
threadgroup_barrier(...)
```

guarantees every thread has finished loading its tile entries before any thread reads the shared tile.

The second guarantees no thread begins overwriting the shared arrays for the next $K$-tile while another is still using the current one.

Without either barrier, you have a race.

---

**Shared-memory bank conflicts**

Shared/threadgroup memory is physically partitioned into banks so multiple lanes can access it simultaneously.

Conceptually:

$$
\text{bank}(address)
=
(address/\text{wordSize})\bmod B
$$

for some hardware-dependent bank count $B$.

If many lanes access different addresses mapping to the same bank, accesses can serialize.

That is a **bank conflict**.

If lanes access different banks, accesses can proceed in parallel.

---

**Why transpose-like accesses can create bank conflicts**

Imagine a 2D shared array:

```cpp id="32aupj"
float tile[32][32];
```

and each lane reads:

```cpp id="dov08i"
tile[lane][constantColumn]
```

depending on bank mapping, stride:

$$
32\times4\text{ bytes}
$$

may align repeatedly to the same bank.

A classic mitigation is padding:

```cpp id="ucsz3u"
float tile[32][33];
```

The extra column changes row stride and therefore bank mapping.

Do not blindly apply CPU cache-line intuition here; shared-memory banks are a distinct hardware organization.

---

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/07 - Typed Interfaces, Identity, and Data Structures|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/09 - GPU Memory, Divergence, Atomics, and Ordering|Next chapter]]
