# Shader buffer ABI

A shader buffer is a binary interface between independently compiled code. CPU and shader code must agree on member offsets, element widths, alignment, array stride, matrix layout, and total record stride. “These structs have the same fields” is not enough.

Suppose a record logically contains a 3-vector followed by a scalar. The mathematical payload is four floats, but the ABI may impose an alignment boundary for the vector or the containing record. The only safe statement is the one established by the layout rules and, ideally, reflection/compiled shader metadata.

## Layout is a function of context

Different interfaces can have different rules: vertex attributes, constant/uniform buffers, storage buffers, argument buffers and push/root constants are not interchangeable contracts. Matrix row/column-major convention is another independent dimension.

On the CPU side, standard C++ object layout also has rules about padding and alignment. Copying bytes from a native struct into a shader buffer is valid only when the native representation is deliberately made compatible or explicit serialization writes the expected offsets.

## Verify rather than guess

A robust pipeline should generate or reflect a machine-readable layout description and compare it with CPU declarations:

- expected offset for every member;
- expected array stride;
- matrix stride and orientation;
- record size/alignment;
- scalar widths and signedness.

Static assertions can catch CPU layout changes. Shader reflection or compiler metadata catches shader-side changes. A golden byte dump can test the complete bridge.

## Failure modes

Common failures include using a 12-byte mental model for a vector type whose ABI slot is 16 bytes, mixing bool widths, assuming packed enums, copying pointer-containing structs, row/column-major disagreement, and updating one side of the interface without rebuilding the other.

Teach and reconstruct: [[Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs#Make a shader record an explicit binary contract]].

Related: [[Alignment and padding]], [[GPU vertex specification]], [[Tensor runtime layout and views]].
