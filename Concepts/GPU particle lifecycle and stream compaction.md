# GPU particle lifecycle and stream compaction

A particle system is not only an integration loop. It owns a changing population: particles spawn, advance, expire, may collide or sort, and their storage must remain dense enough for later GPU work.

## State and update

A minimal particle stores position, velocity, age and lifetime. With constant velocity,

$$
x_{t+Delta t}=x_t+vDelta t,
qquad
a_{t+Delta t}=a_t+Delta t.
$$

The particle remains alive while its age is below its lifetime under the chosen endpoint convention. More complex systems add acceleration, drag, curves, random streams and collision state, but those do not remove the population-management problem.

## Predicate → scan → scatter

On a GPU, deleting expired particles one by one is awkward. A common parallel pattern is:

1. evaluate an alive predicate into 0/1 flags;
2. exclusive-scan the flags;
3. scatter alive records to the scanned offsets;
4. derive the new count from the final offset and flag.

This preserves stable order if scatter follows source indices. An atomic append can be simpler but creates contention and usually does not preserve deterministic order.

Free-list spawning is the inverse population problem. A full system needs capacity bounds, overflow policy, deterministic/random seed ownership, and indirect draw/dispatch counts that become visible to later commands with correct synchronization.

## Executable references

`tools/labs/realtime_effects_reference_lab.cpp` proves deterministic particle integration, lifetime expiry and stable CPU compaction. `tools/labs/gpu_parallel_primitives_lab.cpp` independently proves predicate-scan-scatter compaction against a serial oracle. Together they connect particle semantics to the parallel primitive used to maintain dense storage.

They do not yet prove a native GPU free list, indirect command generation, depth sorting, collision, or backend synchronization.

Related: [[Prefix scan and stream compaction]], [[Stateful pseudorandom generators and stream state]], [[Deterministic reduction tree]].
