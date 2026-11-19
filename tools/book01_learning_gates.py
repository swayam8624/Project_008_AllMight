"""Chapter-specific reconstruction gates for the source-preserving reader.

Each gate asks the student to build an independent mental model, devise a
falsifying test, and transfer the principle. None is a claim that the tests
were run on an arbitrary reader platform.
"""
from __future__ import annotations

GATES = {
    1: ("Derive an eight-bit encoding from an observable state and show how the same bits admit different signed and unsigned interpretations.",
        "Write a bounded decoder for a tiny versioned record; test truncation, invalid versions and an exact maximum value.",
        "Explain which parts of an emulator cartridge header and a GPU asset record share the same encoding obligations."),
    2: ("Draw storage and object lifetimes separately; identify a valid address that cannot legally be used as the presumed object.",
        "Construct, move and destroy objects under sanitizers; predict which references and iterators become invalid.",
        "Explain why persistent object identity cannot be established by saving an in-process pointer."),
    3: ("Trace constructor selection, move eligibility and destruction for a resource-owning class.",
        "Write a failure-injection test that leaves the previous valid value intact when construction of a replacement fails.",
        "Transfer that exception-safety argument to publishing a freshly loaded texture or cartridge."),
    4: ("Specify the precondition, invariant, postcondition and failure condition of a bounded parsing function.",
        "Write positive, negative, boundary and mutation tests, and explain what each could falsify.",
        "Distinguish the evidence of a passing sanitizer run from a universal language guarantee."),
    5: ("Explain the difference between a syntactic concept constraint and the semantic promises of a generic interface.",
        "Implement a generic bounded transform; instantiate it with two types and test a rejected type or invalid input.",
        "Classify which C++ facilities in the example are language rules, library interfaces or implementation capabilities."),
    6: ("Derive the validation sequence of an index-plus-generation handle without confusing slot location with persistent identity.",
        "Create, remove and reuse slots while proving that a stale generation cannot access the new occupant within the chosen wrap policy.",
        "Apply the same registry contract to a scene entity and a GPU resource cache."),
    7: ("Trace one instruction through architectural state changes and distinguish ISA semantics from speculative microarchitecture.",
        "Compare a contiguous traversal with a seeded irregular traversal using an independent sum oracle and repeated measurements.",
        "Explain why compiler optimization and cache behavior affect measured time without changing the desired computation."),
    8: ("Draw a dependency DAG and identify which nodes can execute independently and which form the critical path.",
        "Compute a small exclusive scan by hand, then implement count-scan-scatter with bounds and output-uniqueness tests.",
        "Transfer the same ownership proof to a visibility list and a tile-entry generator."),
    9: ("Show how four conceptual lanes access contiguous versus strided storage and why their traffic may differ.",
        "Implement a predicated compact operation with an independent serial oracle; vary workgroup size without assuming one value is always best.",
        "Explain when atomics prove uniqueness and when scans eliminate contention by assigning disjoint output offsets."),
    10: ("Sketch the build graph from a changed source file through compiled objects to a reproducible executable.",
         "Pin a toolchain and construct a clean rebuild; deliberately vary one dependency and verify that the correct target changes.",
         "Separate correctness tests, build reproducibility and performance evidence into independent reports."),
    11: ("Trace a non-owning span, its owning allocation and a delayed device consumer on a common lifetime axis.",
         "Test container relocation and rejection of a stale borrowed view without first dereferencing invalid storage.",
         "Explain why const-qualified access does not establish object ownership or an inter-thread memory-ordering guarantee."),
    12: ("Name encoding, submission, device execution, visibility and retirement as distinct events.",
         "Build a deterministic pending-consumer state machine and reject resource reuse before completion.",
         "Translate the abstract lifetime proof into one API-specific device experiment with explicit capability checks."),
    13: ("Reconstruct the Vulkan object and allocation graph; distinguish buffer handles, bound memory, descriptors and command buffers.",
         "Specify each stage of a staging upload and verify readback bytes against a host oracle.",
         "Explain how an image transition and descriptor lifetime solve different obligations."),
    14: ("Name the writer, reader, resource range, stage scopes, access types and completion witness of a cross-pass dependency.",
         "Compare a correct narrowly scoped dependency with a deliberately broader legal dependency while retaining a correctness oracle.",
         "Explain why a barrier cannot repair an out-of-range scatter or a prematurely destroyed allocation."),
    15: ("Draw a Metal command-buffer lifecycle from host encoding through commit, execution and completion.",
         "Build a versioned frame-slot model and ensure a slot cannot be reused while a submitted GPU read is pending.",
         "Distinguish shared physical memory from coherent access and valid temporal ordering."),
    16: ("Construct a roofline-style bound from measured bytes, operations, time and the actual device capabilities.",
         "Use repeated experiments with a correctness oracle, controls and uncertainty estimates before claiming a speedup.",
         "Explain one case in which optimizing a kernel makes total frame time worse because another stage becomes dominant."),
}


def gate_latex(chapter: int) -> str:
    if chapter not in GATES:
        raise ValueError("Missing chapter-specific learning gate")
    def escape(s: str) -> str:
        mapping = {"\\": r"\textbackslash{}", "&": r"\&", "%": r"\%",
                   "$": r"\$", "#": r"\#", "_": r"\_", "{": r"\{", "}": r"\}"}
        return "".join(mapping.get(c, c) for c in s)
    names = ("Reconstruct", "Falsify", "Transfer")
    parts = [
        r"\subsection*{Reconstruct, Test, and Transfer}",
        r"\begin{enumerate}",
    ]
    for title, sentence in zip(names, GATES[chapter]):
        parts.append(r"\item \textbf{" + title + ".} " + escape(sentence))
    parts.append(r"\end{enumerate}")
    return "\n".join(parts) + "\n"
