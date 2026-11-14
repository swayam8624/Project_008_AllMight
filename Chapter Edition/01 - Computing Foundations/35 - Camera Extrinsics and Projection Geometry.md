# 35 — Camera Extrinsics and Projection Geometry

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field|Next chapter]]

Intrinsics describe how a camera forms image measurements, while extrinsics place its optical frame in the world. The chapter derives camera-center and transform conventions, then extends rays to fisheye and XR-oriented sampling geometry.

**By the end:** Convert between camera-to-world and world-to-camera conventions and derive rays from image measurements for non-pinhole cameras.

**Recall question:** Why is the translation column of a world-to-camera matrix not generally the camera’s world-space position?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 86865–89919; source-span SHA256 48bb391c2f104a753e3e8be7e12a92afce6837509bb0390a1bde75894e3003b8. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Extrinsics place the optical camera in the world

*World-to-camera matrix, graphics/CV frame conversion, pinhole model, ambiguity and Jacobians.*

Intrinsics describe the camera itself. Extrinsics connect its optical frame to the world and make a complete projection possible.

**Camera extrinsics**

Camera extrinsics describe the relationship between world coordinates and the camera's coordinate frame.

A rigid camera pose contains rotation and translation.

The direction of the transformation must be explicit.

In computer vision, a common extrinsic convention maps world coordinates into camera coordinates:

$$
 \boxed{ \mathbf X_C=R_{CW}\mathbf X_W+\mathbf t_{CW} }. 
$$

The subscript $CW$ here means world-to-camera.

### Camera centre versus extrinsic translation

Suppose the camera centre in world coordinates is:

$$
 \mathbf C_W. 
$$

At that point:

$$
 \mathbf X_W=\mathbf C_W. 
$$

Its camera coordinates must be zero:

$$
 \mathbf0=R_{CW}\mathbf C_W+\mathbf t_{CW}. 
$$

Therefore:

$$
 \boxed{ \mathbf t_{CW}=-R_{CW}\mathbf C_W }. 
$$

This is one of the most important extrinsic relationships.

The translation vector in the world-to-camera extrinsic matrix is generally not the camera's world-space position.

### Numerical example · V11.12.03

Suppose the camera is at:

$$
 \mathbf C_W= \begin{bmatrix}3\\0\\0\end{bmatrix} 
$$

with identity world-to-camera rotation.

Then:

$$
 R_{CW}=I. 
$$

The extrinsic translation is:

$$
 \mathbf t_{CW}= -I \begin{bmatrix}3\\0\\0\end{bmatrix} = \begin{bmatrix}-3\\0\\0\end{bmatrix}. 
$$

The world point:

$$
 \mathbf X_W=(4,0,5) 
$$

transforms to:

$$
 \mathbf X_C= (4,0,5)+(-3,0,0). 
$$

Thus:

$$
 \boxed{ \mathbf X_C=(1,0,5) }. 
$$

The point is one unit right of the camera and five units forward.

### Camera-to-world versus world-to-camera

If the camera-to-world pose is:

$$
 T_{WC}= \begin{bmatrix} R_{WC}&\mathbf C_W\\ 0&1 \end{bmatrix}, 
$$

then:

$$
 \boxed{ T_{CW}=T_{WC}^{-1} }. 
$$

For a rigid rotation:

$$
 R_{CW}=R_{WC}^T. 
$$

And:

$$
 \boxed{ \mathbf t_{CW}=-R_{WC}^T\mathbf C_W }. 
$$

Thus a camera pose and an extrinsic matrix encode the same rigid relationship in opposite directions.

**The world-to-camera extrinsic matrix**

The world-to-camera rigid extrinsic matrix is:

$$
 \boxed{ T_{CW}= \begin{bmatrix} R_{CW}&\mathbf t_{CW}\\ 0&1 \end{bmatrix} }. 
$$

For projection calculations, we frequently use only the top three rows:

$$
 \boxed{ [R_{CW}\mid\mathbf t_{CW}] } 
$$

which form a $3\times4$ matrix.

The full ideal pinhole projection is then:

$$
 \boxed{ \lambda\tilde{\mathbf x} = K[R_{CW}\mid\mathbf t_{CW}] \tilde{\mathbf X}_W } 
$$

where:

$$
 \tilde{\mathbf x}= \begin{bmatrix}u\\v\\1\end{bmatrix} 
$$

is a homogeneous image point and:

$$
 \tilde{\mathbf X}_W= \begin{bmatrix}X\\Y\\Z\\1\end{bmatrix} 
$$

is a homogeneous world point.

### The camera projection matrix in computer vision

Define:

$$
 \boxed{ P_{\mathrm{CV}}=K[R\mid t] }. 
$$

This is a $3\times4$ projective camera matrix.

It maps projective 3D world points into projective 2D image points.

In homogeneous notation:

$$
 \boxed{ \tilde{\mathbf x}\sim P_{\mathrm{CV}}\tilde{\mathbf X}_W }. 
$$

The symbol $\sim$ means projective equivalence up to a nonzero scalar.

This matrix has a different purpose from the $4\times4$ GPU clip-space projection matrix derived earlier.

### Example · V11.12.06

Suppose:

$$
 K= \begin{bmatrix} 800&0&960\\ 0&800&540\\ 0&0&1 \end{bmatrix}. 
$$

Let:

$$
 R=I, 
$$

$$
 t=(-1,0,0)^T. 
$$

Then:

$$
 [R\mid t] = \begin{bmatrix} 1&0&0&-1\\ 0&1&0&0\\ 0&0&1&0 \end{bmatrix}. 
$$

Multiply:

$$
 P_{\mathrm{CV}}= K[R\mid t]. 
$$

The result is:

$$
 \boxed{ P_{\mathrm{CV}}= \begin{bmatrix} 800&0&960&-800\\ 0&800&540&0\\ 0&0&1&0 \end{bmatrix} }. 
$$

Consider world point:

$$
 \tilde{\mathbf X}_W=(2,0,5,1)^T. 
$$

Multiply:

$$
 P_{\mathrm{CV}}\tilde{\mathbf X}_W = \begin{bmatrix} 800(2)+960(5)-800\\ 800(0)+540(5)\\ 5 \end{bmatrix}. 
$$

Thus:

$$
 = \begin{bmatrix} 5600\\2700\\5 \end{bmatrix}. 
$$

Divide by the last coordinate:

$$
 \boxed{ (u,v)=(1120,540) }. 
$$

This is consistent with the camera being located one world unit to the right of the origin and observing the point at relative $X=1,Z=5$.

### Coordinate convention mismatch: graphics versus CV

Our graphics camera uses:

$$
 +X\text{ right},\quad+Y\text{ up},\quad-Z\text{ forward}. 
$$

Our CV camera uses:

$$
 +X\text{ right},\quad+Y\text{ down},\quad+Z\text{ forward}. 
$$

A rotation that converts graphics view coordinates to this CV optical frame is:

$$
 \boxed{ S= \begin{bmatrix} 1&0&0\\ 0&-1&0\\ 0&0&-1 \end{bmatrix} }. 
$$

Then:

$$
 \boxed{ \mathbf X_{\mathrm{CV}}=S\mathbf X_{\mathrm{graphicsView}} }. 
$$

For graphics point:

$$
 (1,2,-5), 
$$

the CV coordinate becomes:

$$
 \boxed{(1,-2,5)}. 
$$

The point is right, above the optical axis, and in front.

Because CV image $Y$ increases downward, its pixel $v$ coordinate is less than the principal-point $c_y$, as expected.

Notice:

$$
 \det S=(1)(-1)(-1)=+1. 
$$

This is a proper 180° rotation about the X axis, not an orientation-reversing reflection.

Explicitly applying this convention conversion is much safer than scattering Y and Z sign flips throughout rendering and calibration code.

**The pinhole camera model**

The pinhole camera is the ideal projective model underlying much of computer vision and graphics.

A three-dimensional point projects onto a two-dimensional image plane along a line passing through the camera centre.

There is no lens aperture, defocus blur, or optical distortion in the ideal model.

### Complete projection equation

Given world point:

$$
 \mathbf X_W, 
$$

first transform into camera coordinates:

$$
 \boxed{ \mathbf X_C=R\mathbf X_W+\mathbf t }. 
$$

Write:

$$
 \mathbf X_C=(X_C,Y_C,Z_C). 
$$

Assume:

$$
 Z_C>0. 
$$

Normalize:

$$
 \boxed{ x=\frac{X_C}{Z_C}, \qquad y=\frac{Y_C}{Z_C} }. 
$$

Apply intrinsics:

$$
 \boxed{ u=f_xx+sy+c_x } 
$$

and:

$$
 \boxed{ v=f_yy+c_y }. 
$$

Together:

$$
 \boxed{ \tilde{\mathbf x} \sim K[R\mid t]\tilde{\mathbf X}_W }. 
$$

### Why a single image cannot determine unique 3D depth

Suppose a pixel corresponds to normalized coordinates:

$$
 (x,y). 
$$

All points along the camera ray:

$$
 \boxed{ \mathbf X_C(\lambda)= \lambda \begin{bmatrix} x\\y\\1 \end{bmatrix} } 
$$

for:

$$
 \lambda>0 
$$

project to the same ideal pixel.

Indeed:

$$
 \frac{\lambda x}{\lambda}=x, 
$$

$$
 \frac{\lambda y}{\lambda}=y. 
$$

The scale cancels.

Therefore one ideal monocular pixel does not identify a unique 3D point.

It identifies a ray.

Additional information is required to determine depth, such as stereo correspondence, known geometry, active depth sensing, motion parallax, a depth prior, or other constraints.

### Projective scale ambiguity

A similar ambiguity appears in monocular reconstruction.

If all scene points and camera translations are scaled by the same positive factor under compatible conditions, the projected pixel positions can remain unchanged.

For example, doubling the depth and transverse camera-relative coordinates preserves normalized ratios:

$$
 \frac{2X}{2Z}=\frac XZ. 
$$

Thus image observations alone may not determine absolute metric scale.

Known object dimensions, calibrated baselines, depth measurements, or other physical information can resolve this ambiguity.

This is a central issue in structure-from-motion and monocular SLAM.

### Reprojection error

Suppose an observed image point is:

$$
 \mathbf u_i^{\mathrm{obs}} 
$$

and the camera model predicts:

$$
 \hat{\mathbf u}_i= \pi(K,R,t,\mathbf X_i). 
$$

Define the reprojection residual:

$$
 \boxed{ \mathbf r_i= \hat{\mathbf u}_i-\mathbf u_i^{\mathrm{obs}} }. 
$$

A common calibration objective is:

$$
 \boxed{ E= \sum_i \\|\mathbf r_i\\|_2^2 }. 
$$

This connects camera geometry directly to least-squares estimation and robust optimization.

For a complete calibration problem, we may optimize intrinsics, distortion parameters, camera poses, and sometimes 3D point positions.

