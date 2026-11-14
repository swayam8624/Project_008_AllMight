# 36 — Exposure, Physical Optics, and Depth of Field

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging|Next chapter]]

Camera exposure converts scene radiance into sensor measurements, while aperture and focus shape depth of field and blur. The chapter ties physical optics to rendering and image formation, then connects those models to pose estimation and bundle adjustment.

**By the end:** Relate exposure and optical parameters to image formation, and understand how multi-view consistency constrains camera and scene estimates.

**Recall question:** Which camera controls change exposure, which change focus blur, and how can their effects be distinguished in an image?

<!-- Derived from Continuous Notes/Computing Foundations - From Bits to C++ and Systems APIs.md; source lines 89920–92789; source-span SHA256 ca3daa5411b64517156270bd4ca15121143ad0ef96df38fc7b31de3fca54753f. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Model exposure, physical optics, and depth of field


Exposure controls how much light contributes to an image measurement over a given capture interval and optical configuration.

In a physical camera, the relevant quantities include lens transmission, aperture area, shutter duration, scene radiance, sensor response, and electronic or photographic sensitivity.

In a graphics renderer, there is an additional distinction between physical exposure and an arbitrary display-exposure multiplier applied to an HDR image.

### Exposure versus scene radiance

Radiance $L$ describes the light travelling along a ray per unit projected area and solid angle, with spectral or wavelength dependence in the full physical definition.

Exposure does not change the light emitted by the scene.

Instead, exposure affects how light arriving at the imaging system is converted into sensor signal or displayed brightness.

A simplified image model might be:

$$ \boxed{ I_{\mathrm{linear}}=E\,L } $$

where $E$ is a dimensionless rendering exposure scale after incorporating the chosen normalization and physical assumptions.

This is a useful renderer abstraction, but it should not be mistaken for the complete radiometric sensor equation.

### Exposure stops

A stop is a factor of two in exposure.

Increasing exposure by one stop doubles the exposure multiplier:

