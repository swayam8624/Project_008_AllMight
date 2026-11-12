# GGUF container and tensor directory

GGUF is a self-contained binary model container used by GGML-family runtimes. The important runtime lesson is not memorizing a magic number; it is building a parser whose publication boundary is later than its byte-reading boundary.

A loader should parse and validate the fixed header, metadata key/value records, tensor descriptors, required alignment and tensor-data ranges before publishing any tensor view. Tensor descriptors identify a name, dimensions, type and an aligned offset into the tensor-data region. The byte extent of a quantized tensor depends on the selected GGML block type, so a loader must not assume “elements × scalar byte size.”

Treat all counts, string lengths, dimensions, offsets and type codes as untrusted. Use checked arithmetic, explicit endianness, defensible count limits and overlap/bounds checks. If tensor views borrow a memory mapping, their lifetime is subordinate to the mapping owner.

The current authoritative format description is maintained by ggml under `docs/gguf.md`; PROJECTALLMIGHT should always distinguish that external format authority from its own reference parser.

**Executable witness:** the Phase-29 lab validates the fixed header and malformed-magic/truncation boundary. It is intentionally not a full GGUF parser.

**Next depth:** metadata values, tensor-directory extents, selected quantization block decoding, mmap ownership and differential tests against a trusted reader.