The optimization can be nonlinear and sensitive to parameter initialization, outliers, and gauge freedoms.

### Projection Jacobian

Let:

$$
 u=f_x\frac XZ+c_x, 
$$

$$
 v=f_y\frac YZ+c_y 
$$

for zero skew.

The Jacobian with respect to camera-space point coordinates is:

$$
 \boxed{ J_\pi= \begin{bmatrix} \dfrac{f_x}{Z}&0&-\dfrac{f_xX}{Z^2}\\ 0&\dfrac{f_y}{Z}&-\dfrac{f_yY}{Z^2} \end{bmatrix} }. 
$$

This follows by partial differentiation.

For example:

$$
 \frac{\partial u}{\partial X}=\frac{f_x}{Z}, 
$$

$$
 \frac{\partial u}{\partial Z}=-\frac{f_xX}{Z^2}. 
$$

The Jacobian maps small 3D camera-space perturbations into image-space pixel perturbations:

$$
 \boxed{ \Delta\mathbf u\approx J_\pi\Delta\mathbf X_C }. 
$$

If the 3D point has covariance:

$$
 \Sigma_X, 
$$

then first-order image covariance is:

$$
 \boxed{ \Sigma_u\approx J_\pi\Sigma_XJ_\pi^T }. 
$$

This is a direct application of probability and covariance theory.

It is especially relevant to uncertainty-aware splatting, camera calibration, and reconstruction diagnostics.


### Figure C11-8 — Residual count does not guarantee observable motion

![[Visual Recall/Math Foundations/C11-8.png]]

Toy Lucas–Kanade information matrices H=sum(∇I∇Iᵀ): repeated horizontal gradients give eigenvalues (0,3); diverse gradients give (1,3). Small eigenvalues expose weak directions. Camera-calibration gauge freedoms require a larger Jacobian model and explicit constraints.


<!-- end V11.12 -->


## distortion is a forward map; undistortion solves its inverse

The ideal pinhole image is not necessarily where a real lens sends a ray. A distortion model maps ideal normalized coordinates $(x,y)$ to measured coordinates $(x_d,y_d)$. For a common radial model, let $r^2=x^2+y^2$ and $L=1+k_1r^2+k_2r^4+k_3r^6$; then $x_d=xL$ and $y_d=yL$, with optional tangential terms. Coefficients are tied to the declared model and coordinate units; they are not universal lens facts.

The forward direction is easy: project the ray ideally, then apply distortion. Undistortion asks for the ideal point whose forward-distorted value matches the measured one. Except for special cases, this is a nonlinear root-finding problem. Iterate from a reasonable initial guess, check the remaining forward residual, and reject nonfinite or nonconverged results. At extreme image radii, a fitted polynomial may cease to be one-to-one; an inverse may be ambiguous or nonexistent. A calibration that fits the center but not the edges is a model/coverage problem, not evidence that an inverse routine should continue indefinitely.

<!-- begin V11.13 -->
## Real lenses bend the pinhole image

*Distortion, radial/tangential models, invertibility, undistortion and uncertainty.*

The ideal pinhole model leaves systematic image residuals when real optics bend rays. A nonlinear distortion map completes the forward camera model.

**Lens distortion: why the pinhole model is insufficient**

The ideal pinhole model assumes that a 3D ray maps to a pixel according to a simple projective relationship.

Real lenses do not always obey this mapping exactly.

Optical elements bend rays, and practical lens assemblies may exhibit geometric deviations from an ideal rectilinear projection.

A distortion model describes the difference between ideal normalized pinhole coordinates and measured distorted image coordinates.

### Two different coordinate pairs

Let:

$$
 (x,y) 
$$

be the ideal undistorted normalized camera coordinates.

Let:

$$
 (x_d,y_d) 
$$

be distorted normalized coordinates produced by the lens model.

The distortion transformation is:

$$
 \boxed{ D: (x,y)\mapsto(x_d,y_d) }. 
$$

After distortion, the intrinsic matrix converts normalized coordinates to pixels:

$$
 \boxed{ u=f_xx_d+sy_d+c_x } 
$$

and:

$$
 \boxed{ v=f_yy_d+c_y }. 
$$

Therefore the correct pipeline is:

$$
 \boxed{ \mathbf X_W \rightarrow \mathbf X_C \rightarrow (x,y) \rightarrow (x_d,y_d) \rightarrow (u,v) }. 
$$

The order matters.

Ordinary radial/tangential distortion coefficients are typically defined in normalized camera coordinates, not directly in raw pixel coordinates.

### Why distortion must be modeled separately from $K$

The intrinsic matrix $K$ represents a linear homogeneous transformation between normalized ideal camera coordinates and pixel coordinates.

After the perspective division, it produces an affine mapping in the two image coordinates.

Radial and tangential distortion are nonlinear functions of image position.

A single fixed $3\times3$ matrix cannot represent arbitrary nonlinear lens distortion.

Thus a calibrated camera combines a projective mapping with a nonlinear distortion model.

### Different lens families require different models

A standard Brown–Conrady-style radial/tangential model is useful for many ordinary lenses.

But extreme wide-angle and fisheye lenses often require different projection functions and distortion parameterizations.

A fisheye camera may map ray angle to image radius according to equidistant, equisolid-angle, stereographic, or other lens projection models.

One should not assume that a high-degree ordinary radial polynomial is automatically a good model for every fisheye lens.

Fisheye projection requires a separate model, treated in the later panoramic-camera chapters.

Here, the principal model is the common Brown–Conrady radial/tangential formulation used in OpenCV-compatible calibration workflows.&#x20;


### Distortion and calibration residuals

Suppose a camera calibration has a low average reprojection error near the image centre but large errors near the edges.

That pattern may indicate that the chosen distortion model is insufficient, parameters were estimated poorly, or the calibration dataset inadequately sampled the outer image field.

A single global RMS reprojection error can conceal such spatial structure.

A useful calibration report should examine residual vectors as a function of image position and radius, not only their aggregate mean.

This is a direct application of robust statistics and error analysis.

**Radial lens distortion**

Radial distortion depends primarily on distance from the optical centre in normalized image coordinates.

Define:

$$
 \boxed{ r^2=x^2+y^2 }. 
$$

A common polynomial radial factor is:

$$
 \boxed{ L(r)= 1+k_1r^2+k_2r^4+k_3r^6 }. 
$$

The radially distorted coordinates are:

$$
 \boxed{ x_d=xL(r) } 
$$

and:

$$
 \boxed{ y_d=yL(r) }. 
$$

The coefficients $k_1,k_2,k_3$ determine how magnification changes with distance from the image centre.

### Why powers of $r^2$ appear

An ideal rotationally symmetric distortion field should not depend on the angular direction around the optical axis.

Its radial scaling depends on the radius:

$$
 r=\sqrt{x^2+y^2}. 
$$

For a smooth radial model symmetric under coordinate sign reversal, a polynomial in $r^2$ is a natural parameterization.

Thus the distortion scale can be modeled as:

$$
 1+k_1r^2+k_2r^4+k_3r^6+\cdots. 
$$

The constant term is one so the mapping has unit scale at the optical centre in this normalized formulation.

### Barrel distortion

In a common barrel-distortion regime, magnification decreases as distance from the image centre increases.

Points at large radius are mapped inward compared with the ideal pinhole model.

For a simple model with:

$$
 k_1<0 
$$

and sufficiently small higher-order contributions:

$$
 L(r)<1 
$$

away from the centre.

Thus:

$$
 r_d=rL(r)\<r. 
$$

Straight lines away from the image centre can appear bowed outward.

The sign of $k_1$ alone is not sufficient to classify every higher-order model globally; the overall radial mapping determines actual behavior.

### Pincushion distortion

In a common pincushion-distortion regime, magnification increases with radius.

For a simple positive $k_1$ model:

$$
 L(r)>1 
$$

away from the centre.

Then:

$$
 r_d>r. 
$$

Straight lines can appear pulled inward toward the centre.

Again, the total radial polynomial matters, especially when higher-order coefficients are substantial.

### Numerical example · V11.13.09

Take:

$$
 x=0.3,\qquad y=0.4. 
$$

Then:

$$
 r^2=0.3^2+0.4^2 =0.09+0.16 =\boxed{0.25}. 
$$

Let:

$$
 k_1=-0.2, 
$$

$$
 k_2=0.04, 
$$

$$
 k_3=0. 
$$

Then:

$$
 L(r)=1-0.2(0.25)+0.04(0.25^2). 
$$

Compute:

$$
 L(r)=1-0.05+0.0025 =\boxed{0.9525}. 
$$

Therefore:

$$
 x_d=0.3(0.9525) =\boxed{0.28575}. 
$$

And:

$$
 y_d=0.4(0.9525) =\boxed{0.381}. 
$$

The ideal point:

$$
 (0.3,0.4) 
$$

moves inward to:

$$
 \boxed{(0.28575,0.381)}. 
$$

The distortion preserves the point's angular direction in this radial-only model.

### Why radial distortion can become non-invertible

The distorted radial distance is:

$$
 \boxed{ r_d=rL(r) }. 
$$

For a unique radial inverse across the valid domain, we need the radial mapping to be suitably one-to-one.

A sufficient local condition for positive radial orientation is:

$$
 \boxed{ \frac{dr_d}{dr}>0 } 
$$

throughout the modeled domain.

Differentiate:

$$
 r_d=r(1+k_1r^2+k_2r^4+k_3r^6). 
$$

Thus:

$$
 \frac{dr_d}{dr} = 1+k_1r^2+k_2r^4+k_3r^6 +r(2k_1r+4k_2r^3+6k_3r^5). 
$$

Combine:

$$
 \boxed{ \frac{dr_d}{dr} = 1+3k_1r^2+5k_2r^4+7k_3r^6 }. 
$$

If this derivative becomes zero, the radial mapping can locally fold or lose invertibility.

A calibration optimizer may produce coefficients that fit observed points but behave pathologically outside the sampled radius range.

OpenCV's calibration documentation explicitly discusses the need for physically sensible and invertible radial mappings and cautions that unconstrained optimization may produce nonmonotonic distortion functions.&#x20;


### Rational radial distortion

More complex OpenCV-compatible models can use a rational radial factor:

$$
 \boxed{ L(r)= \frac{ 1+k_1r^2+k_2r^4+k_3r^6 }{ 1+k_4r^2+k_5r^4+k_6r^6 } }. 
$$

This adds flexibility but also introduces possible denominator singularities.

The model must be validated over the image's actual normalized radius domain.

More coefficients do not automatically mean a better-calibrated camera.

Overparameterization can reduce training reprojection error while making predictions unstable.

**Tangential lens distortion**

Tangential distortion models asymmetric displacement associated with decentring or alignment imperfections in the optical system.

Unlike purely radial distortion, it does not simply multiply both image coordinates by the same radial factor.

The common two-parameter model uses:

$$
 p_1,\quad p_2. 
$$

