#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
NOTE = ROOT / "Continuous Notes/Game Engine - From First Frame to Living Worlds.md"

def code_block(path, language):
    return "\n\n~~~" + language + "\n" + path.read_text().rstrip() + "\n~~~\n"

def insert_or_refresh_before(text, anchor, marker, body):
    if anchor not in text:
        raise SystemExit("missing anchor: " + anchor)
    normalized = body.strip("\n")
    if marker in text:
        start = text.index(marker)
        while start > 0 and text[start - 1] == "\n":
            start -= 1
        end = text.index(anchor, start)
        return text[:start] + "\n\n" + normalized + "\n\n" + text[end:]
    idx = text.index(anchor)
    start = idx
    while start > 0 and text[start - 1] == "\n":
        start -= 1
    return text[:start] + "\n\n" + normalized + "\n\n" + text[idx:]

text = NOTE.read_text()

rollback_code = code_block(ROOT / "tools/labs/rollback_netcode_lab.cpp", "cpp")
audio_code = code_block(ROOT / "tools/labs/audio_voice_mixer_lab.cpp", "cpp")
ai_code = code_block(ROOT / "tools/labs/ai_decision_models_lab.cpp", "cpp")

rollback = """
<!-- Bible-final-pass: rollback netcode -->
### Prediction hides latency; rollback repairs a wrong prediction

The earlier networking sections correctly start from authority, identity, packet validation, and bounded work. A fast action game still has one practical question: what should the local machine do before a remote input or server correction arrives?

If the client waits for every remote input before simulating, network delay becomes control delay. Prediction avoids that wait by simulating with the best information currently available. Prediction creates a second problem: it can be wrong. Rollback is the repair mechanism.

Keep the simulation on integer ticks. Write one tick as:

~~~text
S_(k+1) = F(S_k, I_k)
~~~

S_k is the complete gameplay state needed to continue the simulation. I_k is the complete input set for that tick.

For a two-player rollback peer:

~~~text
known local input    -> use it
known remote input   -> use it
missing remote input -> predict it
simulate tick
store resulting state
~~~

A common prediction rule is to repeat the last known remote input. It is simple and often useful for held movement. It is not a universal rule. A fighting game might repeat a direction while predicting one-shot attack buttons as unpressed. Prediction policy affects game feel.

Suppose the peer has simulated through tick 105 while the remote input for tick 101 was predicted. The real tick-101 input later arrives.

If it matches the prediction, no repair is needed.

If it differs:

~~~text
restore S_100
replace predicted I_101 with real I_101
resimulate 101, 102, 103, 104, 105
continue from corrected S_105
~~~

The player normally sees the newest corrected state, not the temporary historical replay.

#### Rollback requires reproducible simulation

Rollback assumes that the same starting state plus the same input sequence produces the same result. That requirement reaches beyond networking. It affects update order, random-number generation, physics ordering, unordered-container iteration, script side effects, spawning and stable IDs, event delivery, and any system that reads wall-clock time or other mutable external state.

The witness uses integer gameplay state so this requirement is visible. A production engine can use floating point, but supported platforms still need a tested determinism policy instead of an assumption.

A useful diagnostic is a state hash:

~~~text
hash(tick, gameplay state that should agree)
~~~

Comparing hashes can reveal the first divergent tick. A hash says that state differs; it does not say why. Serious replay tooling should also expose subsystem or component traces so the first differing value can be found.

#### Rollback history is finite

To repair an input from N ticks ago, the engine needs a state from before that input and enough input history to replay forward.

A rollback buffer therefore has a finite window:

~~~text
oldest retained state ... current speculative state
~~~

The window costs memory and resimulation CPU. If a correction arrives older than retained history, the engine needs a declared recovery policy: request an authoritative snapshot, hard-correct, pause/disconnect, or another explicit rule. Missing history must not become silent state corruption.

#### Prediction, rollback, interpolation, and reconciliation are different tools

Local prediction makes the local player's own controls feel immediate.

Rollback repairs speculative simulation when late input disagrees.

Snapshot interpolation intentionally renders remote objects a little in the past so two received snapshots are usually available for smooth interpolation.

A server-authoritative client can use server reconciliation. The client predicts numbered local inputs. The server later sends an authoritative state and the last processed input number. The client installs the server state, discards acknowledged inputs, and reapplies newer inputs:

~~~text
client predicts input 80
client predicts input 81

server replies with state after input 80 and ack=80

client:
  install server state
  discard history through 80
  replay input 81 and later
~~~

Lag compensation is different again. A server may evaluate a shot against a reconstructed historical world near the shooter's command time. That requires policy: maximum rewind, which objects rewind, interpolation rules, and what happens when the shooter's past view conflicts with another player's present view.

#### Presentation does not have to expose every correction

A corrected gameplay state may jump. Rendering that correction directly can look harsh. Keep simulation truth separate from visual smoothing. The gameplay state can become correct immediately while a presentation transform eases toward it, provided the smoothed value never becomes authoritative collision or gameplay state.

This witness does not implement transport, congestion control, authentication, packet redundancy, anti-cheat, floating-point cross-platform determinism, snapshot compression, or lag-compensated hit validation. It makes the simulation/history contract reconstructable before those layers are added.

The executable witness below predicts missing remote input, stores state history, detects a late mismatch, restores the previous state, resimulates to the present, and verifies that the final state equals a direct authoritative simulation. It also checks duplicate packets and a correction older than retained history.
""" + rollback_code + """

**Reconstruct without reading:** if a wrong tick-101 prediction is discovered after simulating tick 105, name the snapshot restored and every tick replayed. Then distinguish local prediction, rollback, server reconciliation, snapshot interpolation, and lag compensation.
"""

