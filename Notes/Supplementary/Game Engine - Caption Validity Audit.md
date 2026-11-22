# Caption and ASR validity audit — November 10, 2026

**Result:** All 457 saved video sources passed the structural checks below. Spoken accuracy is not certified. One near-duplicate pair and several coverage/terminology warnings require source review before corresponding teaching is accepted.

| Check | Result |
|---|---:|
| Downloaded caption tracks checked | 449 |
| Locally generated ASR transcripts checked | 8 |
| Raw file and stored unit hashes matched | 457 / 457 |
| Reparsed caption text/timestamps match stored cue units | 457 / 457 |
| Every cue retained in model reading spans, in order | 457 / 457 |
| Invalid/nonpositive cue intervals or backwards starts found | 0 |
| Exact normalized transcript duplicates across videos | 0 |
| Near-duplicate pairs requiring original-source confirmation | 1 |
| Original audio manually compared to transcript in this audit | 0 |

A hash identifies saved bytes and detects later changes. A matching hash does not prove that every spoken word was recognized or that a track belongs to the intended recording. The audit checked every saved source, including 414 videos already marked extracted in its input snapshot; it did not validate the generated teaching prose.

## Source-association warning: Kohi 101 and 102

The saved tracks for [Episode 101 — Starting Simple Scene File Parsing](https://www.youtube.com/watch?v=q8AfndGO-Ms) and [Episode 102 — Finalizing Simple Scene Load from File](https://www.youtube.com/watch?v=tjQWdtn9FgY) are unusually similar:

- Both cached video durations are 12,137 seconds.
- Both first usable cues start at 446.749 seconds.
- Their normalized five-word phrase sets have an exact Jaccard similarity of approximately 81%. Their raw hashes differ.
- This could reflect duplicated source uploads, variants of the same recording, or an incorrect caption-to-video association. The local evidence does not distinguish those explanations.
- A live recheck of both YouTube pages was throttled. Their original audio/video has not been confirmed in this audit.

**Review requirement:** Preserve both tracks, but resolve their association before treating them as separate lessons. The extraction coordinator does not automatically merge either source.

## Coverage warnings

All nonempty timestamped cues were accounted for. The raw downloaded tracks also contain 108,048 empty/nontext timing cues, which the importer deliberately omits from text units. These are not missing spoken words.

- 28 tracks start their first usable caption more than two minutes into the recording.
- Five tracks contain internal caption gaps of at least two minutes.
- Cached full-video durations were available for 163 sources; none triggered the audit’s material early-ending or overrun threshold. Those thresholds allow small tails and do not prove complete spoken coverage.
- The remaining 294 sources lack duration metadata in the inspected inventory files, so full-video endpoint coverage remains unverified.

Long starts or gaps can reflect an introduction screen, music, silent coding or missing recognition. They require audio comparison rather than automatic acceptance or rejection.

| Video with an internal gap | Gap location in seconds |
|---|---|
| Finalizing Hot-Reloading (Kohi Ep. 91, Live Stream 2023.02.09) | 3710.99–3831.15 |
| Adding Better Vulkan Debugging (Kohi Ep. 103, Live Stream 2023.04.05) | 82.19–238.61 |
| Material System Redux, Part 3 (Kohi Ep. 114, Live Stream 2023.06.07) | 90.80–215.69; 220.51–595.07 |
| Standard UI Nine-Slice (Kohi Ep. 140, Live Stream 2023.10.11) | 9135.27–9264.51 |
| Standard UI Button, Rotation and Mouse Input/Clicking (Kohi Ep. 141, Live Stream 2023.10.12) | 1905.79–2031.99 |

## Generated ASR: integrity passed, wording requires caution

All eight ASR sources retain verifiable generation receipts: the receipt, saved audio, model and transcript hashes match their recorded values. They were generated with Whisper base.en and remain separate from downloaded YouTube caption tracks. No audio was manually compared during this audit.

Known warnings include “COVID” near 00:01:02 in the Kohi introduction, Vulkan/API spelling errors, and repeated or malformed passages in the ImGui and Sparky renderer transcripts. The existing toolchain teaching addition already treats technical tokens separately from the raw ASR spelling.

Five imported source metadata files do not embed the full ASR generation details; their verified receipts remain available in the separate ASR provenance index. This is a provenance-linking limitation, not a detected audio/transcript hash mismatch.

**Teaching rule:** Use ASR as qualified source evidence. Confirm exact identifiers, quotations and code behavior against original audio/video or independent primary documentation and versioned source code. A generated transcript is not a verified verbatim transcript.

## Reproducible evidence and boundaries

- [Full per-source structural report](/Users/swayamsingal/.codex/source-cache/engine-corpus-2026-11-07/quality/caption-validity-2026-11-10/report.json)
- [Near-duplicate screening report](/Users/swayamsingal/.codex/source-cache/engine-corpus-2026-11-07/quality/caption-validity-2026-11-10/near-duplicates.json)
- [ASR provenance and wording review](/Users/swayamsingal/.codex/source-cache/engine-corpus-2026-11-07/quality/asr-validity-2026-11-10.json)
- [Per-source supervised-review gates](/Users/swayamsingal/.codex/source-cache/engine-corpus-2026-11-07/quality/caption-validity-2026-11-10/review-gates.json)
- [Audit script](/Users/swayamsingal/.codex/source-cache/engine-corpus-2026-11-07/quality/caption-validity-2026-11-10/audit.py)

The audit read the saved sources and wrote these reports. It did not alter caption/audio bytes, replace existing notes, approve generated prose, or stop the running extraction jobs. All cue-level comparisons and exact duplicate checks cover the complete saved corpus. Near-duplicate screening is a heuristic; the flagged pair was additionally compared using complete normalized five-word phrase sets.
