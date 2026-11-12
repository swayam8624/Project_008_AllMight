# ONNX graph SSA validation and lowering

ONNX is useful to an engine runtime because it makes dataflow explicit. A graph contains inputs, outputs, initializers and operator nodes. Node outputs obey single-static-assignment naming and the top-level node list is topologically ordered: a node cannot consume a value that has not already been defined.

An importer should not immediately fuse or execute while parsing. First establish a trustworthy graph:

serialized model → structural validation → opset/domain resolution → value/type registration → node signature validation → internal IR → shape reasoning → capability analysis → backend lowering → memory planning → executable schedule.

Attributes are static node configuration. Inputs/outputs are runtime values. Mixing those categories creates poor APIs and confuses constant folding.

The internal IR should record operator kind/version, ordered inputs/outputs, static attributes, source identity for diagnostics, inferred shape/type information and backend/fallback state. Fallbacks should be observable; silent CPU fallback makes profiling evidence ambiguous.

**Executable witness:** `phase29_ml_runtime_lab.cpp` rejects undefined reads and duplicate/SSA-invalid outputs.

**Next depth:** real protobuf import, opsets, symbolic dimensions, control-flow subgraphs, operator lowering and parity with ONNX Runtime on a small fixture.