audio = """
<!-- Bible-final-pass: real-time audio runtime -->
#### Turn the audio front end into a real-time voice and mixing system

The front-end/backend split above tells us who owns the public API. We still need the runtime path that turns many play requests into the sample block consumed by the device.

Separate four meanings:

~~~text
audio asset
  decoded or compressed sound data

voice
  one active playback instance of an asset

mixer
  combines active voices into output buses

device backend
  delivers final sample blocks to the platform audio API
~~~

One explosion asset can have twenty simultaneous voices. Asset identity and playback identity are different.

#### The audio callback is a deadline boundary

Many audio APIs periodically request the next block of samples. Missing the deadline creates an audible underrun, click, or gap.

Treat the callback more strictly than a normal update. Avoid filesystem/network I/O, unpredictable decoding, waiting on game-thread mutexes, unbounded heap allocation, blocking logging, or complex destruction in the callback path.

A common structure is:

~~~text
game thread:
  enqueue Play(sound, gain, position, priority)

audio thread:
  consume bounded commands
  update preallocated voice slots
  mix next block
  submit samples
~~~

The exact queue can vary. The important contract is that audio completion does not depend on a slow editor or gameplay operation.

#### A voice owns playback state, not the asset

A voice may contain an asset reference, sample cursor, gain, playback rate, loop state, priority, spatial parameters, bus routing, fade/envelope state, and start sequence.

The cursor is per voice. Two enemies playing the same footstep asset at different times must not share one cursor.

When a non-looping cursor reaches the end, the slot becomes reusable. A loop wraps according to its loop range. A streamed voice may point at decoded chunks rather than one fully resident PCM buffer.

#### Mixing is addition plus a level policy

For mono voices x_j[n] and gains g_j:

~~~text
y[n] = sum_j g_j x_j[n]
~~~

Several individually legal samples can sum outside the final range. Hard clipping is easy but distorts. A production engine may keep floating-point headroom, use bus gains, compression/limiting, or another mastering policy.

The witness hard-clips only to keep its small test output bounded. That is a test policy, not a recommendation for final mastering.

#### Voice capacity needs an overload rule

CPU and hardware budgets limit active voices. When all slots are occupied, an engine can reject the new sound, steal the oldest, steal the quietest, steal the lowest priority, or virtualize inaudible voices.

The policy is player-visible. Critical dialogue should not disappear because many low-priority debris sounds started first.

The witness uses a deterministic rule: steal the lowest-priority voice; for equal priority, steal the oldest. If the new request is lower priority than every active voice, reject it.

#### Streaming separates decoding from playback

A long track need not be fully decoded into memory. Keep a small queue of PCM blocks:

~~~text
compressed asset
  -> worker fills block A
  -> worker fills block B

audio consumes A
worker refills retired A
audio consumes B
...
~~~

The decoder must stay ahead of playback. If it falls behind, report an underrun. Seek, pause, loop, and device changes all belong to this state machine.

#### Resampling is part of correct playback

A 44.1 kHz source mixed into a 48 kHz device cannot advance one source sample per output sample without changing speed.

The source-position increment is approximately:

~~~text
source_rate / output_rate
~~~

That gives a fractional cursor. A reconstruction filter estimates values between stored samples. Linear interpolation is simple; higher-quality resamplers spend more work for better frequency behavior.

Changing playback rate also changes pitch and duration. Time stretching without changing pitch is a different algorithm.

#### Spatial audio consumes published world meaning

The game owns emitter and listener transforms. Publish a compact timestamped snapshot for audio rather than letting the callback query arbitrary ECS state. Occlusion queries can run at a safe cadence elsewhere and publish their results to the mixer.

It does not implement HRTF processing, resampling, streaming decode, effects buses, device hot-swap, or thread-safe command publication. Those now have explicit architectural homes instead of being hidden behind a single play-sound call.

The executable witness below owns a fixed voice pool and performs no allocation inside its mixing function. It verifies summation, natural voice retirement, looping, deterministic voice stealing, rejection of an unimportant sound when all slots are more important, and bounded output.
""" + audio_code + """

**Reconstruct without reading:** distinguish an asset from a voice, explain why the callback should not wait on the game thread, derive the basic mix equation, state a voice-stealing rule, and explain how streaming buffers stay ahead of playback.
"""

