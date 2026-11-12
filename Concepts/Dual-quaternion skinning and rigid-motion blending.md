# Dual-quaternion skinning and rigid-motion blending

A unit quaternion represents rotation but not translation. A dual quaternion

[
\hat q = q_r + \varepsilon q_d,qquad \varepsilon^2=0
]

can encode a rigid transform compactly. For unit rotation quaternion (q_r) and translation vector (t) represented as a pure quaternion (t_q=(t,0)),

[
q_d=\frac12 t_q q_r.
]

A point is transformed by the encoded rigid motion; translation can be recovered from

[
t_q=2q_d q_r^*.
]

This is useful for skinning because linearly blending transformation matrices can introduce volume loss and “candy-wrapper” artifacts around twisting joints. Blending dual quaternions better preserves rigid character before the final weighted transform is applied.

The same antipodal issue as ordinary quaternion interpolation remains: (q) and (-q) encode the same rotation, but naïvely averaging opposite signs can cancel. Before weighted accumulation, choose a reference real quaternion and flip contributors whose real part has negative dot product with that reference.

After blending, normalize the dual quaternion consistently. Production implementations should also enforce the unit-dual-quaternion orthogonality condition rather than treating real and dual parts as unrelated vectors.

Dual-quaternion skinning is not universally superior. Non-rigid bone scales and shear are not represented by a pure rigid dual quaternion; engines often need a policy for scale, matrix fallback, or separate scale blending.

**Executable witness:** `tools/labs/dual_quaternion_skinning_lab.cpp`.

**Required next depth:** compare linear-blend skinning and dual-quaternion skinning on a twisting cylinder with bind-pose/inverse-bind conventions, antipodal bones, nonuniform scale and GPU/CPU parity.
