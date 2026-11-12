# Production manifests and transactional publication

A production manifest is a durable description of what should be built and published. It prevents a pipeline from depending on hidden editor state or on “whatever files happen to be in this directory.”

A useful manifest separates identity from location. A logical asset ID names the thing the project means. Source and output paths say where a particular revision is read and written. A cryptographic digest can identify exact content bytes. A schema version says how to interpret all fields.

## Path safety

Normalize paths before planning any write. A production-safe policy normally rejects parent traversal, unexpected absolute paths, ambiguous separators, and paths that escape the declared project root after symlink resolution.

Lexical normalization alone cannot prove symlink safety. The filesystem must be queried at the point where real paths are resolved, and the resolved target must still lie inside the approved root.

## Deterministic plans

The same manifest and tool versions should produce the same ordered build plan. Determinism makes logs diffable, cache keys stable and failures reproducible.

Planning should be side-effect free. A dry run should reveal every intended output, overwrite, deletion and dependency before publication begins.

## Staging and commit

A safe publication sequence is:

1. validate the whole request;
2. write derived outputs to staging names or directories;
3. validate staged bytes and metadata;
4. fsync or equivalent when durability is part of the promise;
5. publish with the platform’s proven atomic replacement mechanism;
6. update manifest/index state;
7. retain enough evidence to diagnose rollback or partial failure.

Atomicity and durability are separate. A rename can make a new name appear atomically yet still not guarantee survival across power loss unless the required data and directory metadata were persisted.

## Idempotence

Repeating the same publication should either be a no-op or produce the same visible state. A pipeline that appends duplicate records, increments versions accidentally or rewrites unrelated timestamps on every run is harder to cache and reason about.

tools/labs/production_pipeline_contract_lab.cpp models deterministic plan generation and idempotent publication in memory. It intentionally labels real filesystem durability as unproved.

Related: [[Atomic publication and durability]], [[DCC adapters and engine publication]].
