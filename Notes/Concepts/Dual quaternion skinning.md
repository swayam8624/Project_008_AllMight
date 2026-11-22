# Dual quaternion skinning

Linear blend skinning blends transformed vertex positions or bone matrices. It is simple and GPU-friendly, but blending rotation matrices componentwise is not itself a rigid transformation and can collapse volume around twisting joints.

Dual quaternions provide a representation of rigid motion that blends rotation and translation together before transforming the point.

A dual quaternion is

$$
\hat q=q_r+\varepsilon q_d,
\qquad \varepsilon^2=0,
$$

where $q_r$ is a unit rotation quaternion. For translation vector $t$ represented as pure quaternion $t_q=(0,t)$ under one common convention,

$$
q_d=\frac12 t_q q_r.
$$

The translation can be recovered by

$$
t_q=2q_dq_r^*.
$$

## Antipodal problem

Rotation quaternions $q$ and $-q$ encode the same physical rotation. Their corresponding dual quaternions likewise differ by a global sign. If two equivalent bones/poses are blended without hemisphere alignment, their real parts can cancel.

Before weighted blending, choose a reference real quaternion and flip any dual quaternion whose real part has negative dot product with that reference.

After blending,

$$
\hat q = \sum_i w_i \hat q_i,
$$

normalize the real quaternion magnitude and apply the same scale to the dual part. This is normalized dual-quaternion blending. It represents rigid blending; ordinary dual-quaternion skinning does not directly encode arbitrary nonuniform scale or shear.

## Bind transforms

Skinning still needs the same coordinate contract as matrix skinning:

current bone world transform × inverse bind transform

must produce the deformation from bind-space vertex to current posed space. Dual quaternions change the representation/blending mechanism, not the ownership of coordinate spaces.

## Executable witness

tools/labs/dual_quaternion_skinning_lab.cpp constructs rigid transforms from quaternion + translation, transforms points, checks distance preservation, blends pure translations, and deliberately blends a dual quaternion with its globally negated equivalent to prove antipodal alignment prevents cancellation.

It does not yet prove arbitrary multi-joint skinning, scale/shear, GPU implementation, inverse-bind import or production animation parity.

Related: [[Quaternion rotation and double cover]], [[Quaternion interpolation and shortest paths]], [[Transform hierarchies TRS and shear]].