Its displacement terms are:

$$
 \boxed{ \Delta x_t= 2p_1xy+p_2(r^2+2x^2) } 
$$

and:

$$
 \boxed{ \Delta y_t= p_1(r^2+2y^2)+2p_2xy }. 
$$

### Full radial-plus-tangential model

Define:

$$
 L(r)=1+k_1r^2+k_2r^4+k_3r^6. 
$$

Then:

$$
 \boxed{ x_d= xL(r)+2p_1xy+p_2(r^2+2x^2) } 
$$

and:

$$
 \boxed{ y_d= yL(r)+p_1(r^2+2y^2)+2p_2xy }. 
$$

This is a standard five-coefficient Brown–Conrady-style camera distortion formulation.

A common OpenCV coefficient order for the five-coefficient case is:

$$
 \boxed{ (k_1,k_2,p_1,p_2,k_3) }. 
$$

The order of parameters in a software file must be treated as an explicit schema contract.

Do not assume every calibration library stores coefficients in the same order.

### Numerical example with tangential distortion only

Take:

$$
 x=0.3,\qquad y=0.4. 
$$

Then:

$$
 r^2=0.25. 
$$

Let:

$$
 p_1=0.01,\qquad p_2=-0.005, 
$$

with radial coefficients zero.

The horizontal tangential displacement is:

$$
 \Delta x_t= 2(0.01)(0.3)(0.4) +(-0.005)(0.25+2(0.3)^2). 
$$

First term:

$$
 2(0.01)(0.12)=0.0024. 
$$

Second term:

$$
 -0.005(0.25+0.18) =-0.005(0.43) =-0.00215. 
$$

Therefore:

$$
 \boxed{ \Delta x_t=0.00025 }. 
$$

Thus:

$$
 \boxed{x_d=0.30025}. 
$$

For Y:

$$
 \Delta y_t= 0.01(0.25+2(0.4)^2) +2(-0.005)(0.3)(0.4). 
$$

First term:

$$
 0.01(0.25+0.32)=0.0057. 
$$

Second term:

$$
 -0.01(0.12)=-0.0012. 
$$

Thus:

$$
 \boxed{ \Delta y_t=0.0045 }. 
$$

Therefore:

$$
 \boxed{y_d=0.4045}. 
$$

The point moves asymmetrically:

$$
 (0.3,0.4)\mapsto(0.30025,0.4045). 
$$

It does not simply move radially inward or outward.

### Units and interpretation

The normalized coordinates $x,y$ are dimensionless ratios after division by depth.

Consequently, the distortion coefficients are defined relative to that normalized coordinate system.

Their numerical magnitudes should not be interpreted as millimetre displacements or pixel displacements by themselves.

Pixel distortion depends on intrinsics:

$$
 \Delta u\approx f_x\Delta x_d+s\Delta y_d, 
$$

$$
 \Delta v\approx f_y\Delta y_d. 
$$

A small normalized distortion can correspond to several pixels when focal scales are large.

### Complete projection pipeline with distortion

A real calibrated camera forward model is:

$$
 \boxed{ \mathbf X_C=R\mathbf X_W+\mathbf t } 
$$

followed by:

$$
 \boxed{ x=X_C/Z_C,\qquad y=Y_C/Z_C } 
$$

then:

$$
 \boxed{ (x_d,y_d)=D(x,y) } 
$$

and finally:

$$
 \boxed{ \begin{bmatrix}u\\v\\1\end{bmatrix} \sim K \begin{bmatrix}x_d\\y_d\\1\end{bmatrix} }. 
$$

This full mapping is nonlinear due to distortion, even though the rigid transform and intrinsic matrix are linear in homogeneous coordinates.

**Undistortion**

Undistortion is the inverse problem.

We observe a pixel coordinate affected by lens distortion.

We want to recover the ideal normalized pinhole coordinate, or construct an undistorted image.

The distortion model provides a forward mapping:

$$
 \boxed{ \mathbf q_d=D(\mathbf q) } 
$$

where:

$$
 \mathbf q= \begin{bmatrix}x\\y\end{bmatrix}. 
$$

Given $\mathbf q_d$, we seek:

$$
 \boxed{ D(\mathbf q)=\mathbf q_d }. 
$$

This is a nonlinear root-finding problem.

Newton iteration, Jacobians, and numerical-stability analysis directly apply.

### Convert observed pixels to distorted normalized coordinates

Given observed pixel:

$$
 (u_d,v_d), 
$$

first remove intrinsics.

For zero skew:

$$
 \boxed{ x_d=\frac{u_d-c_x}{f_x} } 
$$

and:

$$
 \boxed{ y_d=\frac{v_d-c_y}{f_y} }. 
$$

If skew is nonzero, use the inverse intrinsic matrix instead of independently dividing both coordinates.

Now solve:

$$
 D(x,y)=(x_d,y_d). 
$$

### Fixed-point inversion

A simple approach starts with:

$$
 \boxed{ (x_0,y_0)=(x_d,y_d) }. 
$$

If distortion is weak, we can iteratively subtract the estimated distortion displacement.

For a pure radial model:

$$
 x_d=xL(r), 
$$

$$
 y_d=yL(r). 
$$

One possible iteration is:

$$
 \boxed{ x_{k+1}=\frac{x_d}{L(r_k)} } 
$$

and:

$$
 \boxed{ y_{k+1}=\frac{y_d}{L(r_k)} } 
$$

where:

$$
 r_k^2=x_k^2+y_k^2. 
$$

This can work under suitable conditions.

But convergence is not universal.

A strongly distorted or noninvertible mapping can cause divergence or convergence to an unwanted solution.

### Newton inversion

Define the residual function:

$$
 \boxed{ \mathbf F(\mathbf q)= D(\mathbf q)-\mathbf q_d }. 
$$

We seek:

$$
 \mathbf F(\mathbf q)=\mathbf0. 
$$

At iteration $k$, linearize:

$$
 \mathbf F(\mathbf q_k+\Delta\mathbf q) \approx \mathbf F(\mathbf q_k)+J_D(\mathbf q_k)\Delta\mathbf q. 
$$

Set the linear model to zero:

$$
 J_D(\mathbf q_k)\Delta\mathbf q = -\mathbf F(\mathbf q_k). 
$$

Thus:

$$
 \boxed{ \Delta\mathbf q = -J_D(\mathbf q_k)^{-1} \mathbf F(\mathbf q_k) }. 
$$

The update is:

$$
 \boxed{ \mathbf q_{k+1} = \mathbf q_k- J_D(\mathbf q_k)^{-1} [D(\mathbf q_k)-\mathbf q_d] }. 
$$

In practice, solve the $2\times2$ linear system rather than explicitly computing a general matrix inverse.

### Derive the distortion Jacobian

Let:

$$
 r^2=x^2+y^2. 
$$

Define:

$$
 L=1+k_1r^2+k_2r^4+k_3r^6. 
$$

It is convenient to treat $L$ as a function of:

$$
 q=r^2. 
$$

Then:

$$
 L(q)=1+k_1q+k_2q^2+k_3q^3. 
$$

Differentiate:

$$
 \boxed{ L_q=\frac{dL}{dq} = k_1+2k_2q+3k_3q^2 }. 
$$

Because:

$$
 q=x^2+y^2, 
$$

we have:

$$
 \frac{\partial q}{\partial x}=2x, 
$$

$$
 \frac{\partial q}{\partial y}=2y. 
$$

Therefore:

$$
 \boxed{ \frac{\partial L}{\partial x}=2xL_q } 
$$

and:

$$
 \boxed{ \frac{\partial L}{\partial y}=2yL_q }. 
$$

Now recall:

$$
 x_d=xL+2p_1xy+p_2(r^2+2x^2). 
$$

Differentiate with respect to $x$:

$$
 \frac{\partial x_d}{\partial x} = L+x\frac{\partial L}{\partial x} +2p_1y +p_2(2x+4x). 
$$

Thus:

$$
 \boxed{ J_{00}= L+2x^2L_q+2p_1y+6p_2x }. 
$$

Differentiate $x_d$ with respect to $y$:

$$
 \frac{\partial x_d}{\partial y} = x\frac{\partial L}{\partial y} +2p_1x +2p_2y. 
$$

Therefore:

$$
 \boxed{ J_{01}= 2xyL_q+2p_1x+2p_2y }. 
$$

For the second distorted coordinate:

$$
 y_d=yL+p_1(r^2+2y^2)+2p_2xy. 
$$

Differentiate with respect to $x$:

$$
 \boxed{ J_{10}= 2xyL_q+2p_1x+2p_2y }. 
$$

Differentiate with respect to $y$:

$$
 \boxed{ J_{11}= L+2y^2L_q+6p_1y+2p_2x }. 
$$

Thus the full Jacobian is:

$$
 \boxed{ J_D= \begin{bmatrix} L+2x^2L_q+2p_1y+6p_2x& 2xyL_q+2p_1x+2p_2y\\ 2xyL_q+2p_1x+2p_2y& L+2y^2L_q+6p_1y+2p_2x \end{bmatrix} }. 
$$

This analytic Jacobian is implemented in the reference C++23 laboratory.

### Solve the Newton system

Let the current residual be:

$$
 \mathbf e= \begin{bmatrix} e_x\\e_y \end{bmatrix} = D(\mathbf q)-\mathbf q_d. 
$$

Write:

$$
 J_D= \begin{bmatrix} a&b\\c&d \end{bmatrix}. 
$$

The determinant is:

$$
 \boxed{ \Delta=ad-bc }. 
$$

When:

$$
 \Delta\ne0, 
$$

the inverse is:

$$
 J_D^{-1} = \frac1\Delta \begin{bmatrix} d&-b\\ -c&a \end{bmatrix}. 
$$

Therefore the correction is:

$$
 \boxed{ \delta_x= \frac{de_x-be_y}{\Delta} } 
$$

and:

$$
 \boxed{ \delta_y= \frac{-ce_x+ae_y}{\Delta} }. 
$$

Update:

$$
 \boxed{ x_{k+1}=x_k-\delta_x } 
$$

and:

$$
 \boxed{ y_{k+1}=y_k-\delta_y }. 
$$

### What can go wrong?

The Jacobian can become singular or ill-conditioned.

The distortion map may not be one-to-one over the requested image region.

Newton may converge to an unintended solution or diverge from a poor initial guess.

Extreme coefficients may cause overflow or generate nonfinite intermediate values.

A production solver should use convergence criteria, iteration limits, residual diagnostics, step limits or damping where appropriate, and explicit failure reporting.

It should also validate that the recovered point maps back to the observed point within an acceptable tolerance.

### Undistorting an image is not just undistorting its corner points

Suppose we want to create a regular undistorted image.

A common robust approach uses inverse image resampling.

For each output pixel in the desired undistorted image:

1. Convert its output pixel coordinate into an ideal normalized direction.
2. Apply the forward distortion mapping to find where that ideal ray appears in the original distorted image.
3. Convert the distorted normalized coordinate to the original input pixel coordinate.
4. Sample the input image using appropriate interpolation.

