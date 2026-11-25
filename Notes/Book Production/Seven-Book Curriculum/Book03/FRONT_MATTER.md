# Preface {#reader-preface}

The first question in this book is deceptively simple: where did that pixel come from? A picture in a game, a photograph of a house and a depth map all appear as arrays of numbers. What those numbers mean, and which parts of the world can be inferred from them, are different questions.

We will build a little house and follow it from an idea to points, triangles, a mesh, world coordinates, camera coordinates, and finally to the picture on a screen. Then we will turn the journey around. Can a camera tell us where a point was? Can two cameras recover a wall? Can a collection of measurements support a surface that we have never touched?

Our goal is to understand the complete chain, not to memorize a graphics API. Every new abstraction earns its place by solving a problem we can reproduce. The C++23 companion contains small, deliberately bounded reference implementations; the diagrams, numerical examples and controlled experiments explain what they establish and what they do not. The fact that a native test passes does not make an unimplemented extension a working feature. The book distinguishes derivation, implementation, and evidence.

## How to read the book

If the subject is entirely new to you, begin at [Chapter 01](#chapter-01) and follow the house in order. The mathematics of vectors, matrices, calculus, probability and numerical computation in *From Equations to Geometry* supports this journey; refer to that volume whenever a symbolic step feels mysterious. Do not rush past a worked example until you can predict its answer without looking.

If you already program, still read [Chapter 05](#chapter-05) before trying to optimize a geometric predicate, [Chapter 14](#chapter-14) before writing a camera projection, and [Chapter 32](#chapter-32) before claiming that an experiment demonstrates correctness. Every algorithm has a domain. The boundaries between representations are frequently where the difficult bugs live.

For each laboratory, first read the mathematical contract, then the source entry point and the tests. Run the exact fixture before modifying it. Keep one intentionally failing example beside the passing example. After changing the implementation, check whether the independent mathematical witness still agrees. A plot is a way to communicate a result, not a substitute for the result.

## Seven parts, one connected journey

| Part | Start | Question |
|---|---|---|
| I. Coordinates and shapes | [01](#chapter-01) | Where is a point, and which region owns it? |
| II. Shapes become 3D objects | [06](#chapter-06) | How do curves, surfaces and meshes represent a world? |
| III. Worlds become camera views | [11](#chapter-11) | How does a point acquire an image address and depth? |
| IV. The picture becomes an algorithm | [16](#chapter-16) | How do clipping, coverage, visibility and sampling cooperate? |
| V. Cameras become measuring instruments | [22](#chapter-22) | What do optics, sensors and color really measure? |
| VI. Images reveal geometry | [26](#chapter-26) | How can observations constrain unknown three-dimensional structure? |
| VII. Build and verify | [31](#chapter-31) | How do the pieces fit together, and what evidence earns our trust? |

The [companion laboratory guide](#laboratory-guide) gives a source-level route through all thirty-two workshops. The [notation reference](#notation-reference) collects the common frame and numerical contracts. These are aids to navigation, not replacements for the chapters.
