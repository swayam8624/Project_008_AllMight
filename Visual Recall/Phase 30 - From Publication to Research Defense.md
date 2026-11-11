---
tags: [visual-recall, production, dcc, benchmarking, research-method, capstone]
coverage: "Parts 2901–3000, M175–M180"
companion: "Production — From Source Files to Evidence and Defensible Systems"
---

# Phase 30 — Reconstruct an evidence-bearing graphics-engine pipeline

This is an active reconstruction companion. Read [[Continuous Notes/Production - From Source Files to Evidence and Defensible Systems|the complete Production volume]] for explanations and equations. Then attempt the cases below **without** using its definitions as prompts. The goal is to recreate data contracts, algorithm derivations, ownership, failure handling, measurements and actual source-to-output traces, rather than recite titles.

## A missing frame is concealed by a duplicate

An artist supplies panel.0001.png and panel.0003.png. Another tool supplies panel-0001.png. A third file uses panel.03.png. The requested sequence is frames 1–3. Develop a parser that maps each filename to a stem, separator, integer frame index, padding width and extension. Prove that simply counting files cannot establish a complete sequence: a duplicate integer can mask a gap. Prove the stronger condition that the mapping is injective and covers every integer in the declared inclusive interval under the chosen stem/padding/format policy. Give stable diagnostic codes, severity and source context for every failure, and explain how to sort them independently of filesystem enumeration order.

**Independent test:** implement the parser and construct valid, malformed, duplicated, mixed-padding and missing-frame cases. Compare to [[tools/tests/test_production_evidence.py|the reference tests]] only after completing your own solution. Then explain why successful filename validation still requires hashing the contents.

## A production pipeline escapes the workspace despite a prefix check

A script decides that /studio/cache-old is inside /studio/cache because a string prefix matches. Explain why this is wrong using path *components* rather than textual prefixes, then trace a traversal through parent-directory elements and a symlink pointing outside the project. Distinguish lexical normalization from canonical resolution. Explain why resolve and containment are adequate for a cooperative single-writer teaching test but do not prevent malicious time-of-check/time-of-use symlink races.

Construct the full publication state machine: read and fingerprint inputs; produce a mutation-free plan; reject failed diagnostics; prepare a staging directory under the same filesystem; copy and fsync files; hash staged output; write versioned manifest; rename into a previously unused immutable published version; clean up on failure. Differentiate atomic *visibility* from crash durability, distributed transactions and concurrent publisher arbitration. State what happens if a source changes between planning and copying and what happens when the target already exists. Sketch a correct rollback path that does not delete the previously published version.

**Independent test:** inject a changed source hash and show that no new output version survives. Confirm that constructing a dry-run plan never writes to the workspace.

## Four DCCs supply seemingly identical geometry

An object in Blender contains an unapplied modifier; a Maya mesh is instanced through the DAG; a Houdini SOP node changes result with frame; a Nuke Write node uses a different frame range and color space than its Read node. Identify the host-specific evidence each adapter must extract before a single host-neutral validator is entitled to accept the output. Distinguish a source object from its evaluated geometry, transform nodes from shape nodes, geometry attributes from shader/material declarations, authoring nodes from output/cache nodes, and file existence from evaluated semantic correctness.

Draw the data flow from source DCC coordinates and units to exported bytes, engine mesh import, memory allocation, object/world/view/clip transforms, GPU upload and finally one pixel sample. Reconstruct the source coordinate convention and normal transform equation, including why a point and direction cannot share translation rules. Then distinguish what the provided Python file-pipeline test actually verifies from what would require Blender, Maya, Houdini, Nuke, Vulkan, Metal or the current engine repositories to run.

## One fast profiler number proves less than its author claims

A renderer takes 0.2 ms to submit a Vulkan command buffer according to a CPU stopwatch. Explain why that is not necessarily the GPU's execution duration. Draw a timeline showing command recording, queue submission, GPU wait, execution, timestamp query, completion signal and optional readback. Define CPU wall-clock, monotonic elapsed duration and calibrated device timestamps without conflating their clock domains.

Given the five measured durations [10,20,30,40,50], derive arithmetic mean, median and nearest-rank p90/p99. Explain why p99 is the observed maximum in this tiny sample but this does not provide a reliable estimate of a true 99th percentile latency. Design a comparative benchmark with predeclared warm-up and cold-start policies, random or interleaved treatment order, scene/seed distribution, equal workload and uncertainty reporting. Explain why excluding the slowest cases after results arrive is cherry-picking.

## Two images yield the same PSNR and different practical failures

Derive MSE from corresponding channels of two image arrays. Derive PSNR from MSE and a specified peak channel value. Evaluate a reference [0,1], test [0,0] and peak 1: MSE=0.5 and PSNR=10 log10 2, approximately 3.0103 dB. State the identical-image limit. Explain why comparing different crops, color encodings, exposures or alpha conventions may make the result meaningless.

Contrast exact pixel differences, MSE/PSNR and SSIM: which signal property is measured, what assumptions are implicit, and why no one scalar captures silhouette topology, editable geometry, flicker, contact stability or human perceptual preference. Propose an appropriate metric set for one actual engine workload and a synthetic numerical witness that checks the implementation without establishing generalization.

## A reviewer removes the repository and asks for first-principles reconstruction

Convert “uncertainty-aware reconstruction improves geometry editability” into a falsifiable research question with explicit treatment, baseline, outputs, controls, scene split and a predetermined decision rule. Identify synthetic fixtures versus measured data and explain how correlated frames must not be miscounted as independent scene-level samples. Specify a one-component ablation and an interaction experiment that tests coupled components.

Reconstruct a complete system on a whiteboard, starting from one input datum. Give its byte representation, owner, coordinate units, in-memory layout, algorithmic transformation, synchronization boundary and output evidence. Derive a nontrivial equation, show a numeric limiting case, and explain how it would be validated by independent code. Compare three designs with costs in operations, bytes moved, allocations, synchronization and failure modes. Predict the outcome when the reviewer increases scene size, changes numerical precision, forces device loss, introduces a stale resource handle or invalidates temporal history.

The final question is repeatedly “Why?” until an architectural claim reduces to a mathematical, machine or observed contract. No amount of answered terminology alone certifies mastery; an independent implementation, evidence package and oral defense are still required.
