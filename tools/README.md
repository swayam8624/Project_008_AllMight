# Local notes pipeline

`notes_pipeline.py` helps collect public YouTube subtitle tracks and source documents, preserve them in a local cache, and prepare a source-linked draft with a local Ollama model. It is a research assistant: model output is a draft for human review, not a verified transcript or an automatic edit to the continuous engine book.

## Evidence rules

- A subtitle track downloaded by `yt-dlp` is caption evidence. Preserve the language/track type, source ID, timestamp cues, acquisition result, and hashes. A failed, empty, or rate-limited subtitle request is not a transcript.
- Do not ask the model to fill gaps in missing captions. Mark the source unavailable or partial and keep any secondary evidence separate.
- A supplied PDF or companion document is a secondary source. Keep its title, page numbers, file hash, and extracted text associated with that source. It does not become a caption track and cannot prove what was said in a video.
- Keep narrated claims, extracted document text, model summaries, and human-verified claims distinguishable. A generated draft does not establish code behavior, a build, runtime output, or what appeared in a video frame.
- Keep source files and cache receipts. A later pass should be resumable from saved inputs rather than silently replacing provenance.

## Prerequisites

1. Install and start [Ollama](https://docs.ollama.com/quickstart). The pipeline uses its local server at `http://localhost:11434`; local model requests do not need a cloud API key. Check the server and installed models with:

   ```sh
   ollama list
   ollama ps
   ```

   The default model is `qwen3:4b`. If it is not installed, pull it with `ollama pull qwen3:4b`. A larger local model such as `qwen3:8b` can be selected when the machine has enough memory. Larger models may improve synthesis, but use more memory and can run much more slowly on CPU. Check the current [Ollama CLI](https://docs.ollama.com/cli) and [API documentation](https://docs.ollama.com/api) for local model management and generation.

2. Install `yt-dlp` and confirm it is on `PATH`:

   ```sh
   python3 -m pip install yt-dlp
   yt-dlp --version
   ```

   Subtitle options and language selection are documented in the [yt-dlp README](https://github.com/yt-dlp/yt-dlp#subtitle-options). The pipeline processes public subtitle access sequentially, ignores yt-dlp user configuration, strips proxy variables, and does not use account cookies or client-switch workarounds.

3. Use Python 3.10 or newer. PDF import uses `pypdf`; it is optional for subtitle-only runs. Install it in the Python environment used by the script if you need text extraction from PDFs:

   ```sh
   python3 -m pip install pypdf
   ```

   See the [pypdf installation guide](https://pypdf.readthedocs.io/en/latest/user/installation.html). PDF text extraction can misread layout, tables, equations, or scanned pages; check selected source pages before relying on extracted text.

## Paths and cache

The script uses a per-user cache by default: `~/Library/Caches/notes-pipeline` on macOS, `$XDG_CACHE_HOME/notes-pipeline` when `XDG_CACHE_HOME` is set, or `~/.cache/notes-pipeline` otherwise. Use the global `--state-dir` option to point at an existing source cache or to keep state in a chosen location. Do not point it at the live book file. Keep source inputs and generated drafts separate from the continuous engine volume until a human has reviewed them.

From the active Master repository root, set the vault root from Git and the same portable cache location the script uses by default:

```sh
export MASTER_VAULT="$(git rev-parse --show-toplevel)"
case "$(uname -s)" in
  Darwin*) export SOURCE_CACHE="$HOME/Library/Caches/notes-pipeline" ;;
  *) export SOURCE_CACHE="${XDG_CACHE_HOME:-$HOME/.cache}/notes-pipeline" ;;
esac
mkdir -p "$SOURCE_CACHE"
```

`MASTER_VAULT` is used only when you deliberately choose a draft output location. `SOURCE_CACHE` holds raw source files, metadata, transcripts, receipts, model-call checkpoints, and generated drafts.

## Workflow

The global options go before the subcommand: `python3 tools/notes_pipeline.py [--state-dir PATH] [--ollama-url URL] COMMAND ...`.

### 1. Check local tools and model

```sh
python3 tools/notes_pipeline.py --state-dir "$SOURCE_CACHE" doctor --model qwen3:4b
```

The doctor check reports Python and `yt-dlp`, checks whether optional `pypdf` and Whisper are installed, and resolves the chosen model through the localhost-only Ollama endpoint. Run it before a build to catch a missing local model or dependency.

### 2. Add existing material or inspect a playlist

Import one existing local VTT, SRT, TXT, or PDF into the source cache. Directories are not accepted. Preserve the evidence type: mark a book or companion PDF as `secondary_pdf`; it is never a YouTube caption. For a PDF, select physical page numbers explicitly with `--pages` when only part of it is relevant. Page numbers are one-based and remain attached to extracted text. The command returns a deterministic source ID; use that ID for a build rather than the video's ID.

```sh
python3 tools/notes_pipeline.py --state-dir "$SOURCE_CACHE" import "/path/to/source-file.pdf" --origin "book-short-name" --kind secondary_pdf --pages 5-6 --title "Book title"
```

Playlist inventory is metadata discovery. It does not count as caption acquisition or full reading:

```sh
python3 tools/notes_pipeline.py --state-dir "$SOURCE_CACHE" inventory "https://www.youtube.com/playlist?list=PLAYLIST_ID"
```

Use the import command for a local file you already collected rather than fetching that source again. Preserve its origin label, title, caption language/track type where applicable, and hashes. There is no directory-level cache importer.

### 3. Fetch available captions in small sequential batches

```sh
python3 tools/notes_pipeline.py --state-dir "$SOURCE_CACHE" fetch --limit 50 --languages en,en-US,en-GB,en-orig
```

The fetcher processes sources sequentially, up to 50 per invocation. It prefers one available manual English subtitle track and falls back to an automatic English track when needed. Add `--playlist-id ID` to select a saved inventory; otherwise it uses the most recently inventoried playlist. If a request gets HTTP 429, acquisition stops and a persistent global cooldown is recorded using the server's `Retry-After` value when present, otherwise one hour. Re-running while the cooldown is active makes no request; re-run after expiry to resume. Do not loop or switch clients to get around a limit. Failed non-rate-limited requests are skipped by default; `--retry-failed` explicitly retries those outcomes. Sign-in, age, bot, or private-video restrictions remain access outcomes.

When neither manual nor automatic English captions are listed, the fetch result records `no_english_track`. Do not generate substitute captions. The fetch command does not download audio or run ASR.

### 4. Optionally transcribe audio you already have

`transcribe` accepts one local audio file; it does not acquire media. `doctor` reports if Whisper is missing from the current Python. Use [`uv`](https://docs.astral.sh/uv/getting-started/installation/) to run the command with Whisper in a temporary environment instead of changing the base Python installation:

```sh
uv run --no-project --python 3.11 --with openai-whisper==20250625 \
  python tools/notes_pipeline.py --state-dir "$SOURCE_CACHE" transcribe \
  "/path/to/already-downloaded-audio.webm" \
  --model base.en \
  --title "Episode title"
```

The result is imported as `local_asr`, with audio and generated VTT hashes, model/version provenance, and a `generated_not_human_reviewed` status. The generated VTT and receipt are stored in the state directory; keep the original audio file at its input path because the tool records its hash but does not copy the audio. Whisper stores model weights in its own user cache. ASR remains separate from downloaded captions and needs a full transcript review before any source counts or teaching claims are updated.

### 5. Build a reviewable draft

Select sources by their stable IDs, use a bounded number of model calls, and keep the draft in the cache while reviewing it:

```sh
python3 tools/notes_pipeline.py --state-dir "$SOURCE_CACHE" build \
  --source SOURCE_ID \
  --model qwen3:4b \
  --max-calls 100 \
  --chunk-chars 12000 \
  --out "$SOURCE_CACHE/draft"
```

If `--source` is omitted, the build includes all imported sources. Repeat `--source ID` to select several. Use a source ID printed by `import` or saved by `fetch`, not the YouTube video ID. The build uses three bounded local-model passes: extract and account for source points and timestamp/page locators; compose those points in bounded groups into a connected teaching draft; and run an independent audit pass against the source ledger. Model calls are cached by model/source/prompt/schema identity and checkpoints are written atomically, so an interrupted build can resume from saved work. The default Ollama context is 32,768 tokens. Before each call, the pipeline also checks a conservative 24,000-byte UTF-8 budget for the system prompt, user prompt, JSON schema, and overhead; oversized prompts are rejected instead of silently clipped. Every pass is model-assisted, not proof of correctness. Inspect saved inputs/results, source references, and audit output. `--max-calls` bounds uncached model requests and `--chunk-chars` limits each source chunk. Larger models may use more memory; CPU inference may take substantially longer than GPU-backed inference. Start with `qwen3:4b` and raise model size or call budget only when a checkpoint exposes a concrete need.

The draft should retain exact video timestamps or PDF page locations. Before copying anything into the active vault, compare every technical claim to its cited source, confirm that missing portions remain missing, and preserve the original source/cache receipts.

For dense book pages, use smaller source groups and a larger local output allowance: `build --chunk-chars 6000 --output-tokens 8192`. The default output allowance is 2,048 tokens per call. A response that hits its allowance is rejected and checkpointed; increase `--output-tokens` or reduce the source group rather than accepting truncated teaching. Changing generation settings creates a separate cache identity. PDF imports default to `secondary_pdf`; they cannot be labelled as YouTube captions or generated ASR.

## Resume and outputs

### Whole engine corpus: knowledge before prose

`engine_corpus.py` imports the existing audited engine inventory and the supplied books. It preserves the distinction between downloaded captions, generated ASR, and PDF text. `run_engine_corpus.py` plans timestamped video spans and disjoint book page ranges; it retains the original cue/page ancestry. Neither script edits the continuous book.

The current engine run uses a dedicated cache, separate from the earlier smoke run:

```sh
export ENGINE_CORPUS="$HOME/.codex/source-cache/engine-corpus-2026-11-07"
python3 tools/run_engine_corpus.py --state-dir "$ENGINE_CORPUS" status
```

After an audited import, plan the corpus once and extract knowledge in bounded groups. Check for an existing worker before launching another; the runner enforces a single writer with a PID lock. Do not re-import from an older audit or replan an active queue simply to resume it.

```sh
python3 tools/run_engine_corpus.py --state-dir "$ENGINE_CORPUS" plan --pages-per-job 12
python3 tools/run_engine_corpus.py --state-dir "$ENGINE_CORPUS" work \
  --phase extract --max-jobs 50 --max-calls 200 \
  --model qwen3:4b --output-tokens 8192 --chunk-chars 4000
```

`--max-jobs 0` processes all eligible jobs once; `--max-calls` is a per-job allowance. `--retry-failed` explicitly includes failed jobs. Successful source extraction is saved as `knowledge_extracted`, with original source hashes, `points.json`, `source-ledger.json`, and a pending `supervisor-pack.json`. A complete page/cue inventory does **not** prove that every detail was retained. For example, a model may retain a proposed test procedure while losing its pass/fail criteria. Blank or nonextractable pages require visual inspection rather than fabricated text.

Before using an extraction for teaching, run a separate detail check against its original units:

```sh
python3 tools/audit_engine_extraction.py JOB_ID \
  --state-dir "$ENGINE_CORPUS" --model qwen3:4b --max-calls 100
```

This saves `drafts/JOB_ID/detail-audit.json`: proposed missing points and corrections keyed to original point IDs and source units. It leaves `points.json` unchanged. Interrupted checks reuse validated model-call caches when repeated. The report records its input extraction hash; reject a stale report if those points change. This is another model-assisted check, not a guarantee of completeness. Review the reported omissions, numerical conditions, first-use definitions, source qualifications, and any unsupported implementation claims against the actual source before composing prose. Resolve every accepted amendment explicitly; the narrative builder does not silently consume this sidecar.

The report also preserves exact source spans marked `Verification question:` or `ENGINEERING CHECK:` as `required_source_details`. These anchors remain unresolved until supervision maps them to teaching or gives an explicit disposition. This catches marked questions and criteria independently of a model response, but does not detect all unmarked details. The companion calibration found nine omitted questions and twelve omitted acceptance criteria; the first local audit missed confirmed problems. Empty model findings therefore provide no clearance to merge.

Use `--phase all --only JOB_ID` to generate a review draft from a selected extraction when appropriate. The standalone `notes_pipeline.py build --extract-only` also supports knowledge-only work. Full builds produce local paragraph audits, but those audits cannot detect every detail omitted during initial extraction. The book merge remains a separate supervised operation. A source/job cannot approve itself: the status report counts external decisions only when their source and unit hashes match the queue. A local supervisor decision is distinct from human review, video-frame inspection, compilation, or runtime evidence.

`PROGRESS.md`, `worker-status.json`, `queue.json`, and `WORK_STATE.json` under the engine cache are the resume records. Source counts, extraction counts, accepted teaching, and live merges must remain separate. Preserve the saved YouTube cooldown and inaccessible-source records when continuing acquisition in batches of at most 50 new requests.

### Concurrent local extraction

`run_engine_parallel.py` runs independent source jobs concurrently. One coordinator holds the existing corpus lock and exclusively updates the shared queue and progress files. Each source has its own output directory and child process; threads wait for those children while Ollama serves the requests. Completed checkpoints and validated response caches remain reusable. It performs extraction only and never edits the live teaching book.

```sh
python3 tools/run_engine_parallel.py --state-dir "$ENGINE_CORPUS" \
  --workers 2 --max-jobs 50
```

Start the local Ollama server with `OLLAMA_NUM_PARALLEL=2` before using two workers. Each worker retains the existing model, 32,768-token context, 8,192-token output allowance, and source validation. The 200 uncached-call allowance is **per source job**, not a global budget. Use only one corpus coordinator at a time. Parallel requests increase model memory use; more CPU cores do not imply more useful inference slots on a GPU-bound model. Benchmark throughput on the actual machine. The initial small two-request probe measured about 1.04 times serial throughput; this is not a full-corpus speed or quality result.

Ctrl-C asks workers to stop their owned extraction children, saves paused statuses, and leaves completed caches intact. Explicitly paused source jobs must be returned to `queued` only when the user asks to resume. Network caption collection can run separately in its acquisition cache, retaining its sequential requests and saved rate-limit cooldown. Admit new captures only between corpus batches, while holding the corpus lock.

The state directory is the resume point. Keep it intact between runs. Playlist metadata is written to `inventory/<playlist-id>.json`; each subtitle attempt is recorded in `fetch/<video-id>.json`; model responses are cached in `model-cache/`; and the shared 429 pause is `cooldown.json`. Failed fetches are skipped by default; `--retry-failed` explicitly retries those failed outcomes. A `rate_limited` source remains governed by its stored cooldown. Do not manually edit receipts to make a source appear reviewed.

By default, each build writes `runs/<run-id>/draft.md`, `review.md`, `source-ledger.json`, `points.json`, `audit.json`, and `build-state.json` under the state directory. With `--out PATH`, those files go directly into that output directory. Read `review.md` first: it compactly reports source/unit counts, ignored units, points left out of prose, and model-flagged paragraphs. Then inspect the draft and supporting ledgers. The draft includes page/timestamp footnotes, marks inferences and examples, and lists the claim text for points not woven into prose. The ledger keeps captions, local ASR, and secondary PDF evidence distinct. `audit.json` is a model-generated review aid, not factual certification or human review; check claims and source locators yourself.

To keep the run in a stable, portable cache and then copy a reviewed draft into a clearly labeled vault holding area:

```sh
python3 tools/notes_pipeline.py --state-dir "$SOURCE_CACHE" build \
  --source SOURCE_ID \
  --model qwen3:4b \
  --max-calls 100 \
  --chunk-chars 12000 \
  --out "$SOURCE_CACHE/review-run"

sed -n '1,200p' "$SOURCE_CACHE/review-run/review.md"
mkdir -p "$MASTER_VAULT/Supplementary/Engine Drafts"
cp "$SOURCE_CACHE/review-run/draft.md" \
  "$MASTER_VAULT/Supplementary/Engine Drafts/review-run.md"
```

This copies a draft for review; it does not merge the material into the continuous book.

## Smoke-check observations

The latest local smoke run successfully fetched one public automatic-English caption track containing 164 cues. A three-pass build from selected PDF page 5 completed with 16 extracted points, nine draft paragraphs, and 13 uncached model calls, including a validation-repair call. Re-running that build with `--max-calls 0` completed from cache with zero new model calls.

The smoke source set included 33 selected PDF pages imported and classified as secondary sources. The smoke build processed selected page 5 only; it does not demonstrate full-book PDF synthesis. These results show the exercised path on those inputs, not caption or PDF coverage beyond them. Model output and the model audit still require human source review.

## Current limits

- Subtitle availability varies by source. A missing track is an access/evidence gap, not permission to invent dialogue.
- Playlist metadata, extracted PDF text, ASR, and YouTube captions are different evidence types.
- A successful Ollama response only shows that a model returned text. Human source review is still required.
- This tool does not prove an engine build or runtime and does not publish or commit the generated draft.