ai = """
<!-- Bible-final-pass: AI decision layer -->
## Navigation answers how to move; the decision layer answers what to do

The navigation chapter already teaches graphs, A*, navmeshes, funnels, dynamic obstacles, and path queries. Those systems answer a geometric question:

> Given a destination, how can this agent reach it?

They do not decide whether the agent should attack, heal, investigate, retreat, reload, guard, or choose a different goal. That belongs to a decision layer.

Behavior trees, utility systems, and goal-oriented action planning solve related decision problems with different tradeoffs.

### Behavior trees expose authored control structure

A sequence usually means:

~~~text
try child 1
if it succeeds, try child 2
fail on the first failure
succeed if all succeed
~~~

A selector usually means:

~~~text
try child 1
if it fails, try child 2
succeed on the first success
fail if all fail
~~~

Leaves commonly return Success, Failure, or Running. Running matters because movement or animation can take many ticks.

A small combat tree might be:

~~~text
Selector
  Sequence
    IsHealthLow
    MoveToCover
  Sequence
    CanAttack
    Attack
  SearchForEnemy
~~~

Priority is visible in the structure. The tradeoff is that large trees can accumulate repeated conditions and hidden priority interactions.

A running node also needs interruption policy. Does the tree reevaluate higher-priority branches each tick, or stay committed until the current task finishes? Both are valid and produce different behavior.

### Utility AI scores alternatives

Utility AI computes a score for each candidate action:

~~~text
score(action, state) -> real number
choose the highest acceptable score
~~~

For example, cover score can rise as health falls, attack score can be high only when a visible target can be attacked, and search score can rise when no target is visible.

This makes continuous tradeoffs easy to tune, but it adds contracts: normalize inputs, define response curves, prevent tiny score changes from causing rapid action switching, expose scores in debug tools, and use deterministic tie-breaking when replay matters.

Numbers do not automatically make an AI smarter. Bad utility curves can be less readable than a small tree.

### GOAP searches over preconditions and effects

Goal-Oriented Action Planning represents actions with symbolic preconditions, effects, and costs.

Suppose the goal is enemy_defeated=true. Actions might be:

~~~text
find_weapon:
  pre: no weapon
  effect: has weapon
  cost: 2

find_ammo:
  pre: has weapon and no ammo
  effect: has ammo
  cost: 2

find_enemy:
  pre: enemy not visible
  effect: enemy visible
  cost: 1

attack:
  pre: weapon + ammo + visible enemy
  effect: enemy defeated
  cost: 1
~~~

The planner searches the symbolic state graph for a low-cost sequence that reaches the goal.

The symbolic action find_enemy does not specify pathfinding, animation, or perception implementation. It says what subgoal should occur. Lower systems execute it.

### A plan must survive a changing world

Another agent may take the ammo before this agent arrives. Therefore each action must verify its preconditions when it begins and often while it runs. If they fail, repair or replan.

Replanning every frame can waste CPU and make behavior unstable. Useful triggers include action failure, important fact changes, goal changes, invalidated plans, or periodic low-frequency reconsideration.

Planning work must also be bounded. A large symbolic state space can explode just like an unbounded path search.

### Keep perception, memory, decision, navigation, and action separate

A useful pipeline is:

~~~text
sensors / perception
  -> interpreted facts
  -> memory with age/confidence
  -> decision system
  -> chosen goal/action
  -> navigation or task executor
  -> animation/physics/gameplay command
  -> changed world
  -> new perception
~~~

This prevents one opaque AI leaf from raycasting, pathfinding, changing animation, and directly editing physics velocity without clear ownership.

### AI needs observability

For one selected agent, a debug view should show perceived facts, remembered targets and their age, active goal, active behavior-tree branch or utility scores, current plan, path result, action status, and reason for failure/replan.

"Enemy acted stupid" becomes useful only after the engine can show which fact or decision produced the action.

A real game can combine them: utility chooses fight versus flee, a behavior tree executes the mode, GOAP handles a special multistep task, and navigation moves toward requested destinations.

The executable witness below sends small world states through all three models. The behavior tree exposes authored priority, utility AI scores alternatives, and GOAP searches action preconditions/effects. It is a comparison of semantics, not a declaration that one architecture wins.
""" + ai_code + """

**Reconstruct without reading:** explain what A* answers that a behavior tree does not. Write one selector, one utility score, and one GOAP action. Then state when replanning should occur and which AI state the editor must expose.
"""

