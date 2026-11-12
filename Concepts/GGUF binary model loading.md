# GGUF binary model loading

GGUF is a binary container used by GGML-family inference runtimes. Treating it as “a file full of weights” is too vague for a safe loader. A loader must first establish a byte-level contract: magic, format version, tensor count, metadata count, typed metadata records, tensor descriptors, alignment, tensor-data offsets, and bounds. Only after those checks can tensor bytes become runtime values.

The first four bytes identify the format as GGUF. Multi-byte fields must be decoded according to the file format instead of reinterpreting an arbitrary file buffer as a C++ struct. Native C++ structs inherit compiler padding, alignment and host-representation assumptions that a binary file does not owe the process reading it.

A useful loading pipeline is:

1. Read and validate the fixed prefix.
2. Parse metadata key/value records with checked lengths.
3. Parse tensor descriptors: name, rank and shape, data type, and relative data offset.
4. Compute the aligned start of the tensor-data section.
5. Validate every tensor interval against file length and arithmetic overflow.
6. Keep file or mapping ownership alive for every lazy tensor view.
7. Convert the file tensor type into an internal runtime type only after the descriptor is validated.
8. Dispatch dequantization or quantized kernels from that internal type rather than from filename assumptions.

The official GGUF documentation describes the format as a single-file, extensible model container and defines architecture metadata, tensor data layout information, and attention metadata such as head counts and key/value dimensions. Those metadata values affect runtime shape contracts; they are not merely labels.

Primary reference: https://github.com/ggml-org/ggml/blob/master/docs/gguf.md

## Prefix witness

tools/labs/phase29_ml_runtime_lab.cpp parses the GGUF magic, version, tensor count and metadata count from an explicit byte vector using checked little-endian reads. It has a negative case for invalid magic.

That witness intentionally stops at the prefix. It proves that the repository now owns the boundary between raw bytes and validated scalar header fields; it does not pretend that the entire GGUF metadata type system or quantized tensor catalog has already been implemented.

## What a full loader must prove

A production reader must defend against truncated strings, absurd ranks, shape-product overflow, unsupported quantization types, alignment overflow, tensor intervals outside the file, duplicate or contradictory metadata, offsets that alias unintentionally, and files whose declared architecture cannot be executed by the runtime.

For memory mapping, “the pointer is valid” is not enough. The mapping object and file lifetime must outlive every lazy tensor view. Page faults also become part of performance: a model that technically maps instantly can still stall during first use if access order faults pages unpredictably.

## Quantized execution boundary

GGUF storage type and execution type are separate decisions. A block-quantized format normally stores scale or other metadata together with packed codes. A portable reference path should decode or dot-product those blocks in a simple scalar implementation first. SIMD or Metal kernels then have an independent oracle. If the optimized path disagrees, the scalar path is evidence; if both paths share the same unpacking bug, the test is not independent enough.

Related: [[Tensor runtime layout and views]], [[Quantized matvec and dequantization]], [[Transformer attention and KV cache runtime]], [[Binary format contracts]].

## Metadata and tensor-descriptor witness

The earlier prefix-only boundary has now been extended by `tools/labs/gguf_metadata_tensor_lab.cpp`. The subset parser validates GGUF v3 magic and counts, parses typed UINT32 and STRING metadata, consumes `general.alignment`, reads tensor names/ranks/dimensions/type IDs/relative offsets, computes the aligned tensor-data start, and rejects truncation or invalid alignment.

The parser remains deliberately incomplete rather than permissive. Unsupported metadata types are rejected instead of skipped with guessed sizes. It does not yet implement arrays, every GGUF value type, every GGML quantized tensor block, tokenizer metadata, endian conversion for big-endian files, or payload interval validation for all tensor types. That is the correct boundary for an independent reference: a small loader should fail loudly outside the subset it actually understands.

