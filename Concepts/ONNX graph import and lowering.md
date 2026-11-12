# ONNX graph import and lowering

ONNX is a graph interchange format, not a command stream to execute blindly. Import therefore has two separate responsibilities: decode the normative serialized model, then lower its graph semantics into the engine’s own validated intermediate representation.

The ONNX IR describes a graph as a side-effect-free computation containing model parameters and a topologically ordered list of nodes. Node outputs obey single-static-assignment: an output value name is produced once. Inputs, outputs and initializers have declared types and shapes, and symbolic dimensions can constrain values across the graph.

Primary reference: https://onnx.ai/onnx/repo-docs/IR.html

## Import pipeline

Parse. Decode Protocol Buffers into ONNX model objects. Reject malformed serialization before graph reasoning begins.

Validate names and versions. Check IR version, opset imports, graph name, duplicate names and unsupported domains.

Resolve value contracts. Establish element type, rank and known or symbolic dimensions for graph inputs, outputs, initializers and intermediate values.

Validate graph order and SSA. Every consumed value must either be a graph input, initializer or output of an earlier node; every node output must be unique in its namespace.

Normalize operators. Convert ONNX attributes and optional inputs into an internal operator description whose defaults are explicit.

Infer and verify shapes. Propagate shapes where operator rules permit it, while keeping an unknown dimension distinct from an invented concrete dimension.

Lower. Translate supported operators into backend-neutral internal nodes. Unsupported operations must produce a deterministic diagnostic, not fall through to an unrelated kernel.

Plan execution. Topologically schedule nodes, assign storage and lifetimes, select kernels by capabilities, and insert device transfers or fallback boundaries deliberately.

## Why an internal IR matters

If Vulkan, Metal and CPU code each interpret ONNX attributes independently, the repository effectively owns multiple importers with multiple chances to disagree. The importer should instead produce one small semantic IR: tensors, operators, attributes, shapes, constants and explicit capability requirements. Backends lower that IR.

This creates a testable seam. A CPU reference kernel and a Metal kernel can receive the same normalized MatMul or Softmax node. Their numerical agreement can then be tested without re-testing protobuf parsing.

## Executable graph contract

tools/labs/phase29_ml_runtime_lab.cpp does not parse protobuf and labels that limitation explicitly. It models the post-decode internal graph boundary and verifies that inputs and initializers are available before use and that node outputs satisfy SSA uniqueness. Negative cases reject use-before-definition and duplicate outputs.

That is deliberately narrower than ONNX support. Full support requires actual protobuf decoding, opset semantics, operator-specific validation, shape inference and numerical parity against a trusted runtime.

Related: [[Tensor runtime layout and views]], [[Transformer attention and KV cache runtime]], [[Interface contracts and publication]].
