# Vault repair and source retention — 14 November 2026

This is the supervised local repair of the active iCloud/Obsidian Master vault, beginning at commit `c142869a391c32d55068ddfbf4d4ac11ce52762c`. The vault's existing hidden `.obsidian` configuration and unrelated Finder/workspace edits are left in place. The three continuous books remain the teaching surface; this record tracks changes and evidence, not a fourth lesson book.

## Integrated teaching and code comments

Computing Foundations now embeds its temporary-lifetime, SE(3) exp/log and paired-inference witnesses at their teaching seams. The misplaced closing fence after the reverse-mode Tape includes is removed, keeping the class inside its C++ block. The statistical witness validates finite inputs, normalizes its sign-flip comparison and checks measurement-unit rescaling; the pose witness exercises its documented near-pi rejection.

Graphics now embeds the planar eight-state EKF, 3-D zero-order-hold IMU preintegration and discrete NeRF volume-rendering witnesses. The planar camera is explicitly co-located and aligned with the body. The original 1,842-line perceptual/XR packet is relocated intact after rendering so the mathematical prerequisites precede it. The relocation itself preserves 240,779 characters exactly; earlier wiki-route corrections are recorded separately.

Game Engine now embeds the block contact, symbolic subresource-state, rollback, audio and AI witnesses. The new bounded scene-persistence example teaches explicit versioned fields, strict decimal input, stable IDs, forward references, iterative cycle rejection, migration and candidate-then-swap publication. Negative fixtures prove failed loads keep the live scene unchanged. This is an in-memory schema/transaction witness, not filesystem durability or a production scene format.

For these added and revised programs, implementation reasoning, state meanings, units, branch choices and limits are explained in comments beside the relevant operations. Conceptual prerequisites remain before code. Existing teaching elsewhere was preserved; this pass does not claim to have re-commented every historical snippet.

## Navigation, formatting and supervision

Retired Mathematics, XR and Systems wiki targets now lead to the actual canonical books, retaining labels and anchors. Four dangling concept links point to existing developed explanations rather than new glossary stubs. The retired rigid-body route resolves to the preserved Foundations section.

Independent small-agent reviews exposed the missing camera/body extrinsic assumption, scale-dependent statistical tie tolerance, nonfinite-data handling and an untested near-pi branch; those findings were repaired before final verification. Five existing concept cards also had control-byte/missing-command corruption in TeX. Their GGX/Fresnel, homology, spherical harmonics, impulse/friction and Beer–Lambert formulas were repaired with units and domain assumptions.

The source guard compares complete parsed code blocks, including comments, instead of relying on a common first include line or counting only one fence style. The preservation guard pins the actual pre-repair books and permits only the documented route, Tape-fence, closure-statement and intact-XR-relocation changes. It still rejects removal or reordering of other prior teaching.

## Verification and its limits

- `python3 tools/check_continuous_code_sync.py` checks all 26 declared complete C++/Python programs.
- `python3 tools/check_three_volume_consolidation.py` checks the three-book inventory, preserved baseline teaching and historical XR packet.
- `python3 tools/check_vault_links.py` checks files, headings, blocks and embedded figures without writing to the vault.
- `python3 tools/check_patch_idempotence.py` runs all four retained authoring patches twice in an isolated temporary copy; it fails if a patch still changes an already-integrated book.
- `python3 -m unittest discover -s tools/tests -p test_note_integrity.py` tests Markdown parsing, whole-program identity, preservation exceptions and navigation failures.

The eight revised/new C++ labs were compiled and executed with Apple Clang under strict warnings-as-errors, optimized `-DNDEBUG`, and AddressSanitizer/UndefinedBehaviorSanitizer profiles. The four Python witnesses were executed normally and with `-O`; their validation survives optimization. Recorded receipts include initial failures and subsequent successful reruns. Final counts and equation/diagram checks are recorded in the private verification checkpoint linked below.

The final-pass workflows now have read-only repository permissions and verify the checked-in books. They no longer apply or push repairs. They run on the checked-out revision rather than silently targeting an authoring branch. Native Vulkan/Metal, production-engine integration and human-subject XR evidence are separate; this pass does not re-certify them or every existing code fragment.

## Caption and acquisition-tool cleanup decision

The deletion condition is **not met**. The retained source ledger has 590 extracted records and 29,715 claim texts: 28,485 reviewed and 1,230 pending. The newer 693-job queue contains 590 extracted jobs, 96 queued jobs, four failed book jobs, two paused book jobs and one visual-review job. Its 103 open jobs and lack of complete per-source merge proof prevent certifying that all captions are accounted for.

All original captions, ASR text, book extracts, source-unit receipts and acquisition scripts remain available. No acquisition run was started. The current work does not claim full reading of the seven mathematics PDFs' 4,205 pages. Only untracked, reproducible Python bytecode caches were removed; ignore rules keep future interpreter/Finder clutter out of new commits without altering existing tracked workspace metadata.

Source retirement can proceed later under the user's existing conditional authorization once exact source contributions and destinations are closed. A read claim or a similar topic heading alone does not satisfy that condition.

## Recovery and continuation

[Repair checkpoint](/Users/swayamsingal/.codex/source-cache/vault-repair-2026-11-14/WORK_STATE.json) · [Verification receipts](/Users/swayamsingal/.codex/source-cache/vault-repair-2026-11-14/verification-final.json) · [Cleanup audit](/Users/swayamsingal/.codex/source-cache/vault-repair-2026-11-14/cleanup-audit.json) · [Retained corpus ledger](/Users/swayamsingal/.codex/source-cache/manual-integration-2026-11-10/ALL_SOURCES_BACKLOG.json).

The pre-repair snapshot is retained outside the vault. No commit, push, publication or source-corpus retirement is part of this local repair. Whole-corpus closure remains open.