$$ \boxed{E'=2E}. $$

Decreasing exposure by one stop halves it:

$$ \boxed{E'=E/2}. $$

For an exposure adjustment of $s$ stops:

$$ \boxed{ E'=E\,2^s. } $$

Thus:

$$ s=+2\implies E'=4E, $$

and:

$$ s=-3\implies E'=E/8. $$

### Photographic exposure value

A common photographic exposure-value definition is:

$$ \boxed{ EV=\log_2\left(\frac{N^2}{t}\right) } $$

where $N$ is the f-number and $t$ is shutter duration in seconds, interpreted using the standard photographic conventions for exposure-value calculations.

For an ISO-adjusted reference to ISO 100:

$$ \boxed{ EV_{100} = \log_2\left( \frac{N^2}{t}\frac{100}{S} \right) } $$

where $S$ is ISO sensitivity.

This matches the implementation strategy documented in Google's Filament exposure system.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://github.com\&sz=32)

GitHub



### Derive the EV relationship

In a simplified camera, irradiance reaching the sensor from a given scene luminance is approximately proportional to the aperture area. At fixed focal length, aperture diameter is:

$$ D=\frac fN. $$

Thus aperture area scales as:

$$ D^2\propto\frac1{N^2}. $$

Integrating over shutter duration $t$:

$$ \boxed{ H\propto\frac t{N^2} } $$

for a fixed scene, optical transmission, and geometric configuration.

The reciprocal quantity:

$$ \frac{N^2}{t} $$

therefore increases as the capture becomes darker.

Taking the base-two logarithm expresses changes in stops:

$$ EV=\log_2(N^2/t). $$

Increasing $EV$ by one decreases exposure by one stop when the scene and ISO remain fixed.

### Numerical example · V11.17.06

Let:

$$ N=2.8,\qquad t=\frac1{125}\text{ s},\qquad S=100. $$

Then:

$$ EV_{100} = \log_2\left( \frac{2.8^2}{1/125} \right). $$

Compute:

$$ 2.8^2=7.84. $$

Thus:

$$ \frac{7.84}{0.008}=980. $$

Therefore:

$$ \boxed{ EV_{100}=\log_2(980)\approx9.94. } $$

A scene captured at different aperture/shutter combinations can maintain the same exposure value if:

$$ \frac{N^2}{t} $$

remains constant.

### Exposure and tone mapping are not the same

In a typical HDR rendering pipeline, exposure scaling occurs before a tone-mapping transform:

$$ \boxed{ C_{\mathrm{display}} = T(E\,C_{\mathrm{HDR}}) } $$

where $T$ is a tone mapper.

Because tone mapping is nonlinear, generally:

$$ T(EC)\ne E\,T(C). $$

Changing exposure before tone mapping can alter highlight compression, saturation behavior, and the amount of detail visible in bright regions.

Applying a brightness multiplier to the already tone-mapped output is not equivalent to changing scene-linear exposure.

### Auto-exposure

An auto-exposure system estimates scene brightness and adjusts $E$ over time.

One simple strategy estimates a representative log luminance:

$$ \boxed{ L_{\log} = \exp\left( \frac1N\sum_i\log(\max(L_i,\epsilon)) \right). } $$

This is a geometric mean of luminance values after flooring them to a positive $\epsilon$.

A target exposure might be proportional to:

$$ \frac{L_{\mathrm{target}}}{L_{\log}}. $$

But practical systems also need exposure limits, temporal adaptation, outlier handling, and an intentional metering region.

A foveated or gaze-aware renderer might weight the metering region differently, but doing so is a perceptual and artistic design choice rather than a universally correct physical rule.


The aperture is the opening through which light enters the optical system.

In a simplified thin-lens camera, we model it as a circular disk of diameter $D$.

The focal length is $f$.

The f-number is defined as:

$$ \boxed{ N=\frac fD. } $$

Thus:

$$ \boxed{ D=\frac fN. } $$

### F-number is inversely related to aperture size

For a fixed focal length, a smaller f-number means a larger aperture.

For example, with:

$$ f=50\text{ mm}, $$

at f/2:

$$ D=\frac{50}{2}=\boxed{25\text{ mm}}. $$

At f/4:

$$ D=\frac{50}{4}=\boxed{12.5\text{ mm}}. $$

The f/2 aperture has twice the diameter.

Its area is four times larger.

### Derive the aperture area

For a circular opening:

$$ A=\pi r^2. $$

Since:

$$ r=D/2, $$

we obtain:

$$ \boxed{ A=\pi\left(\frac D2\right)^2 =\frac{\pi D^2}{4}. } $$

Substitute $D=f/N$:

$$ \boxed{ A=\frac{\pi f^2}{4N^2}. } $$

At fixed focal length:

$$ A\propto\frac1{N^2}. $$

### The f-stop sequence

To halve the aperture area, the diameter must decrease by a factor of:

$$ \sqrt2. $$

Since:

$$ N=f/D, $$

the f-number must increase by:

$$ \sqrt2. $$

Therefore the one-stop sequence is approximately:

$$ \boxed{ f/1,\ f/1.4,\ f/2,\ f/2.8,\ f/4,\ f/5.6,\ f/8,\ f/11,\ f/16. } $$

Every adjacent step reduces aperture area by approximately half.

This is why the f-stop scale is multiplicative rather than additive.

### Aperture affects more than brightness

A larger aperture allows more light through, but it also changes the family of rays contributing to each image pixel.

That produces shallower depth of field under ordinary thin-lens conditions.

The aperture's shape affects out-of-focus highlight shapes, commonly called bokeh.

A circular aperture produces approximately circular defocus footprints in the simple model.

A polygonal aperture may produce polygonal highlight shapes.

Real optical systems also introduce vignetting, aberrations, pupil magnification, cat's-eye bokeh, and other effects not captured by a simple disk.

PBRT's physical-camera material distinguishes the simplified thin-lens approximation from more realistic multi-element lens systems.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.pbr-book.org\&sz=32)

pbr-book.org



### Sampling a disk uniformly

For ray tracing, we need to generate aperture locations with uniform area probability.

Let:

$$ u_1,u_2\sim U(0,1). $$

Choose:

$$ \boxed{ r=R\sqrt{u_1}, \qquad \phi=2\pi u_2, } $$

where $R=D/2$.

Then:

$$ \boxed{ x_a=r\cos\phi, \qquad y_a=r\sin\phi. } $$

Why $\sqrt{u_1}$?

The area of a disk of radius $r$ is:

$$ \pi r^2. $$

The fraction of total area inside radius $r$ is:

$$ F(r)=\frac{r^2}{R^2}. $$

For inverse-transform sampling:

$$ u_1=\frac{r^2}{R^2}. $$

Therefore:

$$ \boxed{r=R\sqrt{u_1}}. $$

If we sampled $r=Ru_1$, the samples would be excessively concentrated near the centre in area terms.


Shutter time is the duration over which the imaging system integrates incoming light for an exposure.

For a stationary scene with constant illumination, a longer shutter duration collects more light, subject to the camera's physical and electronic behavior.

In rendering, shutter duration also defines the time interval over which geometry and camera motion can contribute to motion blur.

### Mathematical image formation over time

Suppose the radiance contributing to a pixel varies with time:

$$ L(t). $$

A simplified exposure measurement is proportional to:

$$ \boxed{ H=\int_{t_0}^{t_1}L(t)\,dt. } $$

For constant radiance:

$$ L(t)=L_0, $$

the integral becomes:

$$ \boxed{ H=L_0(t_1-t_0). } $$

Thus doubling duration doubles the collected exposure under the simplified assumptions.

### Motion blur as a temporal integral

For a moving scene, the pixel value depends on time:

$$ C(t). $$

A normalized rendering estimate may represent a shutter-averaged pixel:

$$ \boxed{ \bar C= \frac1T\int_{t_0}^{t_1}C(t)\,dt, \qquad T=t_1-t_0. } $$

One Monte Carlo estimator samples times:

$$ t_k=t_0+Tu_k, \qquad u_k\sim U(0,1). $$

Then:

$$ \boxed{ \widehat{\bar C} = \frac1N\sum_{k=1}^{N}C(t_k). } $$

This is unbiased for the idealized time-average under independent uniform sampling, assuming finite integrable values and correct scene evaluation at each sample time.

For physical exposure rather than a normalized time-average, the integration scale $T$ must be retained.

### A moving-point example

Suppose a point moves horizontally at constant world velocity:

$$ v=2\text{ m/s}. $$

The shutter remains open for:

$$ T=\frac1{30}\text{ s}. $$

Its world displacement during the exposure is:

$$ \Delta x=vT. $$

Thus:

$$ \boxed{ \Delta x=\frac2{30}\approx0.0667\text{ m}. } $$

Its corresponding image displacement depends on projection depth and focal scale.

At depth $Z=5$ metres with $f_x=1000$ pixels, approximate image blur displacement is:

$$ \Delta u\approx\frac{f_x\Delta x}{Z}. $$

Therefore:

$$ \Delta u\approx \frac{1000(0.0667)}5 \approx\boxed{13.33\text{ pixels}}. $$

That is potentially significant motion blur.

### Global shutter versus rolling shutter

A simplified global shutter exposes the entire image over a common time interval.

A rolling shutter exposes or reads different image rows at different times.

For a rolling-shutter model, row $j$ might have sample time:

$$ \boxed{ t(j,u)= t_{\mathrm{frame}} +\alpha\frac jH +uT_{\mathrm{exposure}} } $$

where $\alpha$ describes the readout progression and $u\in[0,1]$ selects a time within that row's exposure interval.

This is a simplified model; real rolling shutters have device-dependent timing.

Rolling shutter can skew moving objects, bend straight lines, and introduce pose-dependent calibration errors.

It matters in camera tracking, fast motion, and visual-inertial systems.

### Shutter angle

Cinematic rendering sometimes specifies shutter using a shutter angle rather than absolute seconds.

For a frame interval:

$$ T_f=\frac1{\mathrm{fps}}, $$

a conventional shutter angle $\alpha$ degrees corresponds to exposure duration:

$$ \boxed{ T_{\mathrm{exposure}} = T_f\frac{\alpha}{360^\circ}. } $$

At 24 frames per second and 180° shutter:

$$ T_{\mathrm{exposure}} = \frac1{24}\cdot\frac12 = \boxed{\frac1{48}\text{ second}}. $$

This links cinematic motion blur to the camera's frame rate.

### A critical ray-tracing implementation detail

When a ray is assigned a sample time $t$, every time-dependent intersection must evaluate geometry at a compatible time.

Sampling a shutter time but intersecting against geometry frozen at a single frame does not produce correct motion blur.

Likewise, acceleration structures may need swept bounds or time-dependent transforms to remain conservative across the shutter interval.


ISO sensitivity describes a standardized relationship between the camera's exposure and resulting image response. In digital cameras, the practical behavior depends on sensor electronics, gain, processing, and the ISO standard or measurement method being used.

The most important distinction is:

Increasing ISO does not cause more photons to enter the camera.

Aperture and shutter time affect collected optical exposure. ISO settings affect the resulting image response or amplification under the camera's implementation.

### Simple exposure model

For a fixed scene, focal length, optical transmission, and viewing geometry, a simplified sensor signal before response processing scales as:

$$ H\propto\frac t{N^2}. $$

If we model ISO gain as proportional to $S/100$, an illustrative output-brightness factor is:

$$ \boxed{ E_{\mathrm{relative}} = \frac t{N^2}\frac S{100}. } $$

This is not a complete physical sensor model, but it is useful for a graphics renderer's camera-control abstraction.

### Numerical example · V11.17.21

Suppose:

$$ N=4,\qquad t=\frac1{100},\qquad S=100. $$

Then:

$$ E=\frac{0.01}{16}=\boxed{0.000625}. $$

Now change ISO to 200 while leaving aperture and shutter unchanged:

$$ E'=\frac{0.01}{16}\cdot2 =\boxed{0.00125}. $$

The simplified output-brightness factor doubles.

But the actual number of collected photons does not.

### ISO and noise

Photon shot noise follows photon-counting statistics approximately.

If $N_\gamma$ photons are collected, a simplified Poisson model gives:

$$ \operatorname{Var}(N_\gamma)=\mathbb E[N_\gamma]. $$

Thus signal magnitude scales as:

$$ N_\gamma, $$

while noise standard deviation scales as:

$$ \sqrt{N_\gamma}. $$

The photon-limited signal-to-noise ratio is approximately:

$$ \boxed{ \mathrm{SNR}\approx\sqrt{N_\gamma}. } $$

Doubling electronic gain amplifies signal and upstream noise together; it does not double the photon count or automatically improve photon-limited SNR.

Real sensors also have read noise, dark current, quantization, saturation, and gain-stage details, so ISO behavior can be more complicated.

### Why physically based renderers must distinguish exposure from gain

If the goal is physically plausible image formation, aperture diameter and shutter duration should affect photon collection and noise characteristics.

ISO-like gain should influence the sensor response without pretending to change light transport.

For an artistic renderer, a simpler exposure multiplier may be sufficient.

But a rigorous camera API should not silently equate all three quantities with the same generic brightness control.

### EV100 example

Using:

$$ N=2,\quad t=1/125,\quad S=100, $$

we obtain:

$$ EV_{100} = \log_2\left(\frac4{0.008}\right) = \log_2(500). $$

Thus:

$$ \boxed{EV_{100}\approx8.965784}. $$

The laboratory evaluates this exact configuration.

If we double ISO to 200 without changing aperture or shutter, the ISO-adjusted EV100 decreases by one stop:

$$ \boxed{ EV'_{100}=EV_{100}-1. } $$


The circle of confusion (CoC) is the diameter of the blur footprint produced on an image plane by a point that is not exactly in focus under a finite-aperture imaging system.

An ideal pinhole camera associates one infinitesimal ray with each image point and has no ordinary geometric defocus blur.

A finite-aperture lens accepts a bundle of rays from the same scene point. Those rays ideally converge to one image point only at the correct conjugate image distance.

If the sensor plane lies somewhere else, that ray bundle intersects the sensor over a finite region.

For a circular aperture and idealized paraxial thin lens, that region is approximately a disk.

### The thin-lens equation

Let:

- $f$ = focal length, in metres.
- $s$ = object distance from lens, in metres.
- $v(s)$ = corresponding image distance behind lens.
- $S$ = object distance at which the camera is focused.
- $v_f=v(S)$ = image distance at the focus setting.
- $D$ = aperture diameter.

The Gaussian thin-lens equation is:

$$ \boxed{ \frac1f=\frac1s+\frac1{v(s)}. } $$

Solve for image distance:

$$ \frac1{v(s)}=\frac1f-\frac1s =\frac{s-f}{fs}. $$

Therefore:

$$ \boxed{ v(s)=\frac{fs}{s-f}. } $$

The sensor position corresponding to focus distance $S$ is:

$$ \boxed{ v_f=\frac{fS}{S-f}. } $$

### Derive the CoC diameter

A ray cone has diameter $D$ at the lens and converges to zero diameter at the ideal image point $v(s)$.

At sensor position $v_f$, similar triangles give:

$$ \boxed{ c=D\left|1-\frac{v_f}{v(s)}\right|. } $$

Substitute the thin-lens image distances:

$$ \frac{v_f}{v(s)} = \frac{fS/(S-f)}{fs/(s-f)} = \frac{S(s-f)}{s(S-f)}. $$

Therefore:

$$ c=D \left| 1-\frac{S(s-f)}{s(S-f)} \right|. $$

Combine the numerator:

$$ s(S-f)-S(s-f) = sS-sf-Ss+Sf =f(S-s). $$

Hence:

$$ \boxed{ c= D\frac{f|S-s|}{s(S-f)}. } $$

Since $D=f/N$, we can also write:

$$ \boxed{ c= \frac{f^2|S-s|} {Ns(S-f)}. } $$

This is the idealized sensor-plane CoC diameter.

All distances in the formula must use consistent physical units.

### Numerical example · V11.17.27

Take:

$$ f=50\text{ mm}=0.05\text{ m}, $$

$$ N=2, $$

$$ S=3\text{ m}, $$

$$ s=6\text{ m}. $$

Aperture diameter:

$$ D=\frac{0.05}{2}=0.025\text{ m}. $$

Now:

$$ c= 0.025 \frac{ 0.05|3-6| }{ 6(3-0.05) }. $$

Simplify:

$$ c= 0.025 \frac{0.15}{17.7}. $$

Thus:

$$ c\approx0.0002118644\text{ m}. $$

Convert to millimetres:

$$ \boxed{ c\approx0.2118644\text{ mm}. } $$

This exact result appears in the compiled laboratory.

### CoC in pixels

Suppose the sensor width is:

$$ 36\text{ mm} $$

and the image width is:

$$ 6000\text{ pixels}. $$

The horizontal pixel density is:

$$ \frac{6000}{36} \approx166.667\text{ px/mm}. $$

The blur diameter in pixels is:

$$ c_{\mathrm{px}} = 0.2118644 \cdot166.667. $$

Therefore:

$$ \boxed{ c_{\mathrm{px}}\approx35.31\text{ pixels}. } $$

That is a substantial defocus footprint.

### Signed CoC

For some post-processing implementations, it is useful to preserve whether a point lies in front of or behind the focus plane.

Instead of using $|S-s|$, define a signed quantity:

$$ \boxed{ c_{\mathrm{signed}} = D\frac{f(S-s)}{s(S-f)} } $$

under this explicit sign convention.

Then near and far defocus have opposite signs, while the magnitude determines the approximate blur diameter.

Different engines may choose the opposite sign. The convention must be documented.

### CoC is not a perfect physical blur disk

The circular model assumes an idealized lens and aperture.

Actual defocus can be affected by aperture-blade shape, optical aberrations, cat's-eye pupil effects, vignetting, wavelength, sensor geometry, and diffraction.

The CoC is a geometric approximation, not a complete optical point-spread function.


Depth of field (DoF) is the range of object distances whose projected blur remains below an acceptable threshold.

The wording is important: real lenses do not create a perfectly sharp finite volume followed by a perfectly blurred region. For an ideal thin lens, only the exact focus plane has zero geometric defocus. Nearby depths have progressively increasing CoC.

A depth-of-field range is defined by choosing a maximum acceptable CoC diameter.

Let:

$$ c_{\max}>0 $$

be that threshold on the sensor.

Then a point is considered acceptably focused when:

$$ \boxed{ c(s)\le c_{\max}. } $$

### Depth of field depends on the output criterion

The acceptable CoC threshold is not a universal physical constant.

It depends on sensor size, output resolution, display size, viewing distance, image-processing choices, and desired sharpness.

For example, a CoC of 0.03 mm may be acceptable for one photographic output but visibly blurred at high-resolution pixel inspection.

A physically based renderer may simulate the lens blur directly without imposing a binary “in focus” threshold. The threshold is useful when defining near/far DoF limits.

### Derive the hyperfocal distance

Start from:

$$ c(s)= \frac{f^2|S-s|}{Ns(S-f)}. $$

Consider a point at infinity:

$$ s\to\infty. $$

Then:

$$ \frac{|S-s|}{s}\to1. $$

Therefore the infinity CoC, when focused at $S$, is:

$$ \boxed{ c_\infty= \frac{f^2}{N(S-f)}. } $$

The hyperfocal distance $H$ is conventionally the focus distance at which infinity reaches the chosen acceptable CoC threshold:

$$ c_\infty=c_{\max}. $$

Thus:

$$ c_{\max} = \frac{f^2}{N(H-f)}. $$

Rearrange:

$$ N c_{\max}(H-f)=f^2. $$

Therefore:

$$ \boxed{ H=\frac{f^2}{N c_{\max}}+f. } $$

This formula assumes the paraxial thin-lens model and a consistent sensor-plane CoC threshold.

### Numerical hyperfocal example

Take:

$$ f=50\text{ mm}, $$

$$ N=8, $$

$$ c_{\max}=0.03\text{ mm}. $$

Then:

$$ H=\frac{50^2}{8(0.03)}+50 $$

in millimetres.

Compute:

$$ \frac{2500}{0.24}=10416.6667. $$

Therefore:

$$ H=10466.6667\text{ mm}. $$

Thus:

$$ \boxed{ H\approx10.467\text{ m}. } $$

Under the chosen CoC threshold, focusing at roughly 10.47 metres makes infinity just acceptably sharp according to the model.

### Derive near and far DoF limits

Use focus distance $S$, aperture f-number $N$, focal length $f$, and acceptable CoC $c$.

For a point in front of the focus plane:

$$ s\<S, $$

the CoC formula becomes:

$$ c= \frac{f^2(S-s)}{Ns(S-f)}. $$

Using:

$$ f^2=Nc(H-f), $$

we can write:

$$ 1= \frac{(H-f)(S-s)}{s(S-f)}. $$

Rearrange:

$$ s(S-f)=(H-f)(S-s). $$

Collect the terms in the focus distance directly; this avoids mixing the hyperfocal and object-distance conventions.

$$ s(S-f)+(H-f)s=(H-f)S. $$

Factor $s$:

$$ s[(S-f)+(H-f)]=(H-f)S. $$

Thus the near limit is:

$$ \boxed{ D_{\mathrm{near}} = \frac{S(H-f)}{H+S-2f}. } $$

However, photographic references often define the hyperfocal distance using a slightly different approximation or reference distance convention, which produces the familiar expressions:

$$ \boxed{ D_{\mathrm{near}} = \frac{HS}{H+(S-f)}, } $$

$$ \boxed{ D_{\mathrm{far}} = \frac{HS}{H-(S-f)}. } $$

To avoid confusing conventions, we should derive the near/far limits consistently from one chosen definition of $H$.

With our exact definition:

$$ H=\frac{f^2}{Nc}+f, $$

substitute directly into the original CoC equation.

For $s\<S$:

$$ cNs(S-f)=f^2(S-s). $$

Collect terms in $s$:

$$ s[cN(S-f)+f^2]=f^2S. $$

Therefore:

$$ \boxed{ D_{\mathrm{near}} = \frac{f^2S}{f^2+Nc(S-f)}. } $$

Since:

$$ f^2=Nc(H-f), $$

we obtain:

$$ \boxed{ D_{\mathrm{near}} = \frac{S(H-f)}{H+S-2f}. } $$

For $s>S$:

$$ cNs(S-f)=f^2(s-S). $$

Rearrange:

$$ s[f^2-Nc(S-f)]=f^2S. $$

Therefore:

$$ \boxed{ D_{\mathrm{far}} = \frac{f^2S}{f^2-Nc(S-f)} } $$

when the denominator is positive.

Substitute the same hyperfocal definition:

$$ \boxed{ D_{\mathrm{far}} = \frac{S(H-f)}{H-S} } $$

for the chosen idealized physical-distance and sensor-plane convention, provided $H>S$.

For practical focus distances much larger than focal length, these differences of order $f$ are small, and the commonly published formulas converge closely. The exact expressions should be kept internally consistent with the definition of distance and acceptable CoC.

The important engineering lesson is that near/far DoF formulas depend on clearly defined geometric reference planes and approximations. Blindly mixing the hyperfocal equation from one convention with near/far formulas from another introduces small systematic discrepancies.

### What happens beyond the hyperfocal focus distance?

For sufficiently distant focus settings under the model, the far acceptable-focus boundary extends to infinity.

This does not mean infinite-depth objects have exactly zero defocus.

It means the predicted CoC is smaller than the chosen acceptable threshold.

### What changes DoF?

Holding the other camera parameters and the evaluation criterion fixed:

A larger aperture diameter generally increases defocus away from the focus plane. A longer focal length can increase defocus under fixed camera positioning and comparable sensor criteria. A nearer focus distance generally makes the acceptable sharpness interval narrower. A larger acceptable CoC threshold makes the reported DoF range larger.

But statements such as “longer lenses always have shallower depth of field” require carefully specifying what remains fixed. Matching framing by changing camera distance changes the geometry and can complicate simple comparisons.

### Depth of field as a rendering integral

A defocused pixel is not generated by one ray. It integrates contributions from a distribution of ray origins across the aperture.

For a lens sample coordinate $\mathbf a$, the scene radiance is:

$$ L(\mathbf a). $$

A simplified aperture-averaged pixel is:

$$ \boxed{ C= \frac1{A}\int_{\mathcal A}L(\mathbf a)\,dA. } $$

A Monte Carlo estimator with uniform aperture samples is:

$$ \boxed{ \widehat C= \frac1N\sum_{i=1}^{N}L(\mathbf a_i). } $$

This estimates the normalized aperture average. A physically scaled exposure calculation must also account for aperture throughput, integration time, radiometric weighting, and the camera model.

This distinction prevents a common bug: increasing aperture radius may change blur but not brightness if the renderer always averages lens samples without a physical throughput factor.


The thin-lens model extends the pinhole camera by allowing rays to originate from points across a finite aperture while ensuring that rays associated with a given image sample converge on the desired focus plane.

PBRT describes this construction directly: select a point on the lens, find the corresponding focus-plane point for the pinhole ray, then form a new ray from the sampled lens position toward that focus point.&#x20;

[image](https://www.google.com/s2/favicons?domain=https://www.pbr-book.org\&sz=32)

pbr-book.org



### Geometry of the camera

Choose the following right-handed camera-local convention:

$$ +X\text{ right},\qquad+Y\text{ up},\qquad-Z\text{ forward}. $$

Place the idealized lens plane at:

$$ z=0. $$

Let the focus plane be:

$$ \boxed{z=-S} $$

where $S>0$ is the axial focus distance.

Let the vertical FOV be:

$$ \theta_y $$

and aspect ratio be $a$.

A raster sample has NDC coordinates:

$$ (x_n,y_n). $$

### Step 1: Construct the pinhole viewing direction

Define:

$$ t_y=\tan(\theta_y/2). $$

The unnormalized pinhole direction is:

$$ \boxed{ \mathbf q= \begin{bmatrix} a\,t_yx_n\\ t_yy_n\\ -1 \end{bmatrix}. } $$

We intentionally leave it unnormalized for the moment.

Why?

Because its forward component is exactly $-1$. That makes intersecting the focus plane easy.

### Step 2: Intersect the pinhole ray with the focus plane

The pinhole ray is:

$$ \mathbf p(\lambda)=\lambda\mathbf q. $$

Its Z coordinate is:

$$ z(\lambda)=-\lambda. $$

The focus plane is:

$$ z=-S. $$

Thus:

$$ -\lambda=-S $$

and:

$$ \lambda=S. $$

Therefore the focus point is:

$$ \boxed{ \mathbf F=S\mathbf q. } $$

Expanded:

$$ \boxed{ \mathbf F= \begin{bmatrix} Sa t_yx_n\\ St_yy_n\\ -S \end{bmatrix}. } $$

### Step 3: Sample the aperture

Let the aperture radius be:

$$ R=\frac D2. $$

Generate independent uniform variables:

$$ u_1,u_2\in[0,1). $$

Sample the disk:

$$ r=R\sqrt{u_1}, $$

$$ \phi=2\pi u_2. $$

The aperture position is:

$$ \boxed{ \mathbf A= \begin{bmatrix} r\cos\phi\\ r\sin\phi\\ 0 \end{bmatrix}. } $$

### Step 4: Construct the new ray

The ray starts at the sampled aperture point:

$$ \boxed{ \mathbf o=\mathbf A. } $$

It must travel through the focus point $\mathbf F$.

Thus its unnormalized direction is:

$$ \mathbf F-\mathbf A. $$

Normalize:

$$ \boxed{ \hat{\mathbf d} = \frac{\mathbf F-\mathbf A} {\|\mathbf F-\mathbf A\|}. } $$

The resulting ray is:

$$ \boxed{ \mathbf r(t)= \mathbf A+t\hat{\mathbf d}. } $$

### Why all aperture rays agree at the focus plane

For the same raster sample $(x_n,y_n)$, the focus point $\mathbf F$ is fixed.

Different aperture samples produce different origins $\mathbf A_i$, but their directions are constructed toward the same $\mathbf F$.

Therefore:

$$ \boxed{ \mathbf F \in\mathbf r_i } $$

for every aperture sample.

An object exactly on the focus plane is sampled at the same surface location regardless of the chosen lens point, ignoring visibility and optical complications.

An object away from that plane is intersected at different positions by different lens rays.

That variation produces defocus blur.

### A simple numerical example

Let the raster sample be the image centre:

$$ x_n=y_n=0. $$

Then:

$$ \mathbf q=(0,0,-1). $$

Suppose the focus distance is:

$$ S=3\text{ m}. $$

Thus:

$$ \boxed{ \mathbf F=(0,0,-3). } $$

Take two aperture samples:

$$ \mathbf A_1=(0.01,0,0), $$

$$ \mathbf A_2=(-0.01,0,0) $$

in metres.

Their unnormalized ray directions are:

$$ \mathbf d_1=(-0.01,0,-3), $$

$$ \mathbf d_2=(0.01,0,-3). $$

The rays are different, but both pass through:

$$ (0,0,-3). $$

Now consider a plane at:

$$ z=-6. $$

The two rays do not intersect it at the same X coordinate. Their X separation represents the aperture-induced defocus footprint at that depth.

This can be tested analytically.

### Step 5: Transform lens rays into world space

Let the camera world position be:

$$ \mathbf c. $$

Let the camera basis vectors be:

$$ \mathbf r,\ \mathbf u,\ \mathbf f $$

where $\mathbf f$ points forward.

A local aperture point:

$$ (A_x,A_y,0) $$

maps to world origin:

$$ \boxed{ \mathbf o_W= \mathbf c+A_x\mathbf r+A_y\mathbf u. } $$

The focus point maps to:

$$ \boxed{ \mathbf F_W= \mathbf c+ F_x\mathbf r+ F_y\mathbf u- F_z\mathbf f } $$

when $\mathbf F$ is expressed using our local $-Z$-forward coordinates and the final term is interpreted consistently. Since $F_z=-S$, a clearer direct expression is:

$$ \boxed{ \mathbf F_W= \mathbf c+ Sa t_yx_n\mathbf r+ St_yy_n\mathbf u+ S\mathbf f. } $$

The world ray direction is:

$$ \boxed{ \hat{\mathbf d}_W= \operatorname{normalize}(\mathbf F_W-\mathbf o_W). } $$

### C++23 implementation

This implementation uses explicit camera basis vectors rather than relying on a hidden matrix convention:

```
struct Ray {
    Vec3d origin;
    Vec3d direction;
};

Ray generateThinLensRay(
    const CameraBasis& camera,
    double ndcX,
    double ndcY,
    double verticalFovRadians,
    double aspectRatio,
    double focusDistance,
    double apertureRadius,
    double randomRadius,
    double randomAngle
) {
    if (!(focusDistance > 0.0) ||
        !(apertureRadius >= 0.0)) {
        throw std::invalid_argument("Invalid lens parameters");
    }

    const double tanHalfFov =
        std::tan(verticalFovRadians * 0.5);

    // This point is on the focal plane at axial
    // forward distance focusDistance.
    const Vec3d focusPoint =
        camera.position +
        focusDistance * (
            camera.forward +
            ndcX * aspectRatio * tanHalfFov * camera.right +
            ndcY * tanHalfFov * camera.up
        );

    // Sample the lens disk uniformly by area.
    const double radius =
        apertureRadius * std::sqrt(randomRadius);

    const double phi =
        2.0 * std::numbers::pi * randomAngle;

    const double lensX = radius * std::cos(phi);
    const double lensY = radius * std::sin(phi);

    const Vec3d lensPoint =
        camera.position +
        lensX * camera.right +
        lensY * camera.up;

    return {
        .origin = lensPoint,
        .direction = normalize(focusPoint - lensPoint)
    };
}
```

The exact source file linked later contains a complete standalone implementation with concrete vector and matrix types.

### Pinhole limit

If aperture radius tends to zero:

$$ R\to0, $$

then every aperture sample converges to the camera centre:

$$ \mathbf A\to\mathbf0. $$

The thin-lens ray becomes the ordinary pinhole ray:

$$ \boxed{ \hat{\mathbf d} \to \operatorname{normalize}(\mathbf F). } $$

Thus the idealized pinhole model is the zero-aperture geometric limit of this thin-lens ray construction.

However, a physically real zero-area aperture would transmit no light. The mathematical pinhole camera ignores throughput limitations.

### Focal length versus focus distance

In our renderer implementation, `focusDistance` specifies the plane where rays converge.

The physical `focalLength` controls field of view in relation to sensor dimensions and aperture diameter through the f-number.

These are different parameters.

For a physically calibrated thin-lens camera, focus breathing and changing image distance can modify the effective framing. A simple computer-graphics thin-lens model often assumes a fixed projective image plane and independently configurable focus distance.

That approximation is useful, but should not be confused with simulating the entire optical system.

### Lens and shutter sampling together

A complete ray sample can contain:

$$ \boxed{ (\text{pixel position}, \text{lens position}, \text{shutter time}) }. $$

For example:

```
struct CameraSample {
    Vec2d rasterPosition;
    Vec2d lensSample;
    double shutterSample;
};
```

The camera maps this sample to a world-space ray with an origin, direction, and time.

A ray tracer then evaluates scene geometry at the sampled time.

If the scene is dynamic, different aperture and shutter samples may encounter different surfaces or lighting.

This is where spatial antialiasing, depth of field, and motion blur become parts of one multidimensional sampling problem.



### Figure C11-5 — Focus distance, focal length and aperture are different controls

![[Visual Recall/Math Foundations/C11-5.png]]

Paraxial 50 mm lens at f/2 focused at 2 m: 1/F=1/u+1/v and c=D|v0/v−1|, with D=F/N. The 0.03 mm threshold is an explicitly chosen viewing criterion. This excludes diffraction, aberrations and sensor processing.


<!-- end V11.17 -->

<!-- begin V11.07 -->
## A camera image changes over time

*Temporal jitter, previous-frame reprojection, motion vectors and sampling semantics.*

A real renderer produces a sequence of images. Projection jitter and camera motion must be distinguished when frames share history.

**Camera jitter for temporal sampling**

Ordinary rasterization samples geometry at fixed pixel locations. Temporal antialiasing (TAA) deliberately changes the subpixel sampling positions from frame to frame, then attempts to combine information across time.

The important idea is that camera jitter changes where the scene is sampled, not where the physical camera is located.

Suppose a pixel has integer coordinates $(i,j)$. Its conventional centre is:

$$
 (i+0.5,j+0.5). 
$$

If we want a sample shifted by:

$$
 (\delta_x,\delta_y) 
$$

measured in pixels, its sampling position becomes:

$$
 \boxed{ (i+0.5+\delta_x,\ j+0.5+\delta_y) }. 
$$

The shifts are typically subpixel quantities.

### Why different samples reveal different information

Imagine a thin diagonal line passing through a pixel.

A sample at the pixel centre might miss it.

A sample shifted toward one corner may intersect it.

Across frames, different subpixel offsets can sample different parts of the underlying image.

If we can correctly associate samples with the same surface through camera and object motion, temporal reconstruction can combine these observations to produce a better approximation of spatial coverage.

This connects camera sampling to Monte Carlo and stratified sampling methods.

However, the samples are not necessarily independent, and temporal reconstruction involves filters, rejection rules, and a changing scene.

A TAA image is not automatically an unbiased Monte Carlo estimate.

### A simple two-frame jitter example

Suppose a camera renders at:

$$
 W=1920,\qquad H=1080. 
$$

On frame 0, use jitter:

$$
 \boldsymbol\delta_0=(+0.25,-0.25) 
$$

pixels.

On frame 1, use:

$$
 \boldsymbol\delta_1=(-0.25,+0.25). 
$$

The projected scene shifts relative to the pixel grid differently on the two frames.

A stationary edge may therefore cover a different part of a pixel in each image.

The history-reconstruction algorithm attempts to combine the complementary samples.

But if the camera or object moves, we cannot simply average the same pixel index from consecutive frames.

We need motion information.

### Jitter sequences

Useful jitter sequences include stratified patterns, Halton sequences, and scrambled low-discrepancy patterns.

For example, using Halton bases two and three:

$$
 \mathbf h_k=(\phi_2(k),\phi_3(k)), 
$$

we can centre the result:

$$
 \boxed{ \boldsymbol\delta_k= \mathbf h_k-(0.5,0.5) }. 
$$

That produces subpixel offsets in approximately:

$$
 [-0.5,0.5)^2. 
$$

The precise sequence length and indexing policy matter.

Cycling through a small periodic jitter set can introduce visible repetition or resonance with scene motion.

A poor sequence can create structured ghosting or incomplete coverage.

### Jitter amplitude

Jitter is usually specified in pixels, not world units.

The corresponding NDC displacement depends on framebuffer dimensions.

This is why rendering resolution, dynamic-resolution scaling, and viewport changes must be handled explicitly.

A quarter-pixel shift at 1920×1080 corresponds to a different NDC shift than a quarter-pixel shift at 3840×2160.

The pixel-space shift is the same relative to the current raster grid, but the normalized displacement differs.

### The geometry of temporal error

For a surface point with world position $\mathbf p_W$, jittered projection is:

$$
 \mathbf p_{C,j}=P_jV\mathbf p_W. 
$$

If $P_j$ changes, its raster position changes even when $V$ and $\mathbf p_W$ are constant.

Thus projected motion contains two possible contributions:

1. Real geometry/camera motion.
2. Deliberate projection jitter.

A motion-vector convention must say whether its output includes or excludes the jitter contribution.

Mixing conventions across render passes is a major source of TAA instability.

**Jittered projection matrices**

We now derive exactly how a subpixel offset modifies a projection matrix.

Let the original clip coordinates be:

$$
 \mathbf p_c=P\mathbf p_v. 
$$

We want the new NDC coordinates to be:

$$
 \boxed{ x_n'=x_n+j_x, \qquad y_n'=y_n+j_y } 
$$

where $j_x,j_y$ are NDC offsets.

Since:

$$
 x_n=\frac{x_c}{w_c}, 
$$

we require:

$$
 \frac{x_c'}{w_c}= \frac{x_c}{w_c}+j_x. 
$$

Multiply by $w_c$:

$$
 \boxed{x_c'=x_c+j_xw_c}. 
$$

Similarly:

$$
 \boxed{y_c'=y_c+j_yw_c}. 
$$

The depth and $w$ coordinates remain unchanged.

### Matrix-row derivation

Let the projection matrix rows be:

$$
 \mathbf r_0,\mathbf r_1,\mathbf r_2,\mathbf r_3. 
$$

They produce:

$$
 x_c=\mathbf r_0\mathbf p_v, 
$$

$$
 y_c=\mathbf r_1\mathbf p_v, 
$$

$$
 w_c=\mathbf r_3\mathbf p_v. 
$$

To obtain:

$$
 x_c'=x_c+j_xw_c, 
$$

the first row must become:

$$
 \boxed{ \mathbf r_0'=\mathbf r_0+j_x\mathbf r_3 }. 
$$

Similarly:

$$
 \boxed{ \mathbf r_1'=\mathbf r_1+j_y\mathbf r_3 }. 
$$

Therefore the most general row-based jitter operation is:

$$
 \boxed{ P_j= \begin{bmatrix} \mathbf r_0+j_x\mathbf r_3\\ \mathbf r_1+j_y\mathbf r_3\\ \mathbf r_2\\ \mathbf r_3 \end{bmatrix} }. 
$$

This works for our column-vector convention and also describes the operation for general perspective matrices with nonstandard off-axis terms.

### Convert pixel jitter to NDC jitter

For a top-left raster image whose NDC $Y$ increases upward:

$$
 x_p=\frac{W}{2}(x_n+1), 
$$

$$
 y_p=\frac{H}{2}(1-y_n). 
$$

Differentiate:

$$
 \Delta x_p=\frac W2\Delta x_n. 
$$

Thus:

$$
 \boxed{ j_x=\Delta x_n=\frac{2\delta_x}{W} }. 
$$

For Y:

$$
 \Delta y_p=-\frac H2\Delta y_n. 
$$

Therefore:

$$
 \boxed{ j_y=-\frac{2\delta_y}{H} }. 
$$

The negative sign follows from the chosen top-left raster mapping.

### Numerical example · V11.07.09

For:

$$
 W=1920,\qquad H=1080, 
$$

and jitter:

$$
 \delta_x=0.25,\qquad\delta_y=-0.5, 
$$

we get:

$$
 j_x=\frac{0.5}{1920} =\boxed{0.000260416667}. 
$$

And:

$$
 j_y=-\frac{-1}{1080} =\boxed{0.000925925926}. 
$$

If the original raster position is:

$$
 (x_p,y_p)=(960,540), 
$$

the new projected position is:

$$
 \boxed{(960.25,539.5)}. 
$$

The reference C++23 laboratory verifies this exact shift numerically.

### How this changes a conventional perspective matrix

For our conventional right-handed perspective matrix:

$$
 \mathbf r_3=(0,0,-1,0). 
$$

Thus:

$$
 \mathbf r_0'= (s_x,0,-j_x,0) 
$$

and:

$$
 \mathbf r_1'= (0,s_y,-j_y,0). 
$$

The jitter appears in the matrix entries:

$$
 \boxed{ P_{0,2}=-j_x, \qquad P_{1,2}=-j_y } 
$$

under the standard centred projection and our convention.

But this shortcut depends on the specific projection matrix layout.

The general row update is safer and more expressive.

### Jittered versus unjittered projection

A production TAA renderer commonly maintains both:

$$
 P_{\mathrm{jittered}} 
$$

and:

$$
 P_{\mathrm{unjittered}}. 
$$

The jittered projection is used to generate spatially shifted raster samples.

The unjittered projection may be used for stable culling, certain motion-vector conventions, picking, and other operations that should not change with temporal sample offsets.

However, the exact motion-vector choice depends on how the temporal reconstruction algorithm accounts for jitter.

If culling uses an unjittered frustum, conservative margins may be needed to account for jittered raster coverage near the edges.

A robust implementation should define this relationship rather than relying on a tiny jitter being harmless.

**Previous-frame camera transforms**

Temporal reconstruction requires information about where the current visible surface appeared in the previous frame.

Let:

$$
 V_t,\ P_t 
$$

be the current view and projection matrices.

Let:

$$
 V_{t-1},\ P_{t-1} 
$$

be their previous-frame counterparts.

Define:

$$
 \boxed{ C_t=P_tV_t } 
$$

and:

$$
 \boxed{ C_{t-1}=P_{t-1}V_{t-1} }. 
$$

For a static world-space point:

$$
 \mathbf p_W, 
$$

its current clip position is:

$$
 \boxed{ \mathbf c_t=C_t\tilde{\mathbf p}_W }. 
$$

Its previous clip position is:

$$
 \boxed{ \mathbf c_{t-1}=C_{t-1}\tilde{\mathbf p}_W }. 
$$

After dividing by their respective $w$ coordinates, we obtain current and previous NDC positions.

### Deriving reprojection for a static world point

Suppose we know the current homogeneous view-space point:

$$
 \tilde{\mathbf p}_{V,t}. 
$$

The current view matrix satisfies:

$$
 \tilde{\mathbf p}_{V,t} = V_t\tilde{\mathbf p}_W. 
$$

Therefore:

$$
 \tilde{\mathbf p}_W= V_t^{-1}\tilde{\mathbf p}_{V,t}. 
$$

The previous clip position is:

$$
 \tilde{\mathbf c}_{t-1} = P_{t-1}V_{t-1}\tilde{\mathbf p}_W. 
$$

Substitute:

$$
 \boxed{ \tilde{\mathbf c}_{t-1} = P_{t-1}V_{t-1}V_t^{-1} \tilde{\mathbf p}_{V,t} }. 
$$

This is a camera-only reprojection transformation.

It is valid for a stationary world point and consistent coordinate frames.

If the point is moving, its previous world position must also be considered.

### Camera-only reprojection from current clip space

If the current clip coordinate is:

$$
 \tilde{\mathbf c}_t, 
$$

then:

$$
 \tilde{\mathbf p}_W = (P_tV_t)^{-1}\tilde{\mathbf c}_t. 
$$

Thus:

$$
 \boxed{ \tilde{\mathbf c}_{t-1} = P_{t-1}V_{t-1} (P_tV_t)^{-1} \tilde{\mathbf c}_t }. 
$$

This expresses how a homogeneous current clip-space point can be mapped to the previous clip frame for static geometry, subject to the relevant inverses and consistent homogeneous scaling.

In screen-space passes, a current NDC position and depth value can be lifted into homogeneous form and processed through the same relationship.

### Previous model matrices

For a moving rigid object:

$$
 \mathbf p_{W,t}=M_t\mathbf p_L. 
$$

Its previous world position is:

$$
 \boxed{ \mathbf p_{W,t-1}=M_{t-1}\mathbf p_L }. 
$$

Therefore:

$$
 \boxed{ \mathbf c_t=P_tV_tM_t\tilde{\mathbf p}_L } 
$$

and:

$$
 \boxed{ \mathbf c_{t-1}= P_{t-1}V_{t-1}M_{t-1}\tilde{\mathbf p}_L }. 
$$

If an engine uses only previous camera matrices but ignores object motion, moving objects may have incorrect motion vectors.

### Deformation is another case

For skinned or deforming geometry, the current and previous local vertex positions may differ.

Then:

$$
 \boxed{ \mathbf c_t=P_tV_tM_t\tilde{\mathbf p}_{L,t} } 
$$

and:

$$
 \boxed{ \mathbf c_{t-1}= P_{t-1}V_{t-1}M_{t-1}\tilde{\mathbf p}_{L,t-1} }. 
$$

The previous vertex state may depend on previous skeletal joint transforms, morph weights, cloth states, or other deformation data.

A previous model matrix alone is insufficient for nonrigid deformation.

### Camera cuts and history validity

Suppose the camera instantly teleports to a new viewpoint.

The previous image may contain little useful information for the new frame.

Blindly reprojecting and accumulating it can produce severe ghosting.

A temporal renderer should explicitly detect or mark camera cuts, invalid history, resized render targets, projection changes, and other discontinuities.

Keeping previous matrices is necessary for temporal rendering, but it is not sufficient for robust history management.

**Motion from camera movement**

A motion vector describes a displacement between corresponding projected positions across frames.

The convention must be declared.

One common TAA convention defines a backward motion vector:

$$
 \boxed{ \mathbf v= \mathbf p_{\mathrm{previous,raster}} - \mathbf p_{\mathrm{current,raster}} }. 
$$

Then the previous sampling position is:

$$
 \boxed{ \mathbf p_{\mathrm{previous}} = \mathbf p_{\mathrm{current}}+\mathbf v }. 
$$

Other pipelines store the opposite sign. Both are valid if consistently implemented.

### Derive camera-translation screen motion

Take a stationary world point:

$$
 \mathbf p_W=(0,0,-5) 
$$

and a camera initially at the origin, looking toward $-Z$.

Assume:

$$
 \theta_y=90^\circ,\quad a=1. 
$$

The horizontal projection is:

$$
 x_n=\frac{x_v}{d} 
$$

because $s_x=1$.

Initially:

$$
 x_v=0,\quad d=5. 
$$

Thus:

$$
 x_{n,0}=0. 
$$

Now move the camera right by:

$$
 \Delta c_x=0.2. 
$$

The stationary world's camera-relative X becomes:

$$
 x_{v,1}=0-0.2=-0.2. 
$$

Thus:

$$
 x_{n,1}=\frac{-0.2}{5}=-0.04. 
$$

For a 1920-pixel-wide viewport:

$$
 \Delta x_p=\frac{1920}{2}(-0.04) =-38.4. 
$$

Therefore the point appears to move left by:

$$
 \boxed{38.4\text{ pixels}}. 
$$

Under the previous-minus-current convention:

$$
 v_x=+38.4. 
$$

The compiled laboratory reproduces this result.

### NDC motion versus pixel motion

Let current and previous NDC positions be:

$$
 \mathbf n_t,\quad\mathbf n_{t-1}. 
$$

An NDC motion vector is:

$$
 \Delta\mathbf n=\mathbf n_{t-1}-\mathbf n_t. 
$$

For a top-left raster mapping, the pixel-space displacement is:

$$
 \boxed{ v_x=\frac W2\Delta n_x } 
$$

and:

$$
 \boxed{ v_y=-\frac H2\Delta n_y }. 
$$

Do not assume that a motion vector stored in NDC can be added directly to a pixel coordinate.

Its units and sign must be defined.

### Jitter contribution to motion vectors

Let:

$$
 \mathbf n_t^{(j)}= \mathbf n_t^{(0)}+\mathbf j_t 
$$

and:

$$
 \mathbf n_{t-1}^{(j)}= \mathbf n_{t-1}^{(0)}+\mathbf j_{t-1}. 
$$

Then the jittered NDC displacement is:

$$
 \begin{aligned} \Delta\mathbf n^{(j)} &= \mathbf n_{t-1}^{(j)}-\mathbf n_t^{(j)}\\ &= \mathbf n_{t-1}^{(0)}-\mathbf n_t^{(0)} +\mathbf j_{t-1}-\mathbf j_t. \end{aligned} 
$$

Therefore:

$$
 \boxed{ \Delta\mathbf n^{(j)} = \Delta\mathbf n^{(0)} + (\mathbf j_{t-1}-\mathbf j_t) }. 
$$

The motion vector includes the difference between temporal sample offsets if jittered positions are used directly.

A TAA implementation may prefer to store unjittered geometric motion and handle jitter separately during reprojection.

Another implementation may store jitter-inclusive motion.

The critical issue is consistency.

### Translation, rotation, and parallax

Camera translation causes depth-dependent screen displacement.

Near objects move more strongly than distant objects under the same translation.

Camera rotation produces image motion that can occur even for extremely distant objects.

This distinction is important in XR.

For rotational head motion, a sufficiently distant scene behaves approximately like an environment mapped by orientation changes.

For translation, finite scene depth creates parallax.

A camera-motion model that accounts only for rotation may be useful for distant content but cannot correctly reproduce translation parallax for nearby objects.

### Motion vectors are not optical flow

A geometric motion vector maps a corresponding surface point from its current image position to its previous image position using known transforms and geometry.

Optical flow is usually estimated from image intensities and attempts to infer apparent image motion.

The two are related but not identical.

Optical flow may represent changes due to reflections, lighting, transparency, shadows, and image patterns that do not correspond to a single rigid 3D surface motion.

For reflective or transparent materials, geometric motion vectors can be insufficient to reconstruct the apparent movement of radiance.

That is one reason TAA history rejection and specialized motion handling remain necessary.


<!-- end V11.07 -->

<!-- begin V11.08 -->
## Large worlds need local arithmetic

*Camera-relative rendering, float spacing, origin rebasing, double-precision pose.*

Temporal transforms become fragile when absolute positions overwhelm floating-point precision. Local subtraction keeps the relevant displacement representable.

**Camera-relative rendering**

Camera-relative rendering represents geometry in a local coordinate system near the active camera.

The motivation is numerical precision.

Let a world-space point be:

$$
 \mathbf p_W. 
$$

Let the camera world position be:

$$
 \mathbf c_W. 
$$

Define the camera-relative vector:

$$
 \boxed{ \mathbf p_R=\mathbf p_W-\mathbf c_W }. 
$$

The camera orientation then transforms it into view space:

$$
 \boxed{ \mathbf p_V=R^T\mathbf p_R }. 
$$

This is mathematically equivalent to applying the inverse rigid camera transform.

The important numerical difference is where and in which precision the subtraction occurs.

### Bad order of operations

Suppose the world position and camera position are large binary64 values.

A poor implementation converts each to binary32 first:

```
Vec3f pointFloat  = castFloat(worldPoint);
Vec3f cameraFloat = castFloat(cameraWorldPosition);

Vec3f relative = pointFloat - cameraFloat;
```

If the positions are near $10^8$, small differences may disappear before subtraction.

No later operation can reconstruct them.

### Better order

Compute the relative displacement in double precision:

```
Vec3d relativeDouble =
    worldPointDouble - cameraWorldPositionDouble;

Vec3f relativeFloat =
    castFloat(relativeDouble);
```

Now the difference is computed while both large coordinates still retain sufficient precision.

The much smaller relative vector is then converted to binary32.

### Numerical demonstration

Let:

$$
 x_W=100000000.125, 
$$

$$
 c_W=100000000. 
$$

Their exact difference is:

$$
 \boxed{x_R=0.125}. 
$$

Binary32 cannot distinguish these two values at that magnitude.

If we first convert both to `float`, they can round to the same representable value:

$$
 \operatorname{float}(x_W) = \operatorname{float}(c_W). 
$$

Then:

$$
 \boxed{ \operatorname{float}(x_W) - \operatorname{float}(c_W)=0 }. 
$$

But if we subtract in binary64 first:

$$
 x_W-c_W=0.125, 
$$

then convert:

$$
 \boxed{ \operatorname{float}(x_W-c_W)=0.125 }. 
$$

The compiled lab verifies exactly this difference.

### Object-relative vertices

For a mesh with local vertices $\mathbf p_L$, a world transform may be:

$$
 \mathbf p_W=A\mathbf p_L+\mathbf t_W. 
$$

Camera-relative position is:

$$
 \mathbf p_R= A\mathbf p_L+(\mathbf t_W-\mathbf c_W). 
$$

This suggests a practical division of work.

On the CPU, calculate:

$$
 \boxed{ \mathbf t_R=\mathbf t_W-\mathbf c_W } 
$$

in high precision.

Upload $\mathbf t_R$ in binary32 where appropriate.

Then the GPU computes:

$$
 \boxed{ \mathbf p_R=A_f\mathbf p_L+\mathbf t_{R,f} }. 
$$

This avoids constructing huge world coordinates in float and subtracting the camera afterward.

### Why camera-relative rendering matters for temporal history

Suppose the camera-relative origin changes every frame.

Current and previous positions might be expressed relative to different camera origins.

If a shader combines coordinates from different relative origins without applying the corresponding conversion, its motion vectors become incorrect.

A robust temporal renderer must preserve or reconstruct the relationship between the current and previous coordinate frames.

The camera-relative representation is an optimization of coordinate arithmetic, not permission to discard world-frame semantics.

**Large-world precision problems**

Binary floating-point precision is relative to the magnitude of the represented number.

For IEEE 754 binary32, normalized values have 24 significant binary digits.

The spacing between representable numbers grows as the exponent increases.

### Derive float spacing

For a normal binary32 value in the interval:

$$
 [2^e,2^{e+1}), 
$$

the spacing is:

$$
 \boxed{ \mathrm{ULP}(x)=2^{e-23} } 
$$

because the significand has 24 significant bits, including the implicit leading one.

`ULP` means unit in the last place.

At values around:

$$
 2^{10}=1024, 
$$

the spacing is:

$$
 2^{10-23}=2^{-13} \approx0.00012207. 
$$

At values around:

$$
 2^{20}=1{,}048{,}576, 
$$

the spacing is:

$$
 2^{20-23}=2^{-3}=0.125. 
$$

At values around:

$$
 2^{26}=67{,}108{,}864, 
$$

the spacing is:

$$
 2^{26-23}=8. 
$$

Thus near $10^8$, adjacent binary32 numbers can differ by eight units.

If those units are metres, the positional representation cannot resolve ordinary centimetre-scale movement there.

### Consequences in a graphics engine

Large-coordinate precision loss can affect vertex transforms, camera matrices, animation, world-space normal reconstruction, lighting calculations, collision detection, physics constraints, particle systems, shadow-map positioning, and ray intersections.

The exact severity depends on where rounding occurs.

A physics engine using double-precision positions can preserve fine detail, while a renderer using absolute float world positions may still jitter.

Conversely, fixing the renderer does not automatically fix a float-based physics subsystem.

### Difference between absolute and relative accuracy

Suppose an object is at world position:

$$
 10^8\text{ metres}. 
$$

Its distance from the world origin is huge.

But the camera may be only:

$$
 2\text{ metres} 
$$

away.

The quantity relevant to the view transform is the two-metre displacement.

Using large absolute float positions to calculate that small displacement wastes most of the available significand precision representing the shared large offset.

Camera-relative coordinates instead represent the meaningful local quantity directly.

### The limitation of merely switching matrix multiplication to double

Suppose vertex positions have already been stored as float world positions near $10^8$.

If they have lost their small local offsets, multiplying them by a double-precision matrix will not restore the lost information.

The high-precision representation must exist at the stage where meaningful differences are still available.

This is why large-world architecture is a data-model decision, not just a shader precision keyword.

Godot's large-world documentation describes related precision concerns and explains why even a double-precision engine may require special handling in its rendering path.&#x20;


**Origin rebasing**

Origin rebasing periodically changes the simulation's local coordinate origin so active objects remain near numerical zero.

Suppose world coordinates are represented relative to a current origin:

$$
 \mathbf O. 
$$

An object's local runtime coordinate is:

$$
 \boxed{ \mathbf p_R=\mathbf p_W-\mathbf O }. 
$$

If the active region moves far from $\mathbf O$, choose a new origin:

$$
 \mathbf O'=\mathbf O+\Delta. 
$$

To preserve the same geometric world position:

$$
 \mathbf p_R'=\mathbf p_W-\mathbf O'. 
$$

Substitute:

$$
 \mathbf p_R' = \mathbf p_W-(\mathbf O+\Delta). 
$$

Since:

$$
 \mathbf p_R=\mathbf p_W-\mathbf O, 
$$

we get:

$$
 \boxed{ \mathbf p_R'=\mathbf p_R-\Delta }. 
$$

Every local position must shift by the negative origin change.

### Numerical example · V11.08.11

Suppose the old runtime origin is:

$$
 \mathbf O=(0,0,0). 
$$

A player is located at:

$$
 \mathbf p_W=(120000,2,50000). 
$$

Choose a new origin:

$$
 \mathbf O'=(120000,0,50000). 
$$

The player's rebased position is:

$$
 \boxed{ \mathbf p_R'=(0,2,0) }. 
$$

The physical player has not moved.

Only the coordinate representation changed.

### What must be rebased?

Anything storing positions in the old runtime frame must either be transformed or be able to interpret the origin change.

This can include rigid bodies, collision geometry, particles, navigation data, spatial indices, light positions, decals, world-space effects, audio positions, camera transforms, and cached scene geometry.

Pure directions should not receive a translation.

Velocities also do not generally require subtracting the origin shift, because translating the coordinate origin by a constant does not change velocity. However, a moving or time-dependent origin introduces additional derivative terms if the coordinate frame itself changes continuously.

### Spatial acceleration structures

Suppose a BVH stores AABBs in a rebased coordinate frame.

When the origin changes, the boxes' coordinates change.

An engine must either update the relevant bounds, use a hierarchical origin representation, or maintain an acceleration structure whose coordinate interpretation includes the origin.

Rebasing scene objects without updating the spatial index can cause incorrect culling or collision queries.

### Temporal history problem

Suppose the previous frame used origin:

$$
 \mathbf O_{t-1} 
$$

and the current frame uses:

$$
 \mathbf O_t. 
$$

The same world point has coordinates:

$$
 \mathbf p_{R,t-1} = \mathbf p_W-\mathbf O_{t-1} 
$$

and:

$$
 \mathbf p_{R,t} = \mathbf p_W-\mathbf O_t. 
$$

Therefore:

$$
 \boxed{ \mathbf p_{R,t-1} = \mathbf p_{R,t} + (\mathbf O_t-\mathbf O_{t-1}) }. 
$$

That origin difference must be accounted for when reconstructing previous-frame positions.

Otherwise the rebasing event can appear as enormous false motion.

### Camera-relative rendering versus origin rebasing

These approaches are related but not identical.

Camera-relative rendering can convert positions near the camera for each render operation without changing the authoritative simulation coordinates.

Origin rebasing changes the local coordinate representation used by some or all runtime systems.

An engine may use one, the other, or both.

For research and deterministic replay, origin changes should be explicit events with recorded offsets.

**Double-precision camera transforms**

A large-world camera should preserve enough precision to represent its absolute position and orientation.

Double precision is often appropriate for global camera positions and scene-to-camera translation calculations.

### A double-precision rigid view transform

Let the camera have world position:

$$
 \mathbf c_d 
$$

stored in binary64.

Let its world-space rotation be:

$$
 R_d. 
$$

Then the exact mathematical view transform is:

$$
 \boxed{ \mathbf p_V=R_d^T(\mathbf p_W-\mathbf c_d) }. 
$$

A good implementation may perform the subtraction in double precision and convert the resulting relative position to float for GPU stages.

For example:

```
struct CameraWorldPose {
    Vec3d position;
    Quaterniond orientation;
};

Vec3f makeCameraRelativePosition(
    const Vec3d& objectWorldPosition,
    const CameraWorldPose& camera
) {
    // Important: subtract first, using binary64.
    const Vec3d relative =
        objectWorldPosition - camera.position;

    // Only now reduce precision for GPU-local calculations.
    return Vec3f{
        static_cast<float>(relative.x),
        static_cast<float>(relative.y),
        static_cast<float>(relative.z)
    };
}
```

This code assumes `Vec3d`, `Vec3f`, and `Quaterniond` have the obvious semantics; it illustrates the data-flow contract, not a complete engine-specific module.

### Why rotation also matters

Camera position is not the only source of precision error.

Very small orientation errors can cause noticeable screen-space shifts at large distances.

Suppose a point lies at distance $d$ and the camera orientation changes by a small angle:

$$
 \delta\theta. 
$$

For small angles, the transverse displacement of the viewing ray at that distance is approximately:

$$
 \boxed{ \delta x\approx d\,\delta\theta } 
$$

with $\delta\theta$ measured in radians.

At:

$$
 d=1000 
$$

and:

$$
 \delta\theta=10^{-5}, 
$$

the transverse displacement is approximately:

$$
 0.01 
$$

world units.

The resulting screen displacement depends on FOV and image resolution.

Thus orientation accuracy can matter substantially in long-range rendering and XR.

### Precision must be end-to-end

A double-precision camera position is valuable only if the relevant coordinate transformations preserve its information.

If object transforms are stored in absolute float coordinates and already quantized, the camera cannot reconstruct their missing local differences.

If shader code reconstructs large absolute world positions in float, it can reintroduce precision problems.

If temporal motion uses previous and current coordinate frames inconsistently, precision improvements can coexist with severe reprojection artifacts.

The architecture should therefore specify authoritative world coordinates, camera-relative conversion, GPU representation, and temporal transform semantics together.

### Repository connection: Maveb camera paths

Your inspected `Maveb/engine/scene/src/CameraPath.cpp` parses keyframe positions as `double`, but stores them in a transform whose translation is `simd_float3`.

It also stores rotation in a float quaternion.

This is coherent for a float-based scene system, but it is not an end-to-end double-precision camera-position path.

If Maveb needs very large world coordinates while preserving small local motion, that conversion point is important.

A future high-precision camera-path design could retain authoritative positions in double and construct renderer-relative float transforms only after subtracting the active camera or scene origin.

That recommendation follows from the source's actual conversion, not from assuming the current camera path is malfunctioning.

After treating the frustum as testable geometry, we can ask what kind of geometry perspective projection preserves.

An ordinary Euclidean transformation may preserve distances and angles. An affine transformation preserves straight lines and ratios along a line. A general projective transformation preserves straightness and incidence—whether points lie on lines or planes—but not ordinary lengths, angles, parallelism, or Euclidean ratios.

Understanding that hierarchy explains why perspective projection can be represented as a matrix in homogeneous coordinates, why parallel railroad tracks appear to converge, and why texture coordinates require perspective-correct interpolation.


<!-- end V11.08 -->


## calibration needs observable parameters and a fixed reference

“Observable” means that the available measurements can distinguish a parameter change from other parameter changes. In the Jacobian, two nearly identical columns mean two parameters produce nearly the same image effect. Their values are hard to separate, even if a numerical solver returns a value. Singular values expose this: a tiny singular value marks a direction in parameter space that barely changes predicted observations.

“Gauge freedom” is a precise version of unobservable choice. If every camera and every 3D point is translated and rotated together, all relative image measurements remain the same. In monocular reconstruction, scaling all scene coordinates and camera translations by the same positive factor also preserves projected pixels. Thus the images alone do not determine a unique world origin, orientation, or metric scale. Fix a gauge by choosing a reference camera pose, a coordinate frame, and (when scale is otherwise free) a baseline length or known object size. This pins down a representative solution without adding information to the images.

The distinction between gauge and poor measurement is useful: gauge is an exact symmetry of the model; weak observability means the data constrain a parameter only slightly. More varied viewing directions, known dimensions, noncoplanar points, and textured corners can improve observability. Repeating the same nearly front-facing view may produce many observations but little new information. A calibration target should cover the image and vary orientation so focal length, principal point, pose, and distortion do not imitate one another.

<!-- begin V11.23 -->
## Calibration needs informative observations and a fixed gauge

Camera calibration estimates parameters from image observations. A common parameter vector might include focal lengths, principal point, distortion coefficients, camera poses, and 3D target points. A reprojection residual for observation $i$ can be written

$$
r_i(\theta)=\pi(\theta, X_i)-u_i,
$$

where $\theta$ collects unknown camera and scene parameters, $X_i$ is a target point or landmark, $u_i$ is its measured image coordinate, and $\pi$ is the chosen camera projection model. Least squares minimizes $\sum_i \|r_i(\theta)\|^2$ (often with robust losses when detections may be outliers).

Having many observations does not guarantee that every parameter is recoverable. What matters locally is how the residuals change when parameters change. Linearizing at the current estimate gives

$$
r(\theta+\delta)\approx r(\theta)+J\delta,
$$

where $J=\partial r/\partial\theta$ is the Jacobian. If some nonzero parameter change $\delta$ satisfies $J\delta\approx0$, the image observations barely change in that direction. That direction is weakly observable or unobservable. The normal matrix $J^T W J$ then has a small eigenvalue; its inverse, when defined, gives a local covariance approximation. A small eigenvalue means large uncertainty along the corresponding parameter combination.

### Gauge freedom: the whole scene can move without changing its image

If both camera poses and all 3D points are unknown, applying the same global rigid transform to camera and points changes no relative camera-point geometry and therefore no image. In monocular structure from motion, a global scale change is also invisible: scale every camera translation and every reconstructed point by the same positive factor and projected image positions remain unchanged. These are **gauge freedoms**, not numerical bugs.

An optimizer cannot uniquely estimate parameters that the observations do not distinguish. Fix a reference camera pose and choose a scale constraint (for example, a known baseline length or a known target dimension). This removes arbitrary coordinate freedoms while preserving the image fit. If the world coordinate frame is already externally surveyed, that information supplies the anchoring constraint instead.

### Planar calibration and the danger of repetitive views

A planar checkerboard is a useful calibration target because a plane-to-image mapping is a homography. But repeated photographs with the board held at nearly the same orientation provide nearly redundant constraints. Parameters such as focal length and distance can compensate for one another: a small target moved farther away can produce a similar image size to a larger target moved closer. Tilting and translating the board across the field changes how the observations constrain focal length, principal point, and distortion.

Radial distortion is especially weakly constrained when all measured corners cluster near the image center. A distortion model predicts small displacement there, so many different distortion coefficients can fit the same central observations. Spread observations toward the image boundary and across both axes. Yet points at the extreme edge alone are not sufficient: include varied tilts and distances so distortion does not become confounded with pose or target warp.

### Small worked observability example

Consider a pinhole camera observing a target of known width $W$ at axial distance $Z$. In a simplified fronto-parallel setup, its image width in pixels is approximately

$$
w_{px}=f\frac{W}{Z},
$$

where $f$ is focal length expressed in pixel units. A single measured width determines only the ratio $f/Z$: doubling both $f$ and $Z$ leaves $w_{px}$ unchanged. Taking logarithms gives $\log w_{px}=\log f+\log W-\log Z$; the local derivative with respect to $(\log f,\log Z)$ is $(1,-1)$. The orthogonal parameter direction $(1,1)$ changes both together and has zero first-order effect on the observation. Add a known distance measurement, a second view with a known camera translation, or target views with useful perspective variation to break the ambiguity.

### Design calibration as an experiment

Before collecting more frames, ask which parameters each planned view will constrain. Vary target tilt around both horizontal and vertical axes; include observations near corners and edges; change distance and image position; avoid motion that repeats the same pose. Examine singular values or eigenvectors of the scaled Jacobian/normal matrix. Scaling matters: focal length in pixels, translations in metres, and distortion coefficients can differ greatly in numerical magnitude, so inspect a dimensionless or sensibly normalized parameterization.

The covariance approximation from $(J^T WJ)^{-1}$ is conditional on the local linear model, chosen noise weights, and correct model. It does not account automatically for wrong lens models, rolling-shutter timing errors, board nonplanarity, or correlated detector errors. Residual plots matter: systematic radial residuals suggest a lens-model mismatch; residual patterns by row may reveal rolling shutter or timing; pose-correlated residuals can signal a calibration setup degeneracy. Calibration quality should be reported with the dataset geometry and residual structure, not only one average pixel error.

### The solver does not erase ambiguity

The bundle-adjustment lecture makes the optimization layer explicit. Stack image residuals into $r(\theta)$, linearize around the current estimate, and solve a damped normal system. Gauss–Newton uses the local model $J^TJ\,\delta=-J^Tr$; Levenberg–Marquardt adds damping, commonly $\lambda I$ or a scaled diagonal, to make a more conservative step when the local quadratic model is unreliable. This is effective for reprojection fitting, but it remains a local method: initialization matters, nonlinear objectives can have multiple minima, and weakly constrained parameter combinations still produce ill-conditioned systems. A more sophisticated optimizer can handle the arithmetic of an ambiguity; it cannot make the observations contain information they do not contain. The lecture describes the reprojection objective, Gauss–Newton approximation, damping, and initialization limits. (V02)

This point can be seen in a second, related estimation problem: optical flow. A local brightness-constancy equation gives one scalar constraint on two flow components. Aggregating a patch yields a structure tensor $G=\sum_k \nabla I_k\nabla I_k^T$. A flat patch gives two tiny eigenvalues; a straight edge gives one strong and one weak eigenvalue; a corner with gradients in independent directions gives two strong eigenvalues. Merely checking $\det(G)\ne0$ is inadequate under noise: a determinant can be nonzero while its smaller eigenvalue is too small for a stable inverse. The TUM lecture derives this rank interpretation, discusses scale-dependent thresholds, and connects it to feature selection and Lucas–Kanade tracking. (V01)

The structure tensor is not itself the camera-calibration normal matrix, and optical-flow conditioning is not a proof of calibration observability. The shared lesson is narrower: inspect the spectrum and the constrained parameter directions, not only whether a determinant is mathematically nonzero. In calibration, use singular vectors of a scaled Jacobian; in patch flow, use the structure-tensor eigenvectors. Both reveal which motions or parameter combinations the measurements actually distinguish.

This section deepens the existing intrinsics/extrinsics and distortion material by explaining why a formally solvable optimization can remain ambiguous. It complements the projection equations without prescribing one universal calibration model. The bundle-adjustment video is used for its optimization mechanics; the observability discussion above is an original mathematical extension and is not attributed to that lecture as a stated theorem.


<!-- end V11.23 -->

<!-- begin V11.27 -->
## Calibrated two-view geometry: direction before distance

For calibrated cameras, write a scene point's normalized camera-image rays as homogeneous vectors $x_1$ and $x_2$. If the first camera frame is the reference, the second camera sees the point after rotation $R$ and translation $t$:

$$\lambda_1x_1=X,\qquad \lambda_2x_2=RX+t.$$

The unknown depths $\lambda_1,\lambda_2$ and 3D point $X$ make direct motion estimation appear circular: to recover pose we want structure, and to recover structure we want pose. The key is to ask what must be true even after the depths disappear. Cross multiplication with $[t]_\times$ removes the component parallel to the baseline; dotting with $x_2$ then removes the remaining scale. The resulting condition

$$x_2^T[t]_\times R x_1=0$$

says that $t$, $Rx_1$, and $x_2$ lie in one plane. The three vectors have zero signed volume. This is the epipolar constraint, and $E=[t]_\times R$ is the essential matrix. It uses only the two image measurements and relative pose; no point depth remains.

This elimination step also says what the equation cannot tell us. It identifies a plane of possible rays, but it does not choose a point within that plane. After pose is found, triangulation supplies depth by intersecting the rays. With noisy measurements the rays generally miss, so “intersection” becomes an estimation problem. Minimizing distance between the 3D rays is not generally the same as minimizing the image reprojection error; the latter asks how far the projected reconstruction lies from observed pixels.


<!-- end V11.27 -->

<!-- begin V11.28 -->
## Gauge freedom, singular values, and choosing a physical pose

The essential matrix has singular values $(\sigma,\sigma,0)$. The zero singular value records the rank-two constraint, while equal nonzero singular values encode the fact that $E$ factors into a skew translation matrix and a rotation. Its magnitude is arbitrary: multiplying $E$ by any nonzero scalar does not change the epipolar equation. In noisy data, estimate a general matrix by linear least squares, then use SVD to project it back onto this constrained family. Replacing its first two singular values by their average and the third by zero gives the nearest constrained matrix in Frobenius distance; setting the first two to one fixes a convenient normalization.

The eight-point method is a particularly transparent linear estimate. Each correspondence contributes one row to a matrix equation in the nine entries of $E$. For exactly eight generic pairs, the $8\times9$ design matrix typically has rank eight and a one-dimensional nullspace, even with noisy measurements. With more than eight noisy pairs, the design matrix is generally full column rank; use its right singular vector for the smallest singular value as the least-squares estimate. This is a convenient linear method, not a statement that eight is the absolute minimum for every solver. Five correspondences can yield a finite set of solutions through more involved constraints; for seven generic pairs the rank-seven nullspace is two-dimensional and can be restricted by the determinant condition, producing up to three solutions.

Decomposing an essential matrix produces four relative pose candidates: two rotations and two signs for the translation direction. A monocular pair does not reveal the length of the baseline. If a reconstruction is enlarged by a factor, the baseline and all scene depths can be enlarged together without changing either image. This is a gauge freedom, not a numerical bug. Choose a convenient baseline for visualization, or use an external measurement such as a known distance to establish metric scale.

The remaining finite ambiguity is resolved with cheirality. Triangulate the matched points for each candidate and check which candidate places the points in front of both camera planes. The correct relative pose should yield positive depths for the visible scene. Thus two distinct ambiguities need different remedies: cheirality selects among discrete pose candidates, while an external ruler fixes the continuous global scale. With pure rotation, $t=0$ and hence $E=0$; there is no translational parallax, so two-view triangulation cannot recover depth. A very small but nonzero baseline is theoretically different yet practically ill-conditioned: tiny pixel errors can create large depth changes.


<!-- end V11.28 -->

<!-- begin V11.29 -->
## Planes change the model

A planar scene is not just a set of ordinary 3D points with a convenient shape. It changes the mapping between images. Let a plane in camera-one coordinates satisfy $n^TX=d$ with $d\ne0$, and let $X_2=RX_1+t$. Assume neither camera centre lies on the plane, so the induced homography is nondegenerate. Since every point on the plane satisfies $(n^T/d)X_1=1$, substitute that identity into the pose equation:

$$X_2=\left(R+t\frac{n^T}{d}\right)X_1.$$

The matrix $H_n=R+t n^T/d$ maps normalized camera-image coordinates by $x_2\sim H_nx_1$; for pixel coordinates, use $H_{px}=K_2H_nK_1^{-1}$. The 3D point has disappeared; all points on the plane share one 3-by-3 homography. Four generic point pairs provide eight independent scalar constraints, enough to estimate the nine homography entries up to their arbitrary common scale.

This explains a practical degeneracy in the general essential-matrix method. If all matches lie on a wall, a book cover, or a road patch, adding more matches on the same plane does not create the missing out-of-plane evidence. A planar model may be the right model and can recover motion relative to the plane, but translation is tied to the unknown plane distance. A small nonplanar region or a known plane distance can add information; merely collecting more points on the same plane cannot manufacture it. When integrating homography formulas, state the pose direction and plane equation convention, since reversing either changes signs.


<!-- end V11.29 -->

<!-- begin V11.30 -->
## Why three views contribute information beyond pairs

In multiple views, a point's preimage in each image is a ray. A valid reconstruction is a common intersection of all rays, not just a collection of pairwise intersections. Pairwise constraints can miss a global inconsistency: camera centres arranged along a line can make every pair of viewing rays intersect while the three rays have no common point. If three camera centres and candidate back-projection rays lie in one plane, pairwise epipolar constraints can hold even when the rays meet at different pairwise points rather than one common point. A genuine point in that plane seen by all cameras still gives concurrent rays; joint concurrency is the requirement. These are geometric degeneracies; they explain why more measurements do not always mean more independent information.

A compact algebraic test stacks the projection equations for a point across images and eliminates the per-view depths. For image vector $x_i$, the skew matrix $[x_i]_\times$ annihilates that ray, so multiplying the projection block by it removes the unknown depth. The resulting matrix has the homogeneous 3D point in its nullspace. Rank three gives a unique point up to homogeneous scale; lower rank gives multiple possible points; full rank means no common point fits exactly. Geometrically, each image contributes two independent planes through its ray, and the reconstructed point must lie in their intersection.

For line features, each image line back-projects to a plane. Two planes always intersect (possibly at infinity), so two views of a line alone cannot impose a useful consistency test on camera motion. Three or more views can require all the back-projected planes to share one 3D line. The two independent vectors describing that line—a base point and a direction—must both lie in the joint nullspace. Hence a stacked line matrix must have rank at most two. This is a simple example of choosing features whose preimage geometry makes the constraint visible.

The familiar pairwise epipolar equations remain part of multi-view consistency, but they do not capture all of it. A multi-view rank condition also requires the depth scale for one point to agree across every view. Expanding this shared condition gives trilinear relations involving triples of images. Pairwise checks can therefore pass while a joint multi-view check fails. This gives a useful workflow: use pairwise geometry to initialize and diagnose matches, then use multi-view consistency to test whether one shared scene explains the complete track.

These distinctions guide method choice. Use an essential matrix when intrinsics are known and a general 3D scene provides parallax; use a homography when the evidence is planar; use multi-view rank or reprojection constraints when tracks span several cameras. Keep the gauge explicit, check discrete pose candidates with positive depth, and test the assumed scene model before interpreting a numerically precise answer as physical geometry.


<!-- end V11.30 -->

<!-- begin V11.24 -->
## A camera ray carries a pixel footprint

A pixel is not a mathematical point. It covers a finite region of the film or sensor. A camera ray through the pixel center says which direction that central sample travels; it says nothing by itself about how much the ray direction changes across the pixel. That change determines the footprint of the sample on surfaces and therefore which texture detail should be averaged.

Let the camera map film coordinates $(s,t)$ to a ray with origin $o(s,t)$ and direction $d(s,t)$. For a pinhole camera, $o$ is constant and $d$ changes with film position. Two neighboring rays give finite-difference estimates of the derivatives:

$$
\frac{\partial d}{\partial s}\approx \frac{d(s+\Delta s,t)-d(s,t)}{\Delta s},\qquad
\frac{\partial d}{\partial t}\approx \frac{d(s,t+\Delta t)-d(s,t)}{\Delta t}.
$$

PBRT's camera interface generates a main ray and differential rays offset by roughly one film pixel. Its texture-antialiasing path intersects those rays with a surface and estimates how the hit point changes across the image. This gives local surface derivatives such as $\partial p/\partial s$ and $\partial p/\partial t$; those can be mapped through surface parameterization into texture coordinates, yielding a finite texture footprint rather than a single lookup point. (E02, E03)

### Why this prevents shimmer

Suppose a checkerboard texture has alternating black and white squares much smaller than a projected pixel. A point lookup chooses one square. As the camera moves by a fraction of a pixel, the sample can jump from black to white, causing flicker. The correct pixel value approximates an integral over the pixel's image footprint. When that footprint spans many checker cells, the average tends toward gray. Texture filtering approximates this integral using a footprint-aware filter or mip level.

At a grazing angle, the projected pixel footprint on a surface is elongated: one screen direction may cover a much larger surface distance than the other. A single isotropic mip level based only on the larger derivative avoids aliasing but over-blurs the shorter direction. Anisotropic filters use both axes of the footprint to retain detail where sampling density is still high. The differential is local and first-order; it may fail at discontinuities, silhouettes, occlusion boundaries, or highly curved mappings. Neighbor rays can strike a different object or miss the central ray's surface, so implementations must handle invalid differentials and use conservative fallbacks.

### Sample spacing is part of the derivative

If a renderer takes $N$ samples per pixel, the random sample locations may be much closer than one pixel. PBRT's differential scaling adjusts the neighboring-ray offsets for an estimated sample spacing. Forgetting this scale can make the footprint too large and blur the result, or too small and reintroduce aliasing. The differential represents a derivative or local footprint estimate, not an additional physical ray that must contribute radiance as an independent Monte Carlo sample.

### The camera's probability measure must match its samples

In a simple perspective camera, uniformly sampling film area induces a nonuniform distribution over ray directions: directions near the image edge cover a different solid angle per unit film area than directions near the center. Those are distinct measures. If an algorithm changes variables from film area $dA_f$ to solid angle $d\omega$, it must include the appropriate Jacobian. For example, if $p_A$ is a density per unit film area, the corresponding directional density is $p_\omega=p_A\left|dA_f/d\omega\right|$; converting the other way uses the reciprocal. Likewise, a realistic lens may transmit different amounts of light for different rays; PBRT's camera interface can return a ray weight rather than assuming every generated ray contributes equally. Its path-space treatment interprets camera ray generation as importance sampling for the camera's measurement function. (E02, E04)

Keep three ideas separate: the ray maps a sample location to geometry; its differential estimates a neighborhood/footprint; its PDF and returned weight define how that sample estimates an integral. A ray differential is not itself the PDF, and a visually plausible ray generator is not automatically an unbiased estimator if the measure conversion is omitted.

### Worked perspective example

Use normalized film coordinates $(x,y)$ and a pinhole camera at the origin looking along negative $Z$, with vertical field-of-view half-angle $\alpha$. An unnormalized camera ray can be

$$
q(x,y)=(x\tan\alpha,\ y\tan\alpha,\ -1),\qquad d=q/\|q\|.
$$

For this square normalized film, the local solid-angle element is

$$
d\omega=\frac{\tan^2\alpha}{\left(1+\tan^2\alpha(x^2+y^2)\right)^{3/2}}\,dx\,dy.
$$

The Jacobian is largest near the optical axis and smaller toward the image corners. With a non-square film, use separate horizontal and vertical ray scales in $q$; the numerator and denominator then use their product and the corresponding squared scales.

For a small horizontal pixel width $h$, the finite direction change across that pixel is $d(x+h/2,y)-d(x-h/2,y)$; dividing by $h$ gives the central-difference estimate of $\partial d/\partial x$. At range $R$ along the ray, a small angular change $\delta\theta$ spans approximately $R\,\delta\theta$ in transverse world distance. If distance instead means axial depth $D$, then $R=D/\cos\theta$ for a ray at angle $\theta$ from the optical axis. Thus the same pixel covers a wider surface region farther away, and an off-axis ray reaches farther than an on-axis ray at equal axial depth. Near the image edge the mapping's derivative also differs from its center value; using the local ray differential captures that change better than assigning every pixel the same world-space footprint.

### Practical checks and limitations

Render a high-frequency checkerboard on a plane receding toward the horizon. Compare point sampling, isotropic mip filtering, and anisotropic filtering while moving the camera slowly; observe aliasing and temporal stability. Inspect the differential footprint near the center and edge of a perspective image; it should reflect the projection geometry. Then test at a silhouette and a curved surface, where differential assumptions can break down. Keep random sampling density, pixel reconstruction filter, lens sample, and motion time in their appropriate dimensions; a physical pixel integrates over a spatiotemporal sensor response, not only a direction.

PBRT's documentation supplies implementation and measure context, but this supplement does not claim that every renderer uses PBRT's structures or that first-order ray differentials solve all antialiasing. They are a useful local approximation that complements multisampling and reconstruction filters.


<!-- end V11.24 -->

<!-- begin V11.25 -->
## Four deeper links from the TUM lecture review

The following sections add source-grounded teaching extensions from the selected Multiple View Geometry playlist and its official chapter slides. The source ledger gives the full read status and exact transcript/page coverage. Claims described as **lecture material** are reported from that source; equations labeled **derivation** are developed here from the source's setup. They extend the previous three sections without making the video titles stand in for lesson evidence.

### Photometric identifiability: a direct method needs a brightness model

Feature pipelines first turn image patches into selected points and descriptors, then match those descriptors across views. Direct methods instead measure the image residual at many or all usable pixels. In a direct alignment, a candidate pose and scene depth predict where a reference pixel should land in the next image. The pose is judged by whether the predicted sample has compatible appearance. This retains image information that sparse selection discards, but it makes photometric assumptions part of the geometry estimator.

The baseline model is brightness constancy along a tracked scene point:

$$
I_1(x) \approx I_2\!\left(\pi(TX(x))\right),
$$

where $X(x)$ is the 3D point represented by the reference pixel and its current depth, $T$ maps it to the second camera, and $\pi$ projects it to image coordinates. A direct residual is

$$
r(x;T,X)=I_2\!\left(\pi(TX(x))\right)-I_1(x).
$$

Minimizing the sum of squared residuals can provide precise subpixel alignment when the image has usable gradients, the initial pose is nearby, and brightness is stable. It can fail when the relevant patch is textureless, when gradients constrain motion in only one direction, when the warp is too large for local linearization, or when the same surface changes measured intensity between frames.

The last failure is common in ordinary cameras. A simplified sensor model is

$$
I_i(x)=G_i\!\left(t_i V_i(x) B_i(x)\right),
$$

where $B$ is scene irradiance reaching the sensor, $V$ is lens vignetting, $t$ is exposure time, and $G$ is the camera response curve. This model and its calibration factors are presented in the official TUM direct-SLAM slides (Chapter 8, pp. 26–27). If the response function is calibrated and invertible over the working range, the image can be mapped back toward irradiance by applying $G^{-1}$ and dividing by the known exposure and vignette factors. That correction does not make the world Lambertian: specular reflection, changing illumination, shadows, occlusion, saturation and unmodeled sensor processing can still change appearance.

#### Derivation: which pose changes are visible to a direct residual?

Let $q=\pi(TX)$ be the predicted image location. For a small pose update $\delta\xi$, linearize the residual:

$$
r(\xi+\delta\xi) \approx r(\xi)+\nabla I_2(q)^T\,J_\pi(TX)\,J_{TX}(\xi)\,\delta\xi.
$$

This is the chain rule: a pose update moves the 3D point in camera coordinates, projection converts that motion to pixels, and the image gradient converts pixel motion to brightness change. If $\nabla I_2(q)$ is close to zero, that pixel contributes little photometric information about pose. If all gradients in a patch point in nearly the same direction, motion tangent to the edge changes intensity very little. A dense set of pixels can therefore still produce a poorly conditioned pose estimate.

This is the same aperture limitation described in the optical-flow lecture. Under the first-order brightness model, a point gives

$$
I_x u+I_y v+I_t=0,
$$

one scalar equation for horizontal and vertical flow $(u,v)$. A flat patch supplies almost no constraint. A straight edge constrains the component normal to the edge but leaves tangential motion ambiguous. Corners and patches containing gradients in independent directions better constrain both components. Dense sampling increases the number of equations, but it cannot repair a repeated or rank-deficient arrangement of image gradients.

Feature descriptors make a different tradeoff. They retain fewer measurements and may discard subtle intensity detail, but are designed to remain recognizable under some changes of scale, rotation, viewpoint or illumination. Direct residuals avoid a discrete feature-detection/matching stage and can exploit small photometric variations, but depend on a calibrated or approximately stable brightness model and usually a good initialization. Neither class dominates in every setting: texture, motion size, illumination change, sensor limits and available computation determine which information is reliable. The TUM Lecture 6 explains sparse/dense and small/wide-baseline tradeoffs; the official direct-SLAM chapter compares the classical sparse pipeline with direct methods (pp. 3–8) and explicitly warns that brightness constancy is violated by exposure, vignette and gamma changes (pp. 25–27).

**Scope note.** The source review reports those assumptions and tradeoffs. The residual Jacobian chain rule and the identifiability reading above are a mathematical extension of them; the lecture does not claim that dense methods are universally more robust or that calibration removes all appearance change.

### A variational optical-flow model, derived from the brightness equation

Lecture 6 gives the brightness-constancy assumption and distinguishes the sparse Lucas–Kanade model from the dense Horn–Schunck model. Lecture 13 explains the variational recipe: state a data-fit term, add a regularizer expressing prior knowledge, derive an Euler–Lagrange condition, and solve it numerically. Combining those two source ideas yields a canonical dense-flow teaching example.

Write flow as two fields $u(x,y)$ and $v(x,y)$, and define the linearized brightness residual

$$
e(u,v)=I_xu+I_yv+I_t.
$$

Penalize brightness disagreement while preferring spatially smooth flow:

$$
E(u,v)=\int_\Omega e(u,v)^2+\lambda\left(\lVert\nabla u\rVert^2+\lVert\nabla v\rVert^2\right)\,dx\,dy.
$$

Here the data term comes from the first-order brightness constraint, and the regularizer encodes the prior that neighboring pixels usually have similar motion. The parameter $\lambda$ balances image evidence against that smoothness prior. This does not assert that true motion is always smooth: object boundaries and occlusions violate it, which is why excessive regularization can smear motion across a boundary.

#### Worked Euler–Lagrange derivation

Assume fixed boundary values (so admissible variations vanish on $\partial\Omega$) for this derivation. If boundary values are free, the gradient penalty instead gives the natural Neumann conditions $\partial_n u=0$ and $\partial_n v=0$ on the boundary.

For $u$, the integrand is $L=e^2+\lambda(u_x^2+u_y^2+v_x^2+v_y^2)$. Its derivative with respect to $u$ is $L_u=2I_x e$. Its derivatives with respect to $u_x,u_y$ are $2\lambda u_x,2\lambda u_y$. Applying the 2D Euler–Lagrange condition,

$$
L_u-\partial_x L_{u_x}-\partial_y L_{u_y}=0,
$$

gives

$$
I_x e-\lambda\Delta u=0.
$$

Similarly, variation with respect to $v$ gives

$$
I_y e-\lambda\Delta v=0.
$$

These equations are coupled through $e=I_xu+I_yv+I_t$. A gradient-flow solver evolves the fields in artificial time along the negative variational derivative; after discretizing space and time, each iteration uses local image derivatives plus neighboring-flow values. The exact discretization and step size affect stability, and an Euler–Lagrange stationary point alone does not certify a global minimum unless convexity and suitable boundary conditions are established.

This model also explains the aperture problem algebraically. At one pixel, the data term constrains only the dot product of flow with the image gradient. If $(I_x,I_y)=(1,0)$ and $I_t=-2$, the data equation says $u=2$ but says nothing about $v$. The smoothness term can fill in a plausible neighboring value for $v$; it cannot recover a component that the local image data never measured. At a corner with independent gradients, aggregating nearby equations can constrain both components. Lucas–Kanade uses that local patch assumption; a dense variational model spreads information with a smoothness prior. The TUM captions introduce the two methods and their different local assumptions, while the objective and derivation here are an original synthesis of Lectures 6 and 13.

### From a six-parameter pose update to a bundle-adjustment Jacobian

An $SE(3)$ pose has six local degrees of freedom: three translation and three rotation coordinates. Chapter 2’s official slides derive twists as tangent vectors and give camera-frame velocity; Lecture 3 explains why optimizing in the tangent algebra avoids nine rotation-matrix entries with orthogonality constraints. Bundle adjustment then measures how each pose and point changes a reprojection residual. Making this connection explicit helps identify which image observations constrain which pose directions.

Fix a convention: use the computer-vision optical frame with $+X$ right, $+Y$ down, and $+Z$ forward; the projection and Jacobian below use this frame. Convert from the graphics camera frame before applying them. Let $T$ map world coordinates to camera coordinates, and use a left-multiplicative update $T'=\exp(\widehat{\delta\xi})T$, with $\delta\xi=(\delta v,\delta\omega)$ ordered translation then rotation. For current camera-space point $p=TX$, a first-order update gives

$$
\delta p=\delta v+\delta\omega\times p
=\begin{bmatrix}I&-[p]_\times\end{bmatrix}
\begin{bmatrix}\delta v\\\delta\omega\end{bmatrix}.
$$

The sign follows from $\delta\omega\times p=-[p]_\times\delta\omega$. With normalized pinhole projection $\pi(p)=(p_x/p_z,p_y/p_z)$ in that explicitly stated $+Z$-forward frame,

$$
J_\pi(p)=
\begin{bmatrix}
1/p_z&0&-p_x/p_z^2\\
0&1/p_z&-p_y/p_z^2
\end{bmatrix}.
$$

For pixel intrinsics with focal lengths $f_x,f_y$, skew $s$, and principal point offsets, left-multiply by the corresponding 2D intrinsic linear part. The reprojection Jacobian for residual $r=\pi(TX)-z$ is therefore

$$
J_{r,\xi}=J_\pi(p)\begin{bmatrix}I&-[p]_\times\end{bmatrix}.
$$

If instead the implementation uses a right update $T'=T\exp(\widehat{\delta\xi})$, or orders rotation before translation, the Jacobian changes by the relevant permutation/adjoint. The convention must be stated; copying the formula without its update side is a common source of sign errors.

In bundle adjustment, each observed point-view pair contributes a small residual and Jacobian block. A point observed by several cameras couples those camera blocks through its shared 3D coordinate; the full normal matrix is sparse because most points appear in only a few views. The same Jacobian spectrum explains observability: if a nonzero parameter perturbation produces almost no image displacement across all observations, the corresponding normal-matrix direction has a small eigenvalue and is weakly constrained. Global coordinate changes and monocular scale are gauge freedoms; they must be fixed by a reference frame or external metric cue. Poor camera motion can create additional weak directions even after the formal gauge is fixed.

For example, repeated nearly identical views of a planar calibration target give many residuals but little new angular diversity. More corners reduce random detection noise, yet do not replace tilted views that change the Jacobian's directions. Conversely, varied poses and points spread across the image improve separation of focal length, pose and distortion effects. The source lectures establish Lie coordinates, the nonlinear reprojection objective and local solvers; the Jacobian product above is an original derivation under the stated convention. It is not a global-optimality claim: bundle adjustment remains nonlinear, may have local minima and cannot identify a gauge or missing scene variation from data alone.

### Pairwise epipolar agreement does not guarantee one shared 3D structure

The essential matrix is a two-view test: a matched pair must satisfy an epipolar constraint. For many views, satisfying each pair separately is not always enough to prove that all observations arise from one common 3D point or line. Each pair can be explained by its own depth/scale choice; a shared reconstruction requires those choices to be mutually compatible.

Lecture 11 derives the multi-view point condition by writing each observed ray as $\lambda_i x_i=\Pi_iX$, then eliminating each unknown depth with the cross-product matrix $[x_i]_\times$, since $[x_i]_\times x_i=0$. Stacking the resulting constraints gives a homogeneous system in the common 3D point. In the lecture’s setup, a rank-three matrix gives a unique homogeneous point up to scale; lower rank leaves a family of possible points; rank four has no nonzero common solution. Geometrically, each view contributes two independent planes through its viewing ray, and their intersection is the point preimage. (The captions themselves contain notation errors; see V11’s source caveat.)

The same source expresses joint compatibility through a multi-view matrix whose rank-one condition carries shared depth consistency. Pairwise epipolar relations are contained in this joint condition, but the joint relation also requires scale factors to agree across views. Three-view trilinear constraints express that coupling without constructing a full point for every pair. This is why “all pairs pass an epipolar test” is weaker than “one 3D point explains every view.”

Lines provide a concrete example of how more views add constraints. A 3D line back-projects to a plane in each camera; two such planes generically intersect in a line, but the line reconstructed from views 1 and 2 need not equal the line reconstructed from views 2 and 3. A third view makes the shared intersection check meaningful. The TUM Lecture 12a review further records the degeneracy where camera translation lies within the observed line’s preimage plane: the camera sees the same plane again and gains no new depth information. These are source-reported constructions; this paragraph's common-structure interpretation is their synthesis.

This distinction is useful in map validation and multi-camera calibration. Use pairwise epipolar residuals to reject obvious mismatches cheaply, but use a joint multi-view objective or rank/trilinear consistency to test whether a track can be represented by one shared landmark. Under noise, exact rank tests become approximate and reprojection fitting is usually more useful; robust losses are needed when some correspondences are outliers. The multi-view rank source explicitly warns that its closed-form alternating solution is not statistically optimal under noise, while the bundle-adjustment source motivates joint reprojection error for noisy observations. A staged solver can therefore use geometric constraints to initialize and validate tracks, then refine the shared camera and point variables jointly.

#### A compact evidence-to-claim map

- **Photometric model and calibration terms:** official TUM Chapter 8, pp. 26–27; direct-vs-feature pipeline and limitations: pp. 3–8; captioned TUM Lecture 6, normalized transcript lines 1–1184, reviewed in `reviews/tum-lecture-01-02-03-06.md`.
- **Brightness equation, sparse/dense flow assumptions:** TUM Lecture 6, lines 1–1184; variational objectives, Euler–Lagrange conditions and gradient descent: TUM Lecture 13, normalized transcript lines 1–3063, reviewed in `reviews/tum-lecture-13.md`. The optical-flow energy and its two Euler–Lagrange equations above are an original derivation.
- **Twist coordinates:** official TUM Chapter 2, pp. 9–20; Lie coordinates also in Lecture 3, transcript lines 1–1741; projection residual and nonconvex solver limits: captioned TUM Lecture 12b, lines 1–1529. The left-update Jacobian above is an original derivation with its update convention stated.
- **Joint multiview rank and line degeneracy:** captioned TUM Lecture 11, normalized transcript lines 1–1783, and Lecture 12a, lines 1–974, reviewed in `reviews/V11.md` and `reviews/tum-lecture-12a-12b.md`; noisy refinement limitation: TUM Lecture 12b, lines 1–1529.


<!-- end V11.25 -->

<!-- begin V11.18 -->
<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/01 - Computing Foundations/00 - Reading Map|Reading map]] · [[Chapter Edition/01 - Computing Foundations/35 - Camera Extrinsics and Projection Geometry|Previous chapter]] · [[Chapter Edition/01 - Computing Foundations/37 - Camera Contracts and Transform-Chain Debugging|Next chapter]]
