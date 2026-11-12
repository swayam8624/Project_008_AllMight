"""Real ONNX protobuf/checker/shape-inference/reference-execution witness.

This is a boundary test for PROJECTALLMIGHT Phase 29. It deliberately uses the
official ONNX package rather than a home-grown representation when proving the
serialized interchange contract.
"""
from __future__ import annotations

from pathlib import Path
import sys
import tempfile

import numpy as np
import onnx
from onnx import TensorProto, checker, helper, numpy_helper, shape_inference
from onnx.reference import ReferenceEvaluator


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def make_model() -> onnx.ModelProto:
    x = helper.make_tensor_value_info("x", TensorProto.FLOAT, ["N", 3])
    y = helper.make_tensor_value_info("y", TensorProto.FLOAT, ["N", 2])

    w_np = np.array(
        [[1.0, -2.0],
         [0.5,  3.0],
         [2.0,  1.0]],
        dtype=np.float32,
    )
    b_np = np.array([0.25, -0.5], dtype=np.float32)

    w = numpy_helper.from_array(w_np, name="w")
    b = numpy_helper.from_array(b_np, name="b")

    nodes = [
        helper.make_node("MatMul", ["x", "w"], ["xw"], name="matmul"),
        helper.make_node("Add", ["xw", "b"], ["biased"], name="bias"),
        helper.make_node("Relu", ["biased"], ["y"], name="relu"),
    ]
    graph = helper.make_graph(
        nodes,
        "allmight_linear_relu",
        [x],
        [y],
        initializer=[w, b],
    )
    model = helper.make_model(
        graph,
        producer_name="PROJECTALLMIGHT",
        opset_imports=[helper.make_opsetid("", 18)],
    )
    # Keep the witness on an IR version supported by the installed package and
    # represented by the current model object rather than hardcoding an old IR.
    return model


def shape_tuple(value_info: onnx.ValueInfoProto) -> tuple[object, ...]:
    result: list[object] = []
    dims = value_info.type.tensor_type.shape.dim
    for dim in dims:
        if dim.HasField("dim_value"):
            result.append(dim.dim_value)
        elif dim.HasField("dim_param"):
            result.append(dim.dim_param)
        else:
            result.append(None)
    return tuple(result)


def test_round_trip_and_shape_inference(work: Path) -> None:
    model = make_model()
    checker.check_model(model)

    path = work / "linear_relu.onnx"
    onnx.save_model(model, path)
    require(path.is_file() and path.stat().st_size > 0, "ONNX serialization produced no file")

    loaded = onnx.load_model(path)
    checker.check_model(loaded)
    require(loaded.graph.name == "allmight_linear_relu", "graph name changed during protobuf round trip")
    require([node.op_type for node in loaded.graph.node] == ["MatMul", "Add", "Relu"],
            "node/operator order changed during round trip")
    require({init.name for init in loaded.graph.initializer} == {"w", "b"},
            "initializer identity changed during round trip")

    inferred = shape_inference.infer_shapes(loaded)
    by_name = {value.name: value for value in inferred.graph.value_info}
    require("xw" in by_name and "biased" in by_name,
            "shape inference did not annotate intermediate values")
    require(shape_tuple(by_name["xw"]) == ("N", 2),
            f"MatMul inferred shape mismatch: {shape_tuple(by_name['xw'])}")
    require(shape_tuple(by_name["biased"]) == ("N", 2),
            f"Add inferred shape mismatch: {shape_tuple(by_name['biased'])}")

    x_np = np.array(
        [[1.0, 2.0, -1.0],
         [0.0, -2.0, 4.0]],
        dtype=np.float32,
    )
    expected = np.maximum(
        x_np @ np.array(
            [[1.0, -2.0],
             [0.5,  3.0],
             [2.0,  1.0]],
            dtype=np.float32,
        ) + np.array([0.25, -0.5], dtype=np.float32),
        0.0,
    )
    evaluator = ReferenceEvaluator(inferred)
    (actual,) = evaluator.run(None, {"x": x_np})
    require(np.array_equal(actual, expected),
            f"ONNX reference execution mismatch:\nactual={actual}\nexpected={expected}")


def test_checker_rejects_invalid_graph() -> None:
    x = helper.make_tensor_value_info("x", TensorProto.FLOAT, [1])
    y = helper.make_tensor_value_info("y", TensorProto.FLOAT, [1])

    # Two node outputs reuse the same name, violating ONNX graph SSA.
    bad_graph = helper.make_graph(
        [
            helper.make_node("Identity", ["x"], ["dup"]),
            helper.make_node("Identity", ["x"], ["dup"]),
            helper.make_node("Identity", ["dup"], ["y"]),
        ],
        "bad_ssa",
        [x],
        [y],
    )
    bad_model = helper.make_model(
        bad_graph,
        producer_name="PROJECTALLMIGHT",
        opset_imports=[helper.make_opsetid("", 18)],
    )

    rejected = False
    try:
        checker.check_model(bad_model)
    except checker.ValidationError:
        rejected = True
    require(rejected, "official ONNX checker accepted duplicate SSA output names")


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="allmight-onnx-") as td:
        test_round_trip_and_shape_inference(Path(td))
    test_checker_rejects_invalid_graph()

    print("onnx_contract: PASS")
    print(
        "proved: real ModelProto serialization/deserialization, opset metadata, "
        "official checker validation, symbolic shape inference, initializer identity, "
        "MatMul/Add/Relu reference execution, invalid-SSA rejection"
    )
    print(
        "not proved: arbitrary opsets/operators, dynamic control-flow/subgraphs, external tensor data, "
        "ONNX Runtime/backend lowering, quantized operators, custom domains, accelerator kernels, "
        "or current KairoONNX production parity"
    )


if __name__ == "__main__":
    main()