resource = """
<!-- Engine-resource-state-plan -->
### Plan subresource state before translating it to an API

A logical render graph describes which pass uses a resource. A backend planner has a second job: track each image mip and layer independently, including whether its contents exist, its current layout, its queue owner, and the stages that read or wrote it. A whole-image flag loses correctness when one mip is uploaded while another is sampled.

Its tests establish only the planner's CPU state contract. Native barrier translation still needs API validation and device execution.

The witness below turns those declarations into symbolic same-queue transitions and release/acquire obligations. The records explain the reason for each transition; they are not Vulkan barriers, Metal fences, or evidence of driver synchronization. The example rejects unknown subresources, duplicate image declarations, undefined reads, unsupported feedback, and illegal stage/layout combinations before returning a plan.

```cpp
""" + (ROOT / "tools/labs/resource_state_plan_lab.cpp").read_text().rstrip() + """
```

"""

scene_persistence = """
<!-- Engine-scene-persistence -->
#### Serialize scene meaning through stable identities

Saving a scene must preserve authored meaning across process lifetimes. Runtime pointers and dense ECS slots are temporary, so each serialized entity and asset needs a stable ID. Each field must be written deliberately; dumping a C++ object layout would also persist padding, compiler ABI, and endianness accidents.

A safe loader first checks byte and record bounds and parses into a candidate scene. It validates unique entity IDs and asset IDs, builds the complete ID table, then resolves parent references in a second pass so forward references work. Only a fully validated candidate can replace the live scene. Version migration supplies defined defaults for fields added later. The small text fixture below demonstrates round-trip, a version-1 migration, and rejection of unsupported versions, oversized counts, malformed coordinates, duplicate IDs, missing assets, and dangling parents. Its codec is intentionally portable and dependency-free; it does not promise filesystem atomicity, crash durability, checksums, escaping for arbitrary strings, or a production binary schema.

```mermaid
flowchart LR
  A[Versioned scene bytes] --> B[Bound tokens and migrate fields]
  B --> C[Build candidate IDs and resolve references]
  C --> D[Reject missing assets and hierarchy cycles]
  D --> E[Swap fully valid candidate into live scene]
  B -->|invalid| F[Keep previous live scene]
  C -->|invalid| F
  D -->|invalid| F
  classDef valid fill:#dcfce7,stroke:#15803d,color:#14532d
  classDef fail fill:#ffedd5,stroke:#c2410c,color:#7c2d12
  class E valid
  class F fail
```

The candidate is private until every gate succeeds. A failed parse, unresolved reference or cycle leaves the old live scene available; the comments in the program explain how the checks and final swap enforce that boundary.

```cpp
""" + (ROOT / "tools/labs/scene_persistence_lab.cpp").read_text().rstrip() + """
```

"""

text = insert_or_refresh_before(
    text,
    "### 6. Dependencies, composition, and the build graph",
    "<!-- Engine-scene-persistence -->",
    scene_persistence,
)
text = insert_or_refresh_before(
    text,
    "### Complete C++20 graph compiler, executable trace and negative cases",
    "<!-- Engine-resource-state-plan -->",
    resource,
)

text = insert_or_refresh_before(
    text,
    "#### Shared systems should pay for themselves",
    "<!-- Bible-final-pass: real-time audio runtime -->",
    audio,
)
text = insert_or_refresh_before(
    text,
    "## Let companion AI and level geometry share one contract",
    "<!-- Bible-final-pass: AI decision layer -->",
    ai,
)
text = insert_or_refresh_before(
    text,
    "### A reconnect window needs an ownership policy",
    "<!-- Bible-final-pass: rollback netcode -->",
    rollback,
)

NOTE.write_text(text)
print("engine final-pass patch applied")