This is called inverse or backward image mapping because output pixels query source-image positions.

It helps avoid holes that occur when source pixels are pushed forward into a new image grid.

The process generally requires image filtering and a choice of output camera intrinsics or crop region.

### Distortion and reprojection uncertainty

Suppose the undistorted normalized point has uncertainty covariance:

$$
 \Sigma_q. 
$$

The distorted point is:

$$
 \mathbf q_d=D(\mathbf q). 
$$

For small uncertainty, first-order covariance propagation gives:

$$
 \boxed{ \Sigma_d\approx J_D\Sigma_qJ_D^T }. 
$$

After mapping to pixels with an intrinsic linear part $K_{2\times2}$:

$$
 \boxed{ \Sigma_{\mathrm{pixels}} \approx K_{2\times2} J_D\Sigma_qJ_D^T K_{2\times2}^T }. 
$$

This allows camera calibration uncertainty to be connected to image-space observation uncertainty.

It is directly relevant to uncertainty-aware reconstruction and Gaussian projection, although a Gaussian projected through a strongly nonlinear distortion mapping is not generally exactly Gaussian.

*Camera-tail continuation for the connected camera narrative covering Parts 1001–1100. This file supplies the final source range, Parts 1078–1100, for integration into [[Chapter Edition/01 - Computing Foundations/32 - Geometry Becomes an Image and an Inverse Problem#A coordinate is a claim about a frame|11 - From Coordinate Frames to Camera Images - Projective Geometry and Cameras]]. The supplied explanations, derivations, worked examples, and fenced code bodies are retained under concept-led chapters. Repository inspections and claimed executions are marked as source-reported, not independently verified here.*

**Coverage:** Parts 1078–1083 (depth interpretation, depth linearization, position reconstruction, and picking rays); 1084–1088 (off-axis projection, stereo, IPD, and parallax); 1089–1091 (cubemap, panoramic, and fisheye cameras); 1092–1098 (exposure, aperture, shutter, ISO, depth of field, and thin-lens rays); 1099–1100 (camera API design and end-to-end debugging).



### Figure C11-6 — Distortion is a map with a domain, not just a coefficient list

![[Visual Recall/Math Foundations/C11-6.png]]

Normalized Brown-style radial example xd=x(1+k₁r²), yd=y(1+k₁r²). This displayed domain has positive radial derivative 1+3k₁r², so the local radial map remains monotone. Larger radii can fold; numerical undistortion requires domain and convergence checks.


<!-- end V11.13 -->

<!-- begin V11.14 -->
## Recover geometry from depth and construct screen-space rays


Consider a camera at the origin looking down its local negative-Z axis. A point in view space is:

$$ \mathbf p_v= \begin{bmatrix} x\\y\\z \end{bmatrix}, \qquad z<0. $$

There are two different quantities commonly called depth.

The positive axial or view depth is:

$$ \boxed{d=-z}. $$

The Euclidean distance from the camera centre to the point is:

$$ \boxed{ r=\|\mathbf p_v\|_2=\sqrt{x^2+y^2+z^2}. } $$

These quantities agree only for points on the camera's optical axis.

### Derive the geometric relationship

Let $\theta$ be the angle between the ray from the camera to the point and the camera's forward direction $(0,0,-1)$. The normalized ray direction is:

$$ \hat{\mathbf q}=\frac{\mathbf p_v}{r}. $$

Its dot product with camera forward is:

$$ \cos\theta = \hat{\mathbf q}\cdot(0,0,-1) = -\frac zr. $$

Since $d=-z$:

$$ \boxed{d=r\cos\theta}, \qquad \boxed{r=\frac d{\cos\theta}}. $$

At the centre of the image, $\theta=0$, so $d=r$. Away from the centre, $\cos\theta<1$, so Euclidean range exceeds axial depth.

### Numerical example · V11.14.03

Take:

$$ \mathbf p_v=(3,4,-12). $$

Then the axial depth is:

$$ \boxed{d=12}. $$

But the range is:

$$ r=\sqrt{3^2+4^2+12^2} =\sqrt{169} =\boxed{13}. $$

The point is 12 units along the forward camera axis and 13 units away from the actual camera centre.

This difference matters in almost every reconstruction pipeline. Conventional perspective depth-buffer mathematics uses axial depth, whereas a range sensor may return distance along a ray. A LiDAR dataset may describe range, a stereo reconstruction algorithm may estimate camera-axis depth, and a depth image format may encode something else entirely.

If you feed Euclidean range into a formula expecting axial depth, reconstructed geometry expands incorrectly away from the optical axis.

### Converting a range image to axial depth

Suppose the normalized camera ray is:

$$ \hat{\mathbf q}= \begin{bmatrix}q_x\\q_y\\q_z\end{bmatrix}, $$

with $q_z<0$ in our graphics convention.

A point at range $r$ is:

$$ \mathbf p_v=r\hat{\mathbf q}. $$

Thus:

$$ \boxed{d=-rq_z}. $$

Conversely:

$$ \boxed{r=-\frac d{q_z}}. $$

The conversion depends on the actual ray direction. It is not a fixed scalar applied to an entire depth image.

For calibrated images, derive that direction through the camera's intrinsics and, when applicable, lens-undistortion model.


A conventional perspective depth buffer normally stores a nonlinear function of axial depth. This follows from the homogeneous projection and perspective divide derived earlier in this sequence.

We can derive a single general formula that handles finite near/far planes, reversed-Z, and different depth conventions.

### Start from the perspective matrix

For a right-handed perspective camera, the clip-space depth rows can be written:

$$ z_c=Az_v+B, \qquad w_c=-z_v. $$

Since $z_v=-d$, with $d>0$:

$$ z_c=-Ad+B, \qquad w_c=d. $$

After division:

$$ \boxed{ q=\frac{z_c}{w_c}=-A+\frac Bd } $$

where $q$ is NDC depth, not necessarily the raw framebuffer depth value.

Rearrange:

$$ q+A=\frac Bd. $$

Therefore:

$$ \boxed{ d=\frac B{q+A}. } $$

This is the general perspective axial-depth recovery equation for the given matrix form.

### Conventional finite zero-to-one depth

For near distance $n$ and far distance $f$, the standard matrix coefficients are:

$$ A=-\frac f{f-n}, \qquad B=-\frac{fn}{f-n}. $$

Substitute:

$$ d= \frac{-fn/(f-n)} {q-f/(f-n)}. $$

Multiply numerator and denominator by $f-n$:

$$ d=\frac{-fn}{q(f-n)-f}. $$

Therefore:

$$ \boxed{ d_{\mathrm{standard}}(q) = \frac{fn}{f-q(f-n)}. } $$

Check the boundaries. At $q=0$:

$$ d=\frac{fn}{f}=n. $$

At $q=1$:

$$ d=\frac{fn}{f-(f-n)}=f. $$

Both are correct.

### Finite reversed-Z

For reversed zero-to-one perspective projection:

$$ A=\frac n{f-n}, \qquad B=\frac{nf}{f-n}. $$

Substitute into $d=B/(q+A)$:

$$ \boxed{ d_{\mathrm{reversed}}(q) = \frac{nf}{n+q(f-n)}. } $$

At $q=1$, we obtain $n$. At $q=0$, we obtain $f$.

### Infinite reversed-Z

As $f\to\infty$, the projection coefficients become:

$$ A=0,\qquad B=n. $$

Thus:

$$ q=\frac nd. $$

Invert:

$$ \boxed{ d=\frac nq. } $$

At $q=0$, the ideal mathematical depth is infinite, not a finite reconstructed position. Shader code must handle this boundary before dividing.

### Traditional OpenGL NDC depth

Traditional OpenGL maps the near plane to $q=-1$ and far plane to $q=1$. Its right-handed perspective coefficients are:

$$ A=-\frac{f+n}{f-n}, \qquad B=-\frac{2fn}{f-n}. $$

Then:

$$ d= \frac{-2fn} {q(f-n)-(f+n)}. $$

Therefore:

$$ \boxed{ d_{\mathrm{GL}}(q)= \frac{2fn}{f+n-q(f-n)}. } $$

If a depth texture contains ordinary OpenGL window depth $z_b\in[0,1]$, and the default depth-range mapping is used:

$$ q=2z_b-1. $$

You must first convert the stored value into the NDC convention used by the projection formula.

### A generalized endpoint derivation

Let the desired near and far NDC values be $q_n$ and $q_f$. Instead of memorizing separate formulas, write:

$$ q(d)=C+\frac Dd. $$

The endpoint conditions give:

$$ q_n=C+\frac Dn, \qquad q_f=C+\frac Df. $$

Subtract:

$$ q_n-q_f=D\left(\frac1n-\frac1f\right). $$

Thus:

$$ \boxed{ D=\frac{nf(q_n-q_f)}{f-n}. } $$

Then:

$$ \boxed{ C=\frac{fq_f-nq_n}{f-n}. } $$

Therefore:

$$ \boxed{ d(q)=\frac D{q-C}. } $$

This is a convenient API-neutral formulation. It makes the chosen depth endpoints explicit rather than concealing them inside a projection helper.

### Numerical example · V11.14.11

Use:

$$ n=0.1,\qquad f=1000, $$

and a point at axial depth:

$$ d=10. $$

For standard zero-to-one projection:

$$ q=\frac f{f-n}-\frac{fn}{(f-n)d}. $$

That gives approximately:

$$ \boxed{q=0.99009901}. $$

For finite reversed-Z:

$$ \boxed{q\approx0.00990099}. $$

For infinite reversed-Z:

$$ \boxed{q=0.01}. $$

All three encode the same physical camera-axis depth of ten.

The stored value is meaningful only together with the projection convention.

### Numerical stability matters when linearizing

The formula:

$$ d=\frac{fn}{f-q(f-n)} $$

can suffer precision loss near the far plane if its denominator is obtained by subtracting nearly equal floating-point values.

Use appropriate intermediate precision when needed, and avoid reconstructing distant positions from a low-precision conventional depth buffer if a more stable reversed-Z path is available.

Nathan Reed's [Visualizing Depth Precision](https://developer.nvidia.com/blog/visualizing-depth-precision/?utm_source=chatgpt.com) provides a particularly useful analysis of why hardware depth has a reciprocal-depth form and why floating-point reversed-Z is effective.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://developer.nvidia.com\&sz=32)

NVIDIA Technical Blog




A depth value gives only one scalar. To reconstruct a 3D point, we must also know which image location or camera ray produced it.

For a conventional perspective camera, a pixel identifies a ray, and the depth identifies a point along that ray.

We will derive two methods: an analytical FOV-based reconstruction and a general inverse-projection reconstruction.

### Method A: reconstruct from NDC and FOV

Let normalized device coordinates be:

$$ (x_n,y_n) $$

with $+Y$ upward. Assume a symmetric right-handed perspective camera with vertical FOV $\theta_y$ and aspect ratio:

$$ a=\frac WH. $$

Define:

$$ s_y=\frac1{\tan(\theta_y/2)}, \qquad s_x=\frac{s_y}{a}. $$

The perspective projection equations are:

$$ x_n=\frac{s_xx_v}{d}, \qquad y_n=\frac{s_yy_v}{d}, $$

where:

$$ d=-z_v. $$

Solve for view-space coordinates:

$$ x_v=\frac{x_nd}{s_x}, $$

$$ y_v=\frac{y_nd}{s_y}, $$

$$ z_v=-d. $$

Substitute $1/s_y=\tan(\theta_y/2)$ and $1/s_x=a\tan(\theta_y/2)$:

$$ \boxed{ \mathbf p_v= \begin{bmatrix} a\,d\,\tan(\theta_y/2)\,x_n\\ d\,\tan(\theta_y/2)\,y_n\\ -d \end{bmatrix}. } $$

This formula is extremely useful for screen-space ambient occlusion, screen-space reflections, depth-based post-processing, volumetric effects, and ray generation.

### Numerical example · V11.14.14

Suppose:

$$ \theta_y=90^\circ,\qquad a=1, $$

$$ (x_n,y_n)=(0.5,-0.25), $$

and linearized axial depth:

$$ d=8. $$

Because:

$$ \tan45^\circ=1, $$

we obtain:

$$ x_v=8(0.5)=4, $$

$$ y_v=8(-0.25)=-2, $$

$$ z_v=-8. $$

Therefore:

$$ \boxed{\mathbf p_v=(4,-2,-8)}. $$

This point lies along the camera ray through that NDC coordinate at axial depth eight.

### Method B: inverse projection

The analytical formula assumes a symmetric perspective camera. It must be modified for off-axis projections, nonstandard projection rows, or orthographic cameras.

A more general approach uses the inverse projection matrix.

Let:

$$ P $$

be the actual projection used to produce the depth texture.

Form an NDC homogeneous vector:

$$ \tilde{\mathbf q}= \begin{bmatrix} x_n\\y_n\\q\\1 \end{bmatrix}. $$

Compute:

$$ \boxed{ \mathbf h_v=P^{-1}\tilde{\mathbf q}. } $$

Because the result is homogeneous, recover the finite view-space point by dividing:

$$ \boxed{ \mathbf p_v=\frac{\mathbf h_{v,xyz}}{h_{v,w}}. } $$

This works whenever the matrix is invertible, the depth coordinate matches its clip convention, and the reconstructed homogeneous point has nonzero $w$.

### Why this works

The original point satisfies:

$$ \mathbf p_c=P\tilde{\mathbf p}_v. $$

After dividing by $w_c$, it becomes:

$$ \tilde{\mathbf q} = \frac{\mathbf p_c}{w_c}. $$

Therefore:

$$ P^{-1}\tilde{\mathbf q} = \frac{\tilde{\mathbf p}_v}{w_c}. $$

The inverse result differs from the original point by a homogeneous scale, but dividing by its final coordinate removes that scale.

### Important implementation details

The inverse method reconstructs exactly according to the matrix supplied, including off-centre terms. But it does not automatically undo nonlinear lens distortion, and it does not correct a viewport coordinate convention mismatch.

You must first convert the sampled pixel centre into the correct NDC XY coordinates and the raw depth sample into the correct NDC Z.

If a depth texture uses reverse-Z, don't feed its value into an inverse matrix constructed for conventional depth. That mismatch will reconstruct incorrect positions even though every matrix multiplication compiles.


Once we recover a view-space point, transforming it to world space is conceptually straightforward.

The view transform converts world to camera:

$$ \mathbf p_v=V\mathbf p_w. $$

Therefore:

$$ \boxed{ \mathbf p_w=V^{-1}\mathbf p_v. } $$

For homogeneous points, the complete inverse view-projection expression is:

$$ \boxed{ \mathbf h_w=(PV)^{-1} \begin{bmatrix} x_n\\y_n\\q\\1 \end{bmatrix}, \qquad \mathbf p_w=\frac{\mathbf h_{w,xyz}}{h_{w,w}}. } $$

Because:

$$ (PV)^{-1}=V^{-1}P^{-1}, $$

this is equivalent to first reconstructing view space and then applying the inverse view matrix.

### A concrete worked example

Let the camera be located at:

$$ \mathbf c=(10,2,5) $$

with identity camera-to-world rotation under our right-handed camera convention.

Suppose the reconstructed view-space point is:

$$ \mathbf p_v=(4,-2,-8). $$

For an identity camera orientation, camera-to-world conversion is:

$$ \mathbf p_w=\mathbf c+\mathbf p_v. $$

Thus:

$$ \boxed{ \mathbf p_w=(14,0,-3). } $$

If we apply the world-to-camera transform again, we recover:

$$ (4,-2,-8). $$

This round-trip is a strong invariant for a reference test.

### Why large-world systems need special care

Suppose the camera world position is near $10^8$, but the reconstructed point is only centimetres away.

A shader that reconstructs:

$$ \mathbf p_w=\mathbf c_f+\mathbf p_v $$

using a binary32 camera position can lose the small displacement.

For many screen-space effects, it is better to keep view-space or camera-relative positions and avoid reconstructing large absolute world coordinates unnecessarily.

When world-space positions are genuinely required, preserve the camera origin in sufficient precision or use a split high/low coordinate representation.

### Why inverse VP sometimes fails in practice

Common causes include an incorrect matrix inverse, transposed upload, stale previous-frame matrices, wrong depth convention, incorrect Y flip, jittered-versus-unjittered projection mismatch, a depth value representing the far/infinite endpoint, and sampling the wrong pixel.

The algorithm is mathematically simple, but it sits at the intersection of almost every coordinate convention in the renderer.


A pixel alone does not determine a unique finite 3D point. It determines a ray, or in an orthographic camera, a line through a pixel-dependent origin with a shared viewing direction.

A ray is represented by:

$$ \boxed{ \mathbf r(t)=\mathbf o+t\hat{\mathbf d}, \qquad t\ge0 } $$

where $\mathbf o$ is its origin and $\hat{\mathbf d}$ is its normalized direction.

### Perspective camera: derive the ray from NDC

For a symmetric perspective camera:

$$ \mathbf q_v= \begin{bmatrix} a\,x_n\tan(\theta_y/2)\\ y_n\tan(\theta_y/2)\\ -1 \end{bmatrix}. $$

This vector points from the camera origin through the desired image position at unit axial depth.

Normalize it:

$$ \boxed{ \hat{\mathbf d}_v= \frac{\mathbf q_v}{\|\mathbf q_v\|}. } $$

The view-space ray is:

$$ \boxed{ \mathbf r_v(t)=t\hat{\mathbf d}_v. } $$

If the camera has world position $\mathbf c$ and camera-to-world orientation $R$, then:

$$ \boxed{ \mathbf o_w=\mathbf c, \qquad \hat{\mathbf d}_w=R\hat{\mathbf d}_v. } $$

### Pixel centre to NDC

For a top-left raster viewport with width $W$, height $H$, and pixel indices $(i,j)$, the centre is:

$$ x_p=i+0.5, \qquad y_p=j+0.5. $$

Map to NDC:

$$ \boxed{ x_n=\frac{2(i+0.5)}W-1, } $$

$$ \boxed{ y_n=1-\frac{2(j+0.5)}H. } $$

If your API's viewport mapping already flips Y in the projection or uses a positive-down NDC convention, adjust this step to the actual pipeline rather than applying an extra flip.

### Example: 90-degree square camera

Let:

$$ \theta_y=90^\circ,\qquad a=1. $$

At:

$$ (x_n,y_n)=(0.5,0.5), $$

the unnormalized ray direction is:

$$ \mathbf q=(0.5,0.5,-1). $$

Its norm is:

$$ \|\mathbf q\|= \sqrt{0.25+0.25+1} =\sqrt{1.5}. $$

Thus:

$$ \boxed{ \hat{\mathbf d} = \frac{(0.5,0.5,-1)}{\sqrt{1.5}} \approx (0.408248,0.408248,-0.816497). } $$

The normalized ray's forward component has magnitude less than one, so axial depth and ray distance differ.

If a depth buffer says the point's axial depth is ten, the corresponding ray parameter is:

$$ t=\frac{10}{0.816497}\approx12.24745. $$

That produces the same point as directly reconstructing $(5,5,-10)$.

### Orthographic cameras are different

In an orthographic camera, all viewing rays are parallel. The ray direction is constant, while the origin varies with the image position.

For an orthographic viewing rectangle with bounds $l,r,b,t$:

$$ x_v=l+\frac{x_n+1}{2}(r-l), $$

$$ y_v=b+\frac{y_n+1}{2}(t-b). $$

A convenient view-space ray is:

$$ \boxed{ \mathbf o_v=(x_v,y_v,-n), \qquad \hat{\mathbf d}_v=(0,0,-1). } $$

It begins on the near plane and travels forward.

A perspective ray, by contrast, originates at the camera centre.

### Ray generation through inverse projection

For general projection matrices, unproject a pixel location at two distinct valid depth values, producing two view-space points $\mathbf p_0,\mathbf p_1$. Their difference gives the ray direction:

$$ \boxed{ \hat{\mathbf d}= \frac{\mathbf p_1-\mathbf p_0} {\|\mathbf p_1-\mathbf p_0\|}. } $$

For a perspective camera, you may choose the camera centre as the ray origin. For orthographic projection, use the unprojected near-plane point.

This method handles off-axis matrices but still requires correct handling of infinite endpoints, nonstandard projection behavior, and coordinate conventions.


Mouse picking uses camera geometry to turn a 2D cursor position into a 3D query.

A typical editor has a scene rendered inside a panel rather than across the whole application window. Therefore mouse coordinates must first be converted into viewport-local coordinates.

Suppose the framebuffer viewport occupies:

$$ (x_0,y_0) $$

with size:

$$ W\times H. $$

The cursor position in the same framebuffer coordinate system is:

$$ (x_m,y_m). $$

Define viewport-local coordinates:

$$ \boxed{ x_\ell=x_m-x_0, \qquad y_\ell=y_m-y_0. } $$

Then:

$$ \boxed{ x_n=\frac{2x_\ell}{W}-1, \qquad y_n=1-\frac{2y_\ell}{H}. } $$

Here $(x_m,y_m)$ are continuous cursor coordinates, not pixel indices. Therefore we do not automatically add 0.5. Half-pixel offsets are appropriate when converting an integer pixel index to its centre.

### Worked viewport example

Suppose a scene viewport begins at:

$$ (x_0,y_0)=(200,100) $$

and has dimensions:

$$ W=800,\qquad H=600. $$

The mouse is at framebuffer coordinate:

$$ (x_m,y_m)=(600,400). $$

Its local position is:

$$ x_\ell=600-200=400, $$

$$ y_\ell=400-100=300. $$

Thus:

$$ x_n=\frac{2(400)}{800}-1=0, $$

$$ y_n=1-\frac{2(300)}{600}=0. $$

The cursor lies on the camera's optical axis.

For a centred perspective camera:

$$ \boxed{ \hat{\mathbf d}_v=(0,0,-1). } $$

### The rest of the picking pipeline

The ray is normally tested against a scene acceleration structure. A broad-phase BVH finds candidate objects, and narrower tests determine actual intersections.

A ray may intersect a triangle:

$$ \mathbf o+t\mathbf d = \mathbf v_0+ u(\mathbf v_1-\mathbf v_0)+ v(\mathbf v_2-\mathbf v_0). $$

Solving for $t,u,v$ gives a candidate hit, with the triangle conditions:

$$ u\ge0,\qquad v\ge0,\qquad u+v\le1. $$

The nearest valid positive $t$ is often the selected surface. The detailed intersection mathematics belongs to the computational-geometry phase that follows.

### Picking from an object-ID buffer

An alternative is to render object identifiers into an integer attachment, then read the identifier at the cursor pixel.

This can be effective for editor selection, and your renderer already has relevant object-ID rendering infrastructure.

But object-ID picking and ray picking solve slightly different problems. The ID buffer identifies the rendered visible object according to rasterization and the current frame's depth. A ray query can provide world-space hit position, normal, barycentric coordinates, material location, and potentially hits behind the first surface.

Both approaches need correct pixel-coordinate and viewport conventions.

### High-DPI displays

On macOS and other high-DPI platforms, UI pointer coordinates may be expressed in logical points while the render target uses physical pixels.

If the content scale factor is $s$, the framebuffer coordinate may be:

$$ x_{\mathrm{fb}}=s\,x_{\mathrm{logical}} $$

under a simple uniform scale. Real applications must use their actual window, drawable, and viewport transforms.

A picking implementation that forgets this conversion can appear correct on one monitor and miss selections on another.



<!-- end V11.14 -->


## stereo turns a calibrated angle into a distance

One camera pixel determines a ray, not a point: distance along the ray remains unknown. A second camera with a known relative pose supplies another ray. Their intersection determines a 3D point in the noise-free ideal case; with noise, choose the point minimizing the distance to both rays or minimizing reprojection error.

For rectified stereo cameras with parallel optical axes and horizontal baseline $B$, corresponding pixels differ by disparity $d=u_L-u_R$. With focal length $f$ in pixels and positive depth $Z$, similar triangles give $d=fB/Z$, hence $Z=fB/d$. For $f=800$ pixels, $B=0.12$ m and $d=48$ pixels, $Z=2.0$ m. If disparity uncertainty is 0.5 pixel, local sensitivity is $|\partial Z/\partial d|=fB/d^2\approx0.0417$ m/pixel, so depth uncertainty is about 0.021 m under the small-error approximation. At $d=4$ pixels, depth becomes 24 m and the same disparity uncertainty implies about 3 m uncertainty. Far points are much less well localized because their rays meet at a shallow angle.

The calculation assumes rectification, a shared focal convention, correct correspondence, and a metric baseline. Wrong correspondences can produce plausible-looking but false depth. Lens distortion must be removed or included in the projection model; otherwise epipolar alignment and disparity are biased. The uncertainty grows when the baseline is short, the image resolution is low, or the point lies far away. This is why a camera pipeline should preserve calibration covariance or at least communicate a depth confidence, rather than treating every reconstructed point as equally certain.

<!-- begin V11.15 -->
## Build projection and stereo geometry around real camera baselines


A symmetric perspective camera assumes the optical axis passes through the centre of its image rectangle. Its near-plane bounds satisfy:

$$ l=-r,\qquad b=-t. $$

But real imaging and XR systems often need asymmetric bounds. The viewing rectangle may be shifted relative to the camera's forward axis.

An off-axis perspective projection allows independent left, right, bottom, and top boundaries.

### Derive the matrix

Assume the camera uses negative-Z forward, positive near distance $n$, and near-plane bounds:

$$ (l,r,b,t). $$

We choose:

$$ w_c=-z_v. $$

For the X clip numerator, permit both an X term and a Z term:

$$ x_c=\alpha x_v+\beta z_v. $$

After division, at the near plane $z_v=-n$:

$$ x_n=\frac{\alpha x_v-\beta n}{n} =\frac{\alpha x_v}{n}-\beta. $$

We require:

$$ x_v=l\longmapsto x_n=-1, $$

$$ x_v=r\longmapsto x_n=1. $$

Therefore:

$$ \frac{\alpha l}{n}-\beta=-1, $$

$$ \frac{\alpha r}{n}-\beta=1. $$

Subtracting:

$$ \alpha\frac{r-l}{n}=2. $$

Hence:

$$ \boxed{\alpha=\frac{2n}{r-l}}. $$

Substituting into either boundary equation:

$$ \boxed{\beta=\frac{r+l}{r-l}}. $$

Repeating vertically gives:

$$ \frac{2n}{t-b} $$

and:

$$ \frac{t+b}{t-b}. $$

The complete right-handed, finite, conventional zero-to-one projection is:

$$ \boxed{ P_{\mathrm{off}}= \begin{bmatrix} \dfrac{2n}{r-l}&0&\dfrac{r+l}{r-l}&0\\ 0&\dfrac{2n}{t-b}&\dfrac{t+b}{t-b}&0\\ 0&0&-\dfrac f{f-n}&-\dfrac{fn}{f-n}\\ 0&0&-1&0 \end{bmatrix}. } $$

The nonzero entries $P_{0,2}$ and $P_{1,2}$ shift the projection's centre.

### Worked example · V11.15.03

Suppose:

$$ n=1,\quad l=-1,\quad r=3,\quad b=-1,\quad t=1. $$

The horizontal coefficients are:

$$ \alpha=\frac{2}{4}=0.5, $$

$$ \beta=\frac{-1+3}{4}=0.5. $$

Thus:

$$ x_c=0.5x_v+0.5z_v. $$

At the near plane $z_v=-1$, the left edge $x_v=-1$ gives:

$$ x_c=-0.5-0.5=-1,\qquad w_c=1. $$

Therefore $x_n=-1$.

At the right edge $x_v=3$:

$$ x_c=1.5-0.5=1, $$

so $x_n=1$.

Now consider the optical-axis point $(0,0,-1)$. Its horizontal projection is:

$$ x_n=-0.5. $$

Thus the camera's optical axis is no longer at NDC centre, even though the physical camera remains at the origin.

### What off-axis projection is not

Off-axis projection does not require rotating the camera.

Changing the view matrix rotates or moves the coordinate frame. Changing the off-axis projection shifts the image window relative to the optical axis.

These operations can sometimes produce similar-looking images for restricted geometry, but their three-dimensional relationships differ.

Off-axis projection is fundamental to calibrated principal-point offsets, head-mounted display views, stereo cameras, projection walls, and certain portal rendering systems.


Stereoscopic vision relies on observing a scene from two different viewpoints.

A simple stereo rig has two cameras separated horizontally by a baseline:

$$ \boxed{B}. $$

Assume their optical axes are parallel, their orientations agree, and their images have been rectified so corresponding epipolar lines are horizontal.

Place the left camera at:

$$ C_L=(-B/2,0,0) $$

and the right camera at:

$$ C_R=(+B/2,0,0). $$

Use a positive-Z-forward CV optical frame for this particular derivation.

Let a world point be:

$$ P=(X,Y,Z),\qquad Z>0. $$

### Derive the two image coordinates

Relative to the left camera:

$$ X_L=X+B/2. $$

For focal scale $f_x$ in pixels:

$$ \boxed{ u_L=f_x\frac{X+B/2}{Z}+c_x. } $$

Relative to the right camera:

$$ X_R=X-B/2. $$

Thus:

$$ \boxed{ u_R=f_x\frac{X-B/2}{Z}+c_x. } $$

Subtract:

$$ \Delta=u_L-u_R. $$

Therefore:

$$ \Delta = f_x \left[ \frac{X+B/2}{Z} - \frac{X-B/2}{Z} \right]. $$

The $X$ terms cancel:

$$ \Delta=f_x\frac BZ. $$

Thus:

$$ \boxed{ \Delta=\frac{f_xB}{Z}. } $$

This is the fundamental rectified stereo-disparity equation.

### Recover depth

Solve for $Z$:

$$ \boxed{ Z=\frac{f_xB}{\Delta}. } $$

Here $Z$ is axial camera depth, not Euclidean range.

The formula assumes parallel, aligned, rectified cameras with compatible intrinsic parameters and a disparity defined using coordinates whose principal-point offsets have been accounted for.

### Numerical example · V11.15.07

Suppose:

$$ f_x=800\text{ pixels}, $$

$$ B=0.064\text{ m}, $$

$$ Z=2\text{ m}. $$

Then:

$$ \Delta=\frac{800(0.064)}2 =\boxed{25.6\text{ pixels}}. $$

Now suppose the measured disparity is 25.6 pixels. Reconstruct:

$$ Z=\frac{800(0.064)}{25.6} =\boxed{2\text{ metres}}. $$

This result is independently checked in the C++23 lab.

### Why stereo depth accuracy worsens at distance

Differentiate:

$$ Z=\frac{f_xB}{\Delta}. $$

Then:

$$ \frac{dZ}{d\Delta} = -\frac{f_xB}{\Delta^2}. $$

Using $\Delta=f_xB/Z$:

$$ \boxed{ \frac{dZ}{d\Delta} = -\frac{Z^2}{f_xB}. } $$

A small disparity error $\delta\Delta$ produces approximately:

$$ \boxed{ \delta Z\approx -\frac{Z^2}{f_xB}\delta\Delta. } $$

The absolute depth uncertainty grows approximately as $Z^2$, for constant disparity uncertainty.

At longer distances, corresponding points appear almost at the same image location.

Depth becomes harder to measure.

### Depth-uncertainty example

Take:

$$ Z=10\text{ m},\quad f_x=800\text{ px},\quad B=0.064\text{ m}. $$

Suppose disparity uncertainty is approximately:

$$ \sigma_\Delta=0.1\text{ px}. $$

The first-order depth uncertainty is:

$$ \sigma_Z\approx \frac{Z^2}{f_xB}\sigma_\Delta. $$

Therefore:

$$ \sigma_Z\approx \frac{100}{51.2}(0.1) =\boxed{0.1953125\text{ m}}. $$

Even a tenth-pixel disparity error corresponds to roughly 19.5 centimetres of axial-depth error at this distance in the simple model.

This is a direct example of measurement uncertainty being amplified by the inverse geometry.

### Rectification and epipolar constraints

For general stereo camera poses, the images are not necessarily row-aligned. Corresponding image points lie along epipolar lines rather than automatically at the same pixel row.

Stereo rectification constructs image transformations that simplify correspondence search, ideally making corresponding epipolar lines horizontal.

The simple disparity equation is then applied in a compatible rectified coordinate system.

For general geometry, triangulation uses the known rays and camera transforms rather than assuming the simple formula holds without preprocessing.


Interpupillary distance, or IPD, is the distance between the centres of the pupils, usually measured in millimetres.

In a simplified stereo-camera model, it is represented as the separation between the two eye viewpoints.

Let the head-centre position be:

$$ \mathbf c. $$

Let the head's right vector be:

$$ \mathbf r. $$

For IPD $B$, define approximate left- and right-eye positions:

$$ \boxed{ \mathbf c_L=\mathbf c-\frac B2\mathbf r, } $$

$$ \boxed{ \mathbf c_R=\mathbf c+\frac B2\mathbf r. } $$

This is the symmetric rigid-head model.

### Why IPD matters

The baseline determines how strongly the two eye views differ.

From stereo geometry:

$$ \Delta=\frac{f_xB}{Z}. $$

Increasing $B$ increases disparity for a fixed scene depth and focal scale.

Decreasing $B$ reduces disparity.

But an XR headset must not choose an arbitrary IPD solely to exaggerate depth.

A mismatch between the user's eye geometry, the headset optics, and the rendered view origins can produce inaccurate depth perception or discomfort.

### Eye centres, lens centres, and display centres are different

A headset has physical lenses, displays, eye relief, and tracked viewing poses.

The separation between lenses is not necessarily identical to the user's anatomical IPD, nor does a graphics application always have direct access to every such physical parameter.

The rendering task is to use the view poses and field-of-view information supplied by the runtime or calibrated system.

OpenXR's view-location model provides per-view poses and field-of-view angles; applications are instructed to derive rendering views from those rather than assuming a universal fixed eye offset.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://registry.khronos.org\&sz=32)

registry.khronos.org+1



### Practical XR architecture

A simple desktop stereo demonstration can use:

```
leftEye  = headPosition - 0.5 * ipd * headRight;
rightEye = headPosition + 0.5 * ipd * headRight;
```

But a production OpenXR renderer should consume each runtime-provided eye pose and projection specification.

The runtime can account for headset-specific optical design, eye positioning, and tracking predictions.

This is particularly relevant to foveated rendering, where the projected location of gaze depends on the calibrated view and optics.


A common mistake in stereo rendering is constructing two identical symmetric projections and simply translating their cameras sideways, without considering the desired display plane or runtime optical geometry.

For some simple stereo arrangements, that is sufficient. For others, each eye requires an asymmetric projection.

### Why asymmetry appears

Suppose a shared virtual projection screen is fixed in world space while two eyes occupy different horizontal positions.

Each eye sees the same screen from a different viewpoint.

Relative to the left eye, the screen's horizontal boundaries are shifted to the right.

Relative to the right eye, those boundaries are shifted to the left.

Therefore the two eye frusta have different left/right near-plane bounds.

### Derive shifted near-plane bounds

Consider a world-space screen plane at forward distance $D$, with horizontal edges at:

$$ X=-W_s/2,\qquad X=+W_s/2. $$

Let the left eye be at:

$$ X_L=-B/2. $$

A ray from the left eye through a screen edge has horizontal displacement:

$$ X-X_L. $$

At a nearer clipping distance $n$, similar triangles scale this displacement by:

$$ \frac nD. $$

Thus the left-eye near-plane bounds are:

$$ \boxed{ l_L=\frac nD\left(-\frac{W_s}{2}+\frac B2\right), } $$

$$ \boxed{ r_L=\frac nD\left(+\frac{W_s}{2}+\frac B2\right). } $$

For the right eye at $+B/2$:

$$ \boxed{ l_R=\frac nD\left(-\frac{W_s}{2}-\frac B2\right), } $$

$$ \boxed{ r_R=\frac nD\left(+\frac{W_s}{2}-\frac B2\right). } $$

The frusta have opposite horizontal offsets.

These bounds are inserted into the off-axis projection matrix defined above.

### The zero-parallax plane

If the left and right images are arranged to correspond to the same physical screen plane, points on that plane can have zero or reference disparity according to the display mapping.

Points in front of the plane and behind it produce different disparity signs relative to that reference.

This is a conventional stereo-display construction, not a universal property of all headset projection systems.

### Why toe-in stereo is problematic

A toe-in rig rotates both cameras inward so their optical axes meet at a chosen convergence point.

Although intuitive, it can create vertical disparities and image-plane distortions when the resulting projections are displayed on ordinary flat aligned screens.

An alternative is to keep camera orientations parallel and use off-axis frusta to obtain the desired shared projection plane.

In XR, neither manually converging two cameras nor assuming identical symmetric frusta is an appropriate substitute for calibrated runtime-provided eye views.

### OpenXR angular frusta

An OpenXR view FOV can specify four angles:

$$ \theta_L,\quad\theta_R,\quad\theta_U,\quad\theta_D. $$

Under the usual sign convention:

$$ \theta_L<0,\qquad\theta_R>0, $$

$$ \theta_D<0,\qquad\theta_U>0 $$

for a symmetric field, although more general views need not straddle the optical axis in every direction.

At near distance $n$, derive the bounds:

$$ \boxed{ l=n\tan\theta_L,\qquad r=n\tan\theta_R, } $$

$$ \boxed{ b=n\tan\theta_D,\qquad t=n\tan\theta_U. } $$

These feed directly into the general asymmetric perspective matrix.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://registry.khronos.org\&sz=32)

registry.khronos.org



The total horizontal FOV is:

$$ \theta_R-\theta_L, $$

and total vertical FOV is:

$$ \theta_U-\theta_D. $$

Thus a production XR projection should be thought of as four angular boundaries, not necessarily one vertical FOV and aspect ratio.


Parallax is the apparent displacement of an object relative to a viewpoint change.

Stereo disparity is one form of parallax, but parallax also appears when a single camera translates through a scene.

### Derive translational parallax

For a pinhole camera with horizontal pixel focal scale $f_x$, consider a stationary 3D point with camera-relative coordinates:

$$ (X,Z) $$

in a positive-Z optical convention.

Its image coordinate relative to the principal point is:

$$ u-c_x=f_x\frac XZ. $$

Now translate the camera horizontally by $\Delta X$.

The point's new camera-relative horizontal coordinate is:

$$ X'=X-\Delta X. $$

Its new pixel coordinate is:

$$ u'-c_x=f_x\frac{X-\Delta X}{Z}. $$

Subtract:

$$ u'-u=-f_x\frac{\Delta X}{Z}. $$

Therefore:

$$ \boxed{ \Delta u=-\frac{f_x\Delta X}{Z}. } $$

The screen displacement is inversely proportional to depth.

Nearby geometry moves more in the image under the same camera translation.

Distant geometry moves less.

### Numerical example · V11.15.20

Let:

$$ f_x=1000\text{ px}, $$

$$ \Delta X=0.1\text{ m}. $$

For a point at depth:

$$ Z=1\text{ m}, $$

the pixel displacement is:

$$ \boxed{\Delta u=-100\text{ px}}. $$

At depth:

$$ Z=10\text{ m}, $$

it is:

$$ \boxed{\Delta u=-10\text{ px}}. $$

At depth:

$$ Z=100\text{ m}, $$

it is:

$$ \boxed{\Delta u=-1\text{ px}}. $$

This is motion parallax.

### Relative parallax

If two objects have depths $Z_1$ and $Z_2$, the difference between their projected displacements is:

$$ \boxed{ \Delta u_{\mathrm{relative}} = -f_x\Delta X \left( \frac1{Z_1}-\frac1{Z_2} \right). } $$

This difference is a depth cue even without stereoscopic viewing.

### Rotation versus translation

Pure camera rotation causes image motion, but the ideal pinhole rotational mapping does not depend on scene depth in the same way as translational parallax.

Under pure rotation:

$$ \mathbf d'_C=R\mathbf d_C, $$

and projection depends on the direction of the ray.

Thus rotating the camera can warp the entire image based on angular directions without knowing the 3D distance to every point, at least under the ideal central-camera model and assumptions about scene visibility.

Translation requires depth to produce correct parallax.

This distinction is essential for XR timewarp and reprojection.

Rotation-only reprojection can often improve the apparent responsiveness of distant imagery but cannot reproduce nearby translational parallax correctly.

### Parallax is not identical to perceived depth

Human depth perception combines binocular disparity, motion parallax, occlusion, perspective, shading, accommodation, vergence, and other cues.

Stereo geometry provides a mathematical disparity relationship, but perceived depth also depends on the observer, display, and presentation conditions.

An XR system can deliver geometrically consistent binocular views while still having accommodation–vergence conflicts because the eyes focus optically on a physical display plane while converging to simulated depths.

A complete XR design must distinguish geometric correctness from perceptual comfort.



### Figure C11-4 — Stereo turns pixel uncertainty into depth uncertainty

![[Visual Recall/Math Foundations/C11-4.png]]

Rectified pinhole pair: focal length 800 pixels, baseline 0.064 m, disparity noise σd=0.25 pixel. Z=fB/d and σZ≈Z²σd/(fB). This local propagation assumes corresponding points, small noise and valid calibration; it is not a perception model.


<!-- end V11.15 -->

<!-- begin V11.16 -->
## Represent directions over the full sphere


A cubemap represents an omnidirectional environment using six square images corresponding to the faces of a cube.

The faces are conventionally identified as:

$$ \boxed{+X,\ -X,\ +Y,\ -Y,\ +Z,\ -Z}. $$

Each face covers a nominal 90° square perspective field.

Collectively they capture directions over the sphere, apart from duplicated boundary directions.

### Construct six camera directions

For a camera located at the cube centre, the six forward directions are:

$$ \begin{aligned} \mathbf f_{+X}&=(1,0,0),\\ \mathbf f_{-X}&=(-1,0,0),\\ \mathbf f_{+Y}&=(0,1,0),\\ \mathbf f_{-Y}&=(0,-1,0),\\ \mathbf f_{+Z}&=(0,0,1),\\ \mathbf f_{-Z}&=(0,0,-1). \end{aligned} $$

But forward direction alone does not determine orientation. Each face also needs right and up directions.

A consistent face-orientation convention is necessary to avoid rotated or mirrored environment seams.

### Derive a cubemap direction mapping

Let a face texture coordinate be:

$$ (u,v)\in[0,1]^2. $$

Map it to signed face coordinates:

$$ s=2u-1, \qquad t=2v-1. $$

For one common convention with face $v$ increasing downward, choose these unnormalized direction vectors:

| Face     | Direction before normalization |
| -------- | ------------------------------ |
| $+X$ | $(1,-t,-s)$                |
| $-X$ | $(-1,-t,s)$                |
| $+Y$ | $(s,1,t)$                  |
| $-Y$ | $(s,-1,-t)$                |
| $+Z$ | $(s,-t,1)$                 |
| $-Z$ | $(-s,-t,-1)$               |

Normalize:

$$ \boxed{ \hat{\mathbf d}= \frac{\mathbf d}{\|\mathbf d\|}. } $$

At the centre of every face:

$$ u=v=0.5, $$

so:

$$ s=t=0. $$

The face-centre rays become the six principal axis directions.

This is verified in the C++23 reference lab.

### Why the mapping is not uniform in solid angle

Every face texel has the same area in the 2D face texture, but it does not cover the same solid angle on the sphere.

For a cube face parameterized by unnormalized direction $(1,s,t)$, the differential solid angle has the form:

$$ \boxed{ d\omega= \frac{ds\,dt} {(1+s^2+t^2)^{3/2}}. } $$

Thus texels near face corners cover different solid angles than texels near the face centre.

For physically based environment-light importance sampling, using equal probability per texel without accounting for solid angle can bias or inefficiently weight the distribution.

The correct PDF transformation matters.

### Cubemaps and real environment reconstruction

Cubemaps are common in skyboxes, reflection probes, environment maps, shadow maps for point lights, and omnidirectional image representations.

But a cubemap from a single viewpoint stores directions, not general parallax-correct geometry.

If the observer moves significantly inside a room and the renderer simply samples a fixed cubemap, reflections may appear incorrectly attached to infinitely distant surroundings.

Parallax-corrected probes, local geometry, or more advanced reconstruction representations can address this limitation.

### Seam debugging

To debug a cubemap, label each face with its axis and local right/up directions, then render a ray at the centre and at all four corners.

A visually plausible cubemap can still have inconsistent face orientation that becomes obvious only when directional textures cross seams.


An equirectangular image represents a spherical environment using horizontal longitude and vertical latitude coordinates.

It is a nonlinear camera mapping.

Unlike a central perspective projection, it cannot represent an entire sphere through one ordinary $4\times4$ perspective matrix.

PBRT's spherical-camera implementation treats this as a separate camera family rather than a standard projective camera.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://pbr-book.org\&sz=32)

pbr-book.org



### Define spherical coordinates

Let longitude be:

$$ \lambda\in[-\pi,\pi) $$

and latitude be:

$$ \phi\in[-\pi/2,\pi/2]. $$

Under our camera-local convention, we choose:

- Longitude $0$ points forward along $-Z$.
- Positive longitude turns toward $+X$.
- Positive latitude points upward along $+Y$.

A unit direction is then:

$$ \boxed{ \mathbf d(\lambda,\phi)= \begin{bmatrix} \cos\phi\sin\lambda\\ \sin\phi\\ -\cos\phi\cos\lambda \end{bmatrix}. } $$

### Convert image UV to angles

Let:

$$ u,v\in[0,1]. $$

Map horizontal UV to longitude:

$$ \boxed{ \lambda=2\pi(u-0.5). } $$

Map vertical UV to latitude:

$$ \boxed{ \phi=\pi(0.5-v). } $$

Thus $(u,v)=(0.5,0.5)$ maps to:

$$ \lambda=0,\qquad\phi=0, $$

giving forward ray:

$$ \boxed{\mathbf d=(0,0,-1)}. $$

The top of the image, $v=0$, maps to the north pole:

$$ (0,1,0). $$

The bottom, $v=1$, maps to the south pole:

$$ (0,-1,0). $$

### Inverse mapping

Given a normalized direction $(d_x,d_y,d_z)$, recover longitude:

$$ \boxed{ \lambda=\operatorname{atan2}(d_x,-d_z). } $$

Recover latitude:

$$ \boxed{ \phi=\arcsin(d_y). } $$

Then:

$$ \boxed{ u=\frac{\lambda}{2\pi}+0.5, \qquad v=0.5-\frac{\phi}{\pi}. } $$

Horizontal wrapping should be handled consistently.

At the poles, longitude is not unique; any horizontal coordinate represents the same pole direction.

### Equirectangular is not equal-area

A small spherical patch has solid angle:

$$ \boxed{ d\omega=\cos\phi\,d\phi\,d\lambda. } $$

Because $u,v$ map linearly to $\lambda,\phi$:

$$ d\lambda=2\pi\,du, $$

$$ d\phi=-\pi\,dv. $$

Taking the positive area measure:

$$ \boxed{ d\omega=2\pi^2\cos\phi\,du\,dv. } $$

Thus a uniform distribution over equirectangular image pixels is not uniform over sphere directions.

Near the poles:

$$ \cos\phi\to0. $$

Each pixel covers a very small solid angle.

Near the equator:

$$ \cos\phi=1. $$

Pixel solid angle is larger.

### Importance sampling an HDR panorama

Suppose texel luminance is $L_{ij}$.

If we want to sample incident environment radiance proportionally to its contribution under an approximately uniform angular weighting, a common discrete importance weight is:

$$ \boxed{ w_{ij}\propto L_{ij}\sin\theta_j } $$

where $\theta$ is polar angle measured from the north pole.

Because:

$$ \sin\theta=\cos\phi. $$

This compensates for equirectangular solid-angle distortion.

For an actual radiance integral, the complete estimator must divide by the directional PDF expressed per unit solid angle and include any BRDF or cosine weighting as appropriate.

### Why this matters to Maveb

A panorama is excellent for recording environment appearance from one viewpoint, but it does not determine nearby scene geometry.

If the camera translates, pure panorama rotation cannot correctly reproduce near-field motion parallax.

For reconstruction, panoramas can complement geometric representations but should not be confused with a complete 3D model.


A fisheye lens maps rays covering a very wide angular field into a finite image disk.

Its mapping is fundamentally different from ordinary rectilinear perspective projection.

For a rectilinear pinhole camera, the image radius for a ray at angle $\theta$ from the optical axis is:

$$ \boxed{ r=f\tan\theta. } $$

As:

$$ \theta\to90^\circ, $$

the radius tends toward infinity.

Therefore one ordinary rectilinear perspective projection cannot represent a 180° field within a finite image plane.

Fisheye models use different radius-versus-angle relationships.

### Four common fisheye mappings

Let $f$ be the relevant image focal scale and $\theta$ the angular distance from the optical axis.

| Projection model     | Radius $r(\theta)$ | Characteristic                                     |
| -------------------- | ---------------------- | -------------------------------------------------- |
| Equidistant          | $f\theta$          | Radius proportional to angle                       |
| Equisolid-angle      | $2f\sin(\theta/2)$ | Equal solid angle maps to equal image area locally |
| Stereographic        | $2f\tan(\theta/2)$ | Conformal spherical mapping, except singularity    |
| Orthographic fisheye | $f\sin\theta$      | Projection of sphere onto plane                    |

These are idealized projection families. Real lenses may deviate from them and require calibration.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.mdpi.com\&sz=32)

mdpi.com



### Derive the equidistant mapping

If image radius is proportional to the incoming ray angle:

$$ r=k\theta. $$

Conventionally write the scale as $f$:

$$ \boxed{r=f\theta}. $$

The inverse is:

$$ \boxed{\theta=\frac rf}. $$

This is particularly convenient for wide-angle ray generation.

### Derive the equisolid-angle form

The solid angle of a spherical cap with polar half-angle $\theta$ is:

$$ \boxed{ \Omega(\theta)=2\pi(1-\cos\theta). } $$

Using the identity:

$$ 1-\cos\theta=2\sin^2(\theta/2), $$

we get:

$$ \Omega(\theta)=4\pi\sin^2(\theta/2). $$

The area of an image disk of radius $r$ is:

$$ A=\pi r^2. $$

For an equal-area mapping, choose image area proportional to solid angle:

$$ \pi r^2=f^2\Omega. $$

Then:

$$ r^2=4f^2\sin^2(\theta/2). $$

Thus:

$$ \boxed{ r=2f\sin(\theta/2). } $$

This explains the geometric meaning of the equisolid-angle formula.

### Numerical comparison · V11.16.16

Take:

$$ \theta=60^\circ=\pi/3, \qquad f=10\text{ mm}. $$

For equidistant projection:

$$ r=10\cdot\frac\pi3 \approx\boxed{10.472\text{ mm}}. $$

For equisolid-angle:

$$ r=20\sin30^\circ =\boxed{10\text{ mm}}. $$

For stereographic:

$$ r=20\tan30^\circ \approx\boxed{11.547\text{ mm}}. $$

For orthographic:

$$ r=10\sin60^\circ \approx\boxed{8.660\text{ mm}}. $$

The same physical ray maps to different image radii under different models.

### Generate a fisheye ray from a pixel

Suppose a pixel is at displacement:

$$ (\Delta x,\Delta y) $$

from the principal point in image-plane distance units, with $+Y$ upward for this local derivation.

Compute:

$$ r=\sqrt{\Delta x^2+\Delta y^2}. $$

Use the chosen inverse radius mapping to recover:

$$ \theta. $$

Define the azimuth:

$$ \psi=\operatorname{atan2}(\Delta y,\Delta x). $$

Then the right-handed graphics-space ray direction is:

$$ \boxed{ \hat{\mathbf d}= \begin{bmatrix} \sin\theta\cos\psi\\ \sin\theta\sin\psi\\ -\cos\theta \end{bmatrix}. } $$

Check at the optical centre:

$$ r=0\implies\theta=0. $$

Therefore:

$$ \hat{\mathbf d}=(0,0,-1). $$

For rays reaching 90° from forward, $d_z=0$, meaning they travel perpendicular to the optical axis.

Some fisheye models can represent rear-hemisphere rays, but their valid angle ranges and injectivity differ. For example, $r=f\sin\theta$ is not one-to-one over the full $0$ to $\pi$ interval.

### Why fisheye matters for XR and capture

Wide-angle lenses can capture much of the visual environment at once, which is attractive for camera tracking and scene reconstruction.

But applying a standard pinhole camera model to fisheye observations can produce large reprojection errors toward the image edges.

A calibrated fisheye model should be used for measurement geometry, with explicit distortion and pixel conventions.

A wide-angle camera can also have nonuniform angular sampling density, which matters for foveated rendering and reconstruction quality.


Parts 1092–1100

The remaining nine parts move from geometric projection to image formation.

A purely geometric camera answers: Which world ray corresponds to this pixel?

A physical camera model also asks: Which rays pass through the aperture, for how long are they collected, where do they focus, and how is the collected light converted into image values?

These questions require both ray geometry and radiometry.



### Figure C11-7 — Image coordinates need a sampling-measure conversion

![[Visual Recall/Math Foundations/C11-7.png]]

For longitude λ and latitude φ, dω=cosφ dλ dφ. Equal-width equirectangular image rows shrink toward the poles. Ray-differential Jacobians generalize the local image-area-to-solid-angle conversion; they do not by themselves specify lens-area or shutter-time probability.


<!-- end V11.16 -->

<!-- begin V11.17 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/34 - Robust Predicates and Camera Sensitivity|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/36 - Exposure, Physical Optics, and Depth of Field|Next chapter]]
