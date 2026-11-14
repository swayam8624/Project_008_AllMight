# 04 — Sampling, Filtering and Reconstruction

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport|Next chapter]]

Once an image is sampled, reconstruction must manage aliasing, filtering, memory layout and execution cost. The practical examples connect signal-processing ideas to real image buffers and render schedules.

**By the end:** Explain how sampling and filtering affect image quality and implementation.

**Recall question:** Why can two different continuous signals produce the same samples?

<!-- Derived from Continuous Notes/University - Worlds, Images and Inference - Graphics, Vision and Photography.md; source lines 3634–5492; source-span SHA256 a5e8c6ab058880d462b98ce41d402c3c9d234b2758b0fab61efaabf5a5e4874b. -->

<!-- CHAPTER-SOURCE-BEGIN -->
## Image sampling, filtering, and efficient reconstruction

Sampling fixes the measurements available; filtering reconstructs a usable image and efficient image operators make those filters practical at scale.

### Animate a pixel by respecting the buffer’s coordinates and lifetime

A CPU pixel buffer is a linear sequence even when the image it represents has two dimensions. For an image with width $W$ and height $H$, the pixel at integer coordinate $(x,y)$ has row-major index

$$
i=yW+x.
$$

This formula assumes tightly packed rows and counts pixels, not bytes. The general layout question is [[Concepts/Matrix storage indexing and strides]]. A padded image instead needs its row stride and pixel format; a four-channel byte buffer is not indexed like an array of complete pixel records. The coordinate check must come before this calculation: require $0\le x<W$ and $0\le y<H$. Checking only that the final index is below $WH$ is insufficient: an $x$ value past the right edge can spill into the next row while still producing an in-range index. Conversely, comparing $x$ to the image height is wrong on a non-square image. The buffer has $WH$ slots, but horizontal and vertical coordinates have different limits.

The pixel-programming episode shows an animation that moves a point beyond its allocated buffer and receives an array-index exception. Its narrated debugging then changes an image-height value to 600 as a temporary fix, while the intended rule is to skip an out-of-bounds write. Preserve that boundary rule, not the unexplained dimension edit. For an image of 800 by 600 pixels, `(799,599)` maps to `479999`; `(800,599)` is invalid even though `599*800+800` equals `480000`, the first index past the allocation. For a clearer row-spill counterexample, `(800,0)` maps numerically to 800, which is inside this allocation but names the first pixel of the next row. The same checks protect negative coordinates before any unsigned conversion. In fixed-width C++ storage code, also check multiplication before forming the allocation size: for positive height, require the width to fit within the maximum representable element count divided by the height, then separately account for bytes per pixel. An overflowed allocation size cannot be repaired by a later coordinate check. [3D Game Programming – Episode 5 – Playing with Pixels!, 00:05:01–00:08:44](https://www.youtube.com/watch?v=dwn_UrItwfw&t=301s)

Animation should be computed in a continuous coordinate space and quantized only when choosing the pixel to write. A useful teaching model is

$$
x(t)=c_x+r_x\sin(2\pi t/T),\qquad y(t)=c_y+r_y\cos(2\pi t/T),
$$

where $T$ is the period and $(c_x,c_y)$ is the center of the screen. Keep $x(t)$ and $y(t)$ as floating-point values through the motion calculation; reject nonfinite values and values outside the chosen integer-conversion range before converting them to pixel coordinates at the raster boundary, then apply the integer bounds test. A floating-to-integer cast is not a clipping operation. This keeps the path calculation separate from the discrete storage grid. The video uses a time remainder and integer conversion to make a periodic motion, but the speaker explicitly says he is unsure how the conversion rounds. The captions therefore support the experiment, not a precise rounding rule or a smoothness guarantee. A real implementation should use a monotonic elapsed-time source, declare whether it quantizes by truncation or rounding, and test wraparound at the period boundary. [00:02:48–00:04:05](https://www.youtube.com/watch?v=dwn_UrItwfw&t=168s)

We can resolve the cast question using the language rules instead of preserving uncertainty from the narration. For finite values in the destination range, Java floating-to-integer conversion truncates toward zero: 3.9 becomes 3, and -3.9 becomes -3. That differs from floor, which maps -3.9 to -4. Near the left edge, truncating -0.2 produces 0; therefore decide whether the continuous sample is outside the image before converting it if negative subpixel locations should be clipped. Java additionally defines behavior for nonfinite and oversized inputs, while C++ makes a floating-to-integer conversion undefined when the truncated result is not representable. Keep the same explicit finite/range checks in a portable teaching algorithm instead of relying on a language-specific exceptional-value conversion. [Java Language Specification, narrowing conversion](https://docs.oracle.com/javase/specs/jls/se21/html/jls-5.html#jls-5.1.3), [C++ draft, floating-integral conversion](https://eel.is/c++draft/conv.fpint).

A changing image also needs a defined policy for old pixels. If every frame represents only the current animation state, clear the pixel buffer once before drawing the new state; otherwise, old samples remain and form trails. If a trail is the intended effect, retain the buffer deliberately and define how it fades or clears. The episode clears all $WH$ pixels before drawing its current contents, then later draws many positions to make a visible tail. This illustrates why clearing belongs to the frame’s rendering policy: clearing inside the loop that creates a trail would erase the very history the effect needs. The reported 13.1 million writes comes from $256\times256$ pixels times 200 repeated draws; it is an arithmetic workload estimate from the tutorial, not a hardware-neutral performance benchmark. [00:09:48–00:13:06](https://www.youtube.com/watch?v=dwn_UrItwfw&t=588s)

The source is a retrieved English VTT caption track, read in full. It does not independently prove what the code finally contained or the runtime speed; the episode itself leaves the temporary dimension adjustment unresolved. The stable lesson is the invariant: map valid coordinates into a known buffer, choose whether the frame is a replacement or accumulated history, then measure the work that policy actually performs.


### Treat a render-order fix as evidence about a frame schedule

A visible black line near a render-distance boundary is a symptom, not a root cause. It could come from missing geometry, a gap between meshes, an incorrect draw order, a depth-test/write mismatch, blending, or a projection/clip convention. In this tutorial, the author responds to a viewer report by moving wall generation out of the floor method, making it a separate `walls` method, and then changing where that method is called in the screen’s render sequence. The captioned result changes as the call order changes: the wall appears in front of the distance limiter when drawn after it. That is a useful debugging observation about this particular scene and renderer path. [3D Game Programming – Episode 18.1 – A Few More Things, 00:00:01–00:05:05](https://www.youtube.com/watch?v=98K5Bm2sApY&t=1s)

The transferable method is to make the frame schedule visible. List the passes or geometry groups in the order they are submitted, then vary one ordering decision while holding the scene fixed. Separating `floor`, `walls`, and `distanceLimiter` into named calls makes that experiment easy to state and repeat. A method boundary can improve the programmer’s ability to isolate work; it does not itself change graphics semantics. The later call order is the intervention being tested.

Do not generalize the result to say that the last submitted object always appears in front in a 3D renderer. With depth testing and depth writes active, depth comparison normally determines which fragment wins; submission order alone does not let a farther opaque surface override a nearer one. If reordering changes the image, inspect the actual depth function and clear value, whether both draws write depth, whether either is transparent or an overlay, and whether the geometry shares the same depth. The video captions do not establish those states, so the exact reason the wall/limiter relationship changed remains open. The correct lesson is to turn render order into an explicit experiment and then inspect the state that mediates its effect.

A two-fragment experiment separates three mechanisms. Declare a conventional depth buffer cleared to 1, a LESS comparison, and depth writes enabled. A red opaque fragment at depth 0.2 followed by a blue opaque fragment at 0.8 leaves red: the second fragment fails the depth comparison. Reverse the submissions and the final pixel is still red. Turn depth testing off and let each opaque write replace the color: now the final color follows submission order. The depth-test mechanism is specified separately from color writes in the [Vulkan fragment-operations specification](https://docs.vulkan.org/spec/latest/chapters/fragops.html#fragops-depth). Neither observation establishes which state the tutorial used.

Transparency is a third case. In linear RGB, put half-opacity red and half-opacity blue over black, using straight-color source-over compositing and no intervening rejection. Red then blue gives $(0.25,0,0.5)$; blue then red gives $(0.5,0,0.25)$. The blending operation is order dependent even though both fragments have the same opacity. A real capture must therefore include color/blend state as well as depth state. Equal-depth fragments, a different comparison, stencil rejection, depth bias, reversed-depth conventions and shader-written depth require their own stated experiment. These numeric examples are original explanations; the video captions do not establish any of those settings.

The code-organization portion is similarly bounded. The source moves wall code to reduce clutter and store a forward value in a class member so the extracted method can access it. That solves the narrated scope error, but a longer-lived member is not automatically the right ownership model; if the value is only needed while building one frame, a parameter or frame-local data object may communicate the lifetime more clearly. The captions describe a small repair in an evolving Java renderer, not a general architecture requirement. (Source `src-b79d721772f4fbacfe25`, caption units `u-1587df8a1f6868718677eec3`, `u-3424fc96be48fb4bc2a01718`, `u-b512eb33526c7caa9701317a`, `u-f5fc74bfc8a9608f0d2cf676`, `u-e96e5ea524660c7a37d89430`.)

The cited material is the retrieved English VTT caption track. It records the presenter’s code changes and described image, but no source tree, depth-state capture, or runtime was independently inspected. Treat the described fix as a reproducible question to investigate, not as proof of the graphics pipeline state.


### Sampling, integral images, and the limits of a box average

Sampling can preserve a continuous signal only under a strong condition: its frequency content must be band-limited below half the sampling frequency. At the Nyquist boundary, phase can make a sinusoid’s samples all equal or alternate in ways that cannot distinguish it from other frequencies; therefore content must be strictly below $f_s/2$. For sample spacing $\Delta$, a sinusoid at frequency $f_0$ has the same samples as frequencies $f_0-kf_s$ for integer $k$, because the phase shift per sample changes by $2\pi k$. Those indistinguishable alternatives are aliases. Once sampled, they have collapsed to the same numbers; no later digital filter can determine which original frequency produced them. Anti-alias filtering must happen before sampling. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]] (video 00:01:39–00:09:35)

Pixel-area integration is already a weak low-pass filter. A one-dimensional box average of width $\Delta$, normalized to preserve constant brightness, has frequency response

$$
H_{\rm box}(\omega)=\frac{\sin(\omega\Delta/2)}{\omega\Delta/2}.
$$

It attenuates high frequencies only as $1/\omega$ and its first zero occurs at $2\pi/\Delta$, twice the Nyquist angular frequency $\pi/\Delta$. Repeating the average squares the response, improving the high-frequency decay to $1/\omega^2$ but also attenuating useful passband frequencies. Each additional box average raises the sinc response to another power. This explains why coarse block averaging is computationally attractive but can still alias, especially in images with stone/table boundaries or fine stripes. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]] (video 00:09:41–00:11:33, 00:17:04–00:26:33)

The Fourier transform also imposes a boundary condition that is easy to overlook. A discrete Fourier transform treats a finite image as periodically repeated. Unless opposite borders match, the wraparound creates an artificial step, injecting a $1/f$ tail into the spectrum. A smooth apodizing window can fade image edges toward zero before analysis; mirror extension avoids a brightness jump but may still introduce a slope discontinuity, with faster spectral decay. Thus a measured frequency spectrum can reveal the windowing and boundary rule as much as the natural image itself. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]] (video 00:27:24–00:31:49)

### Texture mapping is filtering over a changing footprint

A texture is a function over a parameter domain, often the unit square. Each surface point maps to coordinates $(u,v)$ that index the image. A surface may need multiple charts or seams because one planar parameterization cannot cover every shape without distortion. At a receding angle, adjacent screen pixels correspond to unevenly sized and shaped patches of texture. This makes texture lookup another sampling problem. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-07 - Perspective Projection and Texture Mapping|CMU-07]]

Nearest lookup selects one texel. Bilinear filtering blends four neighbors and smooths magnification. But when one screen pixel covers many texels, a single lookup aliases. A mipmap stores prefiltered levels at successively lower resolutions. The renderer selects a level based on the projected footprint. Trilinear filtering blends neighboring levels. [[Convolution and filter footprints|Anisotropic filtering]] handles an elongated footprint, such as a floor receding toward the horizon; isotropic mipmapping can overblur because one square level must approximate a long thin region. A checkerboard that alternates faster than the screen can resolve should average toward gray, rather than flicker black and white as the camera moves. Filter before reducing resolution. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-07 - Perspective Projection and Texture Mapping|CMU-07]]

### Pixel footprints and texture derivatives

Texture lookup is a mapping from screen coordinates $(x,y)$ to texture coordinates $(u,v)$. Locally, derivatives $\partial u/\partial x$, $\partial v/\partial x$, $\partial u/\partial y$, and $\partial v/\partial y$ describe how a pixel footprint stretches into texture space. The Jacobian of this map transforms an approximately square pixel into a parallelogram in texture coordinates. If its singular values are similar, an isotropic mip level is adequate. If one singular value is much larger, the footprint is elongated; anisotropic filtering samples along the long axis. This local Jacobian also explains why perspective changes texture density so quickly near a horizon.

Nearest filtering can preserve crisp texel blocks during magnification but flickers in minification. Bilinear filtering smooths between texels but still uses a tiny support region. Trilinear filtering first bilinearly filters two mip levels and blends between them according to a fractional level. Anisotropic methods take additional samples along the footprint. Each step spends more computation to better approximate an integral over the pixel footprint. The right filter depends on the projected scale and orientation of texture detail, not on an arbitrary preference for soft images. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-07 - Perspective Projection and Texture Mapping|CMU-07]]

### Antialiasing is controlled filtering before and after sampling

A pixel grid samples an underlying continuous image. A ray tracer can answer a query such as “what radiance arrives at image coordinate $(x,y)$?”, but it does not know where to ask, and unqueried detail is lost. Sampling chooses query locations; reconstruction turns those finite values into a displayed image. Quantization is a separate operation: it rounds each real-valued channel to a finite set of codes. Keeping these concepts separate helps diagnose whether an artifact comes from spatial undersampling or limited channel precision. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-15 - Antialiasing|MITG-15]] (00:25:11–00:28:52)

When a signal changes faster than the sample grid can distinguish, distinct patterns can produce the same samples. A high-frequency sinusoid sampled at regular intervals may look like a lower-frequency wave. This “alias” explains moiré in brick patterns and the apparent backward rotation of a wheel whose rotation period nearly matches the camera frame interval. At 30 frames per second, a wheel making one revolution in 29 frames moves only one frame’s worth of apparent phase between captures, so the sampled sequence can suggest slow reverse motion. Rolling-shutter cameras add row-by-row time offsets and can shear moving geometry. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-15 - Antialiasing|MITG-15]] (00:09:43–00:13:01; 00:29:00–00:30:38)

A 1D function can be treated as a vector with one component per location. Its inner product with another function is an integral: $\langle f,g\rangle=\int f(x)g(x)\,dx$. Fourier analysis measures how strongly a signal aligns with sine and cosine waves at each frequency; the complex exponential $e^{i\omega x}=\cos(\omega x)+i\sin(\omega x)$ packages both measurements. Low frequencies describe broad changes, while high frequencies describe rapid edges and fine texture. Filtering can preserve low frequencies for a blur, preserve high frequencies for edge detail, or isolate a band. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-15 - Antialiasing|MITG-15]] (00:36:36–00:50:27)

Sampling repeats a signal’s spectrum at regular frequency intervals. If the original spectrum is band-limited to maximum frequency $f_{max}$, samples spaced at rate $f_s>2f_{max}$ keep those shifted copies from overlapping. The threshold $2f_{max}$ is the Nyquist rate. With no overlap, an ideal low-pass filter can isolate one copy and invert the transform. In the spatial domain, that ideal reconstruction uses the sinc kernel $\operatorname{sinc}(x)=\sin(\pi x)/(\pi x)$, whose infinite extent and negative lobes make exact numerical reconstruction expensive and prone to ringing. Real images contain sharp discontinuities and therefore arbitrarily high-frequency content, so practical graphics uses finite approximations and accepts a tradeoff. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-15 - Antialiasing|MITG-15]] (00:58:26–01:07:38)

Supersampling renders multiple subpixel locations and combines their colors. Uniform samples averaged in a box are simple, but the box filter is not an ideal low-pass reconstruction and can leave aliasing. A finite bicubic Mitchell filter includes a useful negative lobe and often reduces visible moiré at comparable effort. Filter choice affects sharpness and ringing as well as error magnitude. For a texture tilted in perspective, one screen pixel may cover a long, narrow footprint in texture space; a square mip level can average the wrong proportions. Elliptical weighted sampling approximates the actual projected footprint more faithfully. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-15 - Antialiasing|MITG-15]] (01:08:45–01:19:32)

Text is a difficult special case because glyph strokes are thin and occupy few pixels, yet viewers inspect them closely. Some displays expose separate red, green, and blue subpixels in a known horizontal order. ClearType estimates coverage at those subpixel positions, effectively increasing horizontal brightness resolution. It can improve glyph edges, but only when the renderer knows the subpixel layout; rotating the panel or changing its stripe order can create colored fringes. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-15 - Antialiasing|MITG-15]] (01:20:06–01:22:32)

A practical mental checklist is: identify the continuous signal being approximated; estimate its screen-space footprint and frequency; filter before downsampling; use enough samples for the remaining bandwidth; then account for the display’s reconstruction and channel quantization. No single sampling pattern is best for all scenes because geometry, motion, texture scale, contrast, and perception all affect the visible error. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-15 - Antialiasing|MITG-15]] (01:22:35–01:23:00)

### Resolution, blur, and display

A sensor pitch of $p$ micrometers corresponds to a one-dimensional sampling frequency of $1/p$ samples per micrometer in the image plane, with Nyquist at half that frequency. But scene detail maps to sensor detail through magnification and focal length. A fine fabric weave may alias in a distant wide shot yet resolve in a close macro image; changing distance, focus, aperture, and output size changes the pattern’s relationship to the pixel grid. An optical low-pass filter blurs before sampling, while post-capture downsampling should apply an appropriate low-pass filter before decimation. Simply throwing away every other row creates new aliasing. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-07 - Photography session 07|PHOTO-07]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-08 - Photography session 08|PHOTO-08]]

Motion blur can be treated as a point-spread function. If a camera moves linearly during exposure, an idealized distant scene may be convolved with a line segment whose length corresponds to image displacement. A deblurring algorithm can attempt an inverse filter when blur is known, but the inverse amplifies noise where the blur transfer function is weak. Real handheld shake is two-dimensional and may vary across depth; rolling shutter makes each row’s exposure begin at a different time. Optical stabilization shifts a lens group, sensor stabilization moves the sensor, and electronic stabilization crops or warps frames. Each addresses some motion, but none recovers detail lost to severe blur or subject movement. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-10 - Photography session 10|PHOTO-10]]

### Pyramids trade pixel count against aliasing and feature visibility

If image width and height are each reduced by factor $r$, a level costs roughly $r^2N$ pixels for an original $N$-pixel image. A half-scale pyramid therefore costs $N(1+1/4+1/16+\cdots)=4N/3$, only one third more pixels than a single full-resolution level, while providing progressively cheaper coarse search. Yet halving abruptly can alias high frequencies, and box-averaging $2\times2$ cells loses detail aggressively. The checkerboard (red-black) subsampling pattern keeps one of two pixel colors and forms a regular grid rotated by $45^\circ$; its spacing grows by $\sqrt2$ while sample count halves. One level with that pitch change costs half the previous level, for total pyramid cost approaching $2N$. It reduces scale more gently and cycles back into alignment after another such step. SIFT is a contrasting example: it samples multiple intermediate scales within an octave because a feature may be clearest at one blur level and disappear at another. Scale should be selected by both computational cost and the stability of the visual evidence. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-14 - Inspection in PatQuick, Hough Transform, Homography, Position Determination, Multi-Scale|MITV-14]] (video 01:17:34–01:25:31)

### The processing pipeline reconstructs a usable image

A raw file contains minimally processed sensor values, usually before demosaicing and final color rendering. A JPEG is a rendered image: the camera has demosaiced, white-balanced, denoised, sharpened, tone-mapped, converted to a color space, quantized, and compressed. Different raw converters can produce different results because their algorithms and defaults differ. Camera metadata such as focal length, exposure, orientation, and sometimes location can help the converter choose distortion, vignetting, and chromatic-aberration corrections. Re-editing a JPEG repeatedly compounds irreversible quantization and compression; raw or a high-bit-depth intermediate preserves more room for later decisions. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

Gamma-like encoding allocates code values in a way that better matches display behavior and human sensitivity. If display luminance follows approximately $V^{2.5}$, a camera or image encoding can apply a roughly reciprocal power so that the combined system is closer to the desired response. Equal code steps in a nonlinear space approximate more uniform perceptual changes than equal steps in linear radiance, because vision is more sensitive to ratios than to absolute increments. A gamma transform is not itself a complete high-dynamic-range tone mapper. Strong global compression can wash out midtones or desaturate colors. Historical CRT and modern LCD workflows differ in where the encoding and decoding occur, but the system must still account for display characteristics and viewing conditions. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

HDR capture extends the measurable range. Exposure bracketing uses different shutter times to gather long exposures for shadows and short exposures for highlights. For a static scene, align the frames, scale them to a common radiometric basis, reject saturated measurements, and weight longer clean exposures more strongly because they often have better SNR. The method fails when objects move between frames, when camera shake changes alignment, or when noise makes a region appear structurally different. Alternating short- and long-exposure pixels can capture range in one frame but creates demosaicing and spatial-resolution problems. Per-pixel neutral-density filters trade away photons; non-destructive readout can measure bright pixels before saturation but does not automatically improve low-light SNR. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

A robust mobile strategy captures a burst of underexposed frames at the same exposure time. The shared exposure keeps the frames visually similar, making alignment easier; underexposure protects highlights; averaging reduces noise in shadows. After alignment and fusion, shadows can be lifted without highlights already being clipped. This is the core logic behind the HDR+ approach described in the lecture. The general lesson is that computation can trade time and repeated measurements for signal quality, but only if the scene is sufficiently stable and the algorithm knows which pixels correspond. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

Tone mapping compresses an HDR image for a limited display. A global curve maps every pixel of the same luminance the same way. Local tone mapping adapts to neighborhoods, often compressing broad illumination changes while retaining local edges and texture. Human vision also adapts spatially and over time; it does not view all scene luminances simultaneously with one fixed response. Artists have long used high-key and low-key organization to imply dazzling brightness or deep darkness on low-range paper. Local tone mapping can reproduce that cue by preserving local contrast while reducing global contrast, but can create halos and a cartoon-like appearance when pushed too far. A bilateral filter helps by blurring nearby pixels that are similar in intensity while respecting large intensity edges. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

The bilateral filter weights a neighbor according to both spatial distance and intensity difference. In simplified form,

$$
I'(p)=\frac{1}{W_p}\sum_{q\in\mathcal N(p)} I(q)\,w_s(\|p-q\|)\,w_r(|I(p)-I(q)|),
$$

where $w_s$ decreases with spatial distance, $w_r$ decreases with tonal difference, and $W_p$ normalizes the weights. A Gaussian blur has only spatial weights and therefore crosses edges; a bilateral filter can denoise within a region while preserving the boundary. Unsharp masking sharpens by subtracting a blurred version to isolate edge detail, thresholding that detail to avoid amplifying noise, then adding a controlled amount back to the original. Because of thresholding and range-dependent decisions, these are nonlinear operations rather than one fixed convolution. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

JPEG compression exploits spatial frequency and visual sensitivity. It converts RGB into a luma-like channel and two chroma channels, often subsampling chroma because people are more sensitive to fine luminance detail than fine color variation. It divides each channel into $8\times8$ blocks, transforms each block with a discrete cosine transform (DCT), quantizes high-frequency coefficients coarsely, orders coefficients in a zigzag pattern, and entropy-codes repeated zeros and common values. The DCT itself is reversible; quantization discards detail and makes the compression lossy. Repeated recompression, especially after cropping or rotation that shifts block boundaries, can expose block artifacts. Raw capture and lossless working formats are preferable for substantial editing. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-13 - Photography session 13|PHOTO-13]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

### Aligning repeated frames

Image alignment often begins with features: corners, textured patches, or gradients that can be matched between images. If an exposure changes, raw intensity comparisons become less reliable because the same surface can have different pixel values. If a region is saturated, its true brightness is unknown and it cannot provide a stable correspondence. Motion estimation may use optical flow to infer per-pixel displacement, but large motion, occlusion, textureless regions, rolling shutter, and noise can break it. Robust estimators reject inconsistent matches and fit a transform to the rest, but they cannot make a single transform describe a scene with strong parallax. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-10 - Photography session 10|PHOTO-10]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-18 - Photography session 18|PHOTO-18]]

A tripod exposure bracket is comparatively easy to align because the camera stays still, but leaves and water can still move. A handheld burst introduces camera shake, so each frame needs registration. HDR+ uses the same exposure across its burst to make alignment easier; its noise reduction depends on accurate alignment and careful treatment of moving objects. Panorama stitching also uses feature matching, but its correct model depends on camera motion. Rotation around the center of projection supports homographies or cylindrical reprojection; translation through a scene with depth variation requires a richer 3D model or local seams and still may not yield a single seamless image. The algorithm must match the capture geometry. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-10 - Photography session 10|PHOTO-10]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-18 - Photography session 18|PHOTO-18]]

### Image processing: local rules over an image grid

An image can be treated as a sampled function (I(x,y)). A linear filter combines a neighborhood with weights, often written as convolution:

$$
(I*K)(x,y)=\sum_{i,j}K(i,j)I(x-i,y-j).
$$

A blur uses weights that average nearby values; a sharpen filter adds high-frequency detail; an edge detector responds to rapid change. Contrast transforms remap intensity values, while a green-screen key uses color information to estimate which pixels belong to foreground versus background. The image-processing lecture moves among visual editing goals, contrast, local neighborhoods, and edge sharpening [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (00:20:00–00:40:00; 00:45:00–01:10:00).

Linear filters are easy to analyze and efficient to implement, but they blur across edges because they treat similar spatial neighbors alike even when they belong to different objects. A bilateral filter adds a similarity weight in color or intensity, so pixels far apart in appearance contribute less even if they are nearby in space. In simplified form,

$$
I'(p)=\frac{1}{Z_p}\sum_q \exp\!\left(-\frac{\|p-q\|^2}{2\sigma_s^2}\right)\exp\!\left(-\frac{\|I(p)-I(q)\|^2}{2\sigma_r^2}\right)I(q),
$$

with normalization Z_p. Spatial scale $\sigma_s$ controls neighborhood reach; range scale $\sigma_r$ controls which intensity differences count as similar. This can smooth noise while preserving boundaries better than a Gaussian blur, though it can create a cartoon-like flatness or halos if parameters are poorly chosen. The lecture explicitly reaches the bilateral filter and edge emphasis [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (01:00:00–01:15:00).

### Image processing changes pixels, neighborhoods, and exposure

An image is a sampled grid of color values, often RGB plus alpha. Alpha records foreground coverage or opacity for compositing. For foreground color $C_f$, background $C_b$, and opacity $\alpha\in[0,1]$, the “over” operation is $C=\alpha C_f+(1-\alpha)C_b$. If storing premultiplied color $\tilde C_f=\alpha C_f$, the operation becomes $C=\tilde C_f+(1-\alpha)C_b$. Premultiplication avoids repeatedly multiplying RGB by alpha and reduces colored fringes when partially transparent pixels are filtered. It does not solve a bad matte: in the green-screen example, green light reflected from the screen onto a person’s shoulder makes foreground and background colors overlap, so a hard green threshold either clips the shoulder or leaves a green halo. Layers also need lighting and shadow consistency to look physically integrated. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (00:16:19–00:24:25; 00:20:20–00:23:16)

A pointwise filter maps each pixel independently, $I'(x,y)=f(I(x,y))$. A tone curve remaps brightness values to fit a display’s limited dynamic range or to spread a crowded histogram over more output levels. Contrast-limited adaptive histogram equalization (CLAHE) computes local contrast mappings over neighborhoods and limits histogram peaks so local enhancement does not over-amplify noise. High-dynamic-range capture has the opposite limitation: a camera exposure cannot simultaneously preserve a sunlit window and dark church interior when their luminances span far more range than the sensor. Exposure fusion aligns several exposures and chooses/blends useful values per region. Motion causes ghosting—four exposures of a moving basketball can yield four balls—so registration and motion-aware blending are essential. Even a merged HDR image must be tone-mapped to a display’s smaller brightness range. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (00:34:14–00:42:38)

Neighborhood filters use convolution. For image $I$ and kernel $K$, $(I*K)(x,y)=\sum_{i,j}K(i,j)I(x-i,y-j)$. A kernel whose weights sum to one preserves a constant image. For example, the one-dimensional box $K=[1/3,1/3,1/3]$ replaces each value by the average of it and its neighbors. A blur kernel has positive center-heavy weights; an edge filter such as

$$
K_{lap}=\begin{bmatrix}0&-1&0\\-1&4&-1\\0&-1&0\end{bmatrix}
$$

compares the center with its neighbors and responds to rapid changes (a discrete Laplacian). Convolution flips the kernel as it slides; correlation does not. Symmetric blur kernels look the same under either convention, while directional filters expose the distinction. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (00:50:14–00:53:11)

A direct $n\times n$ image convolution with an $m\times m$ kernel costs $O(n^2m^2)$ multiply-adds. Separable kernels factor as $K(i,j)=a(i)b(j)$, letting us apply one horizontal and one vertical 1D pass in $O(n^2m)$ work. A Gaussian is separable: $G_\sigma(x,y)=g_\sigma(x)g_\sigma(y)$. A repeated box blur also approaches a Gaussian because sums/averages of independent samples become bell-shaped (the central limit effect); repeated simple filters can approximate a smooth Gaussian blur. Fourier methods provide another route for large kernels by transforming convolution into pointwise multiplication. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (00:53:45–00:58:00)

Downsampling must remove detail that the coarser grid cannot represent. If we shrink an image, first low-pass filter it, then sample; otherwise fine stripes fold into false coarse patterns (aliasing). Upsampling interpolates missing samples but cannot recover information that was never captured. Bilinear interpolation is a weighted average of the four nearest texels and can look soft or reveal stair-stepped diagonals. Edge-aware upsampling uses image structure to preserve likely boundaries, but it invents plausible detail rather than recovering uniquely determined frequencies. Moving least squares warps an image by blending control-point transformations with spatial weights, conceptually related to skinning. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (00:46:00–00:50:14)

A bilateral filter decides which neighbors to average using both spatial proximity and color similarity: $w(p,q)=\exp(-\|p-q\|^2/(2\sigma_s^2))\exp(-\|I(p)-I(q)\|^2/(2\sigma_r^2))$. Normalize the weights for each center $p$, then average nearby $I(q)$. The range term suppresses averaging across strong edges while the spatial term keeps influence local. Because weights depend on the center pixel’s value, this is not one fixed convolution and is more expensive; ambiguity between texture/noise and a real edge can create artifacts. A median filter instead replaces a pixel with the neighborhood’s median. Since extreme outliers do not pull a median as they pull an average, it removes isolated speckle noise robustly and can fill small missing regions. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (01:07:54–01:11:14)

A tiny convolution trace: for samples $[0,0,9,0,0]$ and the box kernel $[1/3,1/3,1/3]$, the center output is $(0+9+0)/3=3$, spreading the impulse to neighbors. A median over the same three-sample neighborhood returns $0$ at each adjacent location and discards the isolated spike. That difference explains why a mean blurs energy outward, while a median rejects a sparse outlier. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-21 - Image processing|MITG-21]] (00:50:14–00:53:11; 01:10:34–01:11:14)

### Fast image operations: sampling, convolution, and integral images

Image pyramids and multiscale inspection require resampling. If high spatial frequencies are sampled below their Nyquist rate, they fold into lower frequencies as aliasing. A low-pass prefilter suppresses frequencies that cannot be represented at the coarser scale. Repeated block averaging is computationally simple, but its frequency response and warping artifacts differ from an ideal rotationally symmetric filter. The course discusses low-pass approximations, Fourier analysis, and the relationship between discrete impulses and convolution. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]] (notes pp. 93–106)

Convolution of image $I$ with kernel $K$ is

$$
(I*K)(x,y)=\sum_{i,j}I(x-i,y-j)K(i,j).
$$

A separable kernel can be applied as two one-dimensional passes, reducing work from roughly $O(whk^2)$ to $O(whk)$ for an image of width $w$, height $h$, and a $k\times k$ kernel. Box filters and repeated box averaging approximate smoother low-pass profiles with simple additions. The exact approximation tradeoff—speed, isotropy, ringing, and blur—must be chosen for the detector rather than treated as interchangeable.

### Sparse differences make wide spline filters cheap

A later Cognex patent asks how to low-pass filter large images cheaply before subsampling. Direct convolution of an $N$-sample signal with an $M$-sample kernel costs roughly $O(NM)$; large support makes repeated multiscale filtering expensive. The trick is to approximate the desired smooth kernel by a piecewise polynomial spline and apply its $(n+1)$st finite difference, where each spline segment has degree $n$. Within a segment, the $(n+1)$st difference is zero. Only the knots and endpoints survive, so the transformed kernel is sparse. Convolve the signal with those few nonzero coefficients, then apply $(n+1)$ cumulative sums to reconstruct the original smoothing response. Differencing and summation are inverse operations (up to boundary/initialization conventions), and their convolution operators commute and associate, which lets the filter be rearranged into an efficient pipeline. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-15 - Alignment, PatMax, Distance Field, Filtering and Sub-Sampling (US 7,065,262)|MITV-15]] (video 00:58:49–01:07:42)

For a piecewise quadratic kernel, the third difference is zero across every smooth segment and nonzero only around the joined pieces. In the lecture’s worked discrete example, four taps remain; their magnitudes are $1,3,3,1$ with alternating signs. The expensive broad weighted sum becomes a few additions and subtractions followed by three running-sum passes. Expanding the spline support by a scale parameter $S$ changes the tap spacing, not the tap count, so the arithmetic cost remains nearly constant as the low-pass bandwidth changes. This makes the method useful for approximating broader Gaussian-like smoothers or a practical finite approximation to the ideal sinc low-pass before downsampling. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-15 - Alignment, PatMax, Distance Field, Filtering and Sub-Sampling (US 7,065,262)|MITV-15]] (video 01:07:49–01:16:48)

Separable filters can be applied horizontally and vertically as one-dimensional passes. A streaming implementation need not store an entire intermediate image: retain only enough rows for the vertical support, then perform horizontal filtering as those rows become available. A general two-dimensional nonseparable kernel remains harder; this patent’s win is greatest when a good one-dimensional or separable spline approximation suffices. The lecture’s calcite demonstration connects digital and optical prefiltering: a birefringent optical low-pass layer makes two nearly coincident copies of the image, suppressing frequencies the sensor grid cannot represent. Removing it can sharpen detail but creates moiré patterns on fine fabric stripes. Filtering sacrifices some nominal resolution to prevent false low-frequency structure from aliasing. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-15 - Alignment, PatMax, Distance Field, Filtering and Sub-Sampling (US 7,065,262)|MITV-15]] (video 01:17:39–01:23:55)

An integral image $S(x,y)$ stores cumulative intensity over the rectangle from the origin through $(x,y)$:

$$
S(x,y)=\sum_{i\le x,\,j\le y}I(i,j).
$$

For a rectangle with corners $(x_1,y_1)$ and $(x_2,y_2)$, its pixel sum is obtained by inclusion-exclusion:

$$
Q=S(x_2,y_2)-S(x_1-1,y_2)-S(x_2,y_1-1)+S(x_1-1,y_1-1).
$$

After a single $O(wh)$ construction pass, each rectangular sum takes constant time. This is powerful for repeated window statistics and box-like filters, but an integral image does not make arbitrary kernels constant-time, and finite integer storage can overflow. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]]

### Worked example: four-corner rectangle sum

Suppose a $4\times4$ image has values

$$
I=\begin{bmatrix}1&2&0&1\\0&1&3&1\\2&1&0&0\\1&0&2&2\end{bmatrix}.
$$

For the rectangle covering rows 2–3 and columns 2–3 (one-based indexing), the direct sum is $1+3+1+0=5$. The integral image’s four-corner subtraction returns the same sum: cumulative total through the lower-right corner, minus the rectangle above, minus the rectangle left, plus their overlap. The latter plus term is necessary because the upper-left region was removed twice. This inclusion-exclusion pattern generalizes to higher-dimensional summed-area tables.

### An integral image makes every rectangular sum constant-time

For a one-dimensional grayscale sequence $g[i]$, define a cumulative sum $G[i]=\sum_{k\le i}g[k]$. Then an interval sum is a difference, $\sum_{k=i}^{j}g[k]=G[j]-G[i-1]$, independent of interval length. In two dimensions, the summed-area table is

$$
S(x,y)=\sum_{i\le x,\,j\le y}I(i,j).
$$

For a rectangle with lower-right corner $(x_2,y_2)$ and upper-left corner $(x_1,y_1)$, inclusion–exclusion yields

$$
Q=S(x_2,y_2)-S(x_1-1,y_2)-S(x_2,y_1-1)+S(x_1-1,y_1-1).
$$

The top and left exterior regions are subtracted, then their overlap is restored because it was subtracted twice. After one linear-time construction pass, a box average of any size takes four table lookups and a few arithmetic operations. The same idea works for integrated gradient vectors or histograms of oriented gradients, which supports fast rectangular feature summaries on GPUs. Cumulative values need more dynamic range than individual pixels, so 8-bit source storage does not imply that 8-bit accumulators are safe. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]] (video 00:10:39–00:17:04)

### Derivatives and integrals are convolution operators

The impulse $\delta(x)$ is best understood by its sifting property, $\int f(\xi)\delta(x-\xi)\,d\xi=f(x)$. It is a distribution, not an ordinary finite-valued function; a narrow unit-area box or Gaussian can approximate its action in a limiting sense. A finite-difference derivative is convolution with shifted impulses of opposite sign. For step $\epsilon$, one sign convention is

$$
D_\epsilon(x)=\frac{\delta(x+\epsilon/2)-\delta(x-\epsilon/2)}{\epsilon},
$$

which gives a difference quotient when convolved with a signal (the exact sign depends on convolution’s kernel reversal). Convolving this kernel with itself gives a second derivative stencil proportional to $[1,-2,1]/\epsilon^2$. Repeated differences and cumulative sums are inverse operators when boundary conditions and initial values are handled consistently. Their Fourier responses multiply, so a first derivative contributes $j\omega$, a second derivative contributes $-\omega^2$, and integration contributes the reciprocal away from zero frequency. This operator view is what makes convolutional filtering reorderable. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]] (video 00:31:51–00:41:20, 00:47:53–00:54:53)

The defocus example is a two-dimensional circular pillbox rather than a one-dimensional box. A unit-area disk in the frequency domain has radial inverse transform proportional to $J_1(2\pi R\rho)/(\pi R\rho)$, where $R$ is disk radius, $\rho$ is radial distance in the conjugate plane, and $J_1$ is the first-order Bessel function. With normalized frequency response $H(\rho)=2J_1(2\pi R\rho)/(2\pi R\rho)$, the first zero occurs where $2\pi R\rho\approx3.8317$; the zeros are not equally spaced as in a one-dimensional sinc. This is the familiar Airy-pattern mathematics behind circular-aperture resolution. In an actual image, sensors record intensity (squared field magnitude), not signed electric-field amplitude. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]] (video 01:11:25–01:18:00)

An out-of-focus lens ideally convolves the sharp image with a normalized spatial disk of radius $R$, value $1/(\pi R^2)$ inside the disk. Its Fourier transform therefore has a Bessel-shaped response with zero rings. A student measured a DSLR’s unknown focus-motor step by collecting images at successive lens commands and locating the dark frequency rings; after estimating blur radius, known aperture geometry gave the focus-step scale. Real lenses depart from a uniform disk because chromatic and spherical aberration and astigmatism alter the point-spread function; the blur may differ on either side of best focus. A disk is a useful first model rather than a perfect optical law. Likewise, two sequential defocus operations multiply their frequency responses and generally cannot be replaced by one single disk blur: the product’s zero pattern records both operations. That offers a possible cue for detecting a photographed or manipulated image, although noise and lens imperfections complicate the test. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITV-16 - Fast Convolution, Low Pass Filter Approximations, Integral Images (US 6,457,032)|MITV-16]] (video 01:18:00–01:26:12)

## Color perception, encoding, and display

## Make every color number carry a meaning

The workshop's lamp emits a spectrum: a distribution of energy across wavelength. RGB is a chosen three-number representation of perceived color, not a complete spectrum. Different spectra can produce the same RGB response. Such spectra are called metamers. This is why an RGB renderer cannot reproduce every spectral phenomenon merely by increasing the precision of its three channels.

Radiance measures directional light per projected area and solid angle. Solid angle measures the apparent angular size of a region in three dimensions; its unit is the steradian. Irradiance is incident power per surface area, integrated over incoming directions. The surface cosine appears because an oblique beam spreads over more surface area. With normal $N$, incoming radiance $L_i$ and incoming direction $\omega$, irradiance is

$$
E=\int_{\Omega^+}L_i(\omega)\max(0,N\cdot\omega)\,d\omega.
$$

Now separate a physical sum from a storage encoding. A linear value is proportional to the represented signal; adding two linear light contributions has the intended physical meaning. sRGB uses a piecewise nonlinear encoding for storage and display. Decode color textures before lighting and filtering in a linear working representation. Encode only when the destination requires it. Texture coordinates, roughness, IDs and normals are data with their own meanings; applying an sRGB decode to them changes the intended numbers.

Suppose half the sampled area is black and half is unit white. Its linear average is $0.5$. Encoding that average in sRGB produces approximately $0.735357$. Averaging the encoded endpoint values instead produces encoded $0.5$, which decodes to approximately $0.214041$. The dark result is an arithmetic error in the representation, not a material property. Exposure then scales scene-linear values; tone mapping compresses their range for a chosen display. Tone mapping each contribution before summing generally changes the result because the mapping is nonlinear.

Texture sampling is another integration problem. A distant pixel can cover many texels, which are stored texture samples. Bilinear filtering reconstructs from four nearby texels at one level; it does not average an arbitrarily large footprint. Mip levels supply prefiltered scales, while anisotropic filtering handles footprints stretched much more strongly along one direction. Normal maps add a further complication: averaging encoded normals and renormalizing loses information about variation within the footprint. At small scales that variation can behave like additional roughness. This is why normal filtering and specular antialiasing must agree with the material model.

Implementation-specific passages retain observations from the supplied Kairo/Maveb snapshot. They are not a fresh audit of those repositories. Source exercises and illustrative code are teaching material; only explicitly recorded checks have been executed here.


This pass begins Phase 20. **M115 owns Parts 1901–1923**—light/radiometry, color perception, linear RGB/sRGB, color spaces, HDR, tone mapping and exposure. **M116 owns Parts 1924–1944**—textures as sampled signals, filtering, mipmaps, LOD/derivatives, anisotropy, compression, normal/tangent mapping, height/parallax/displacement.

The conceptual transition from Pass 2 is important:

$$
\text{rasterization tells us WHERE to shade}
$$

and now:

$$
\boxed{\text{radiometry + materials + textures tell us WHAT the numbers mean}}
$$

A renderer can have a mathematically perfect rasterizer and still produce completely wrong images if it adds sRGB values, averages encoded mip texels, interprets a normal map as color, uses irradiance where radiance belongs, or destroys scene-linear HDR before post-processing.

---


---

**Light is not RGB**

Real light is electromagnetic radiation.

At a point, light can be described spectrally by wavelength:

$$
\lambda
$$

and a spectral power distribution such as

$$
L(\lambda).
$$

Visible wavelengths occupy only part of the electromagnetic spectrum.

A physically complete renderer would therefore reason about something like

$$
L(\lambda)
$$

at many wavelengths.

Most real-time renderers do not.

They approximate spectral light with three numbers:

$$
(R,G,B).
$$

This is a drastic compression.

It works because human color vision itself approximately reduces spectral information through three cone-response families.

But:

$$
\boxed{\text{RGB is not light itself}}
$$

and

$$
\boxed{\text{different spectra can produce the same RGB/perceived color}}.
$$

That phenomenon is **metamerism**.

This becomes important whenever spectral effects matter:

- dispersion,
- fluorescence,
- wavelength-dependent IOR,
- thin-film interference,
- unusual illuminants.

Our renderer is fundamentally **tristimulus**, not spectral.

---

**Radiometry: the hierarchy**

There are several related quantities, and confusing them causes major rendering mistakes.

We start with radiant energy.

### Radiant energy

Symbol:

$$
Q
$$

Unit:

$$
\mathrm J
$$

It is simply energy carried by electromagnetic radiation.

---

### Radiant flux

Flux is energy per unit time:

$$
\boxed{
\Phi=\frac{dQ}{dt}
}
$$

Unit:

$$
\mathrm W = \mathrm{J/s}.
$$

A 10 W optical emitter emits ten joules of radiant energy every second.

Flux answers:

> How much total radiant power is flowing?

It says nothing yet about area or direction.

---

**Irradiance**

Suppose flux lands on a surface.

Irradiance is incident flux per unit surface area:

$$
\boxed{
E=\frac{d\Phi}{dA}
}
$$

with units

$$
\mathrm{W/m^2}.
$$

If

$$
100\,\mathrm W
$$

lands uniformly on

$$
10\,\mathrm{m^2},
$$

then

$$
E=10\,\mathrm{W/m^2}.
$$

Irradiance answers:

> How much incoming power reaches this patch?

This quantity integrates incoming light from many directions.

---

**Solid angle**

Before radiance makes sense, we need angular measure.

In 2D, angle in radians is

$$
\theta=\frac{s}{r}.
$$

In 3D, solid angle is

$$
\boxed{
\Omega=\frac{A_{\text{sphere}}}{r^2}
}
$$

where $A_{\text{sphere}}$ is the area cut out on a sphere of radius $r$.

Unit:

$$
\mathrm{sr}
$$

or steradians.

A complete sphere subtends

$$
4\pi
$$

steradians.

A hemisphere subtends

$$
2\pi.
$$

For differential solid angle:

$$
d\omega
=
\sin\theta\,d\theta\,d\phi.
$$

This is why hemisphere integrals later contain

$$
\sin\theta.
$$

---

**Radiance — the central rendering quantity**

Radiance describes power with both **spatial and directional density**.

Formally:

$$
\boxed{
L=
\frac{d^2\Phi}
{dA_\perp\,d\omega}
}
$$

where

$$
dA_\perp
=
dA\cos\theta.
$$

Therefore:

$$
\boxed{
L=
\frac{d^2\Phi}
{\cos\theta\,dA\,d\omega}
}
$$

with units

$$
\mathrm{W\,m^{-2}\,sr^{-1}}.
$$

Interpretation:

> How much radiant power travels through an infinitesimal projected surface area inside an infinitesimal direction range?

This is the quantity most directly associated with a ray.

When a path tracer stores:

```cpp
Vec3 radiance;
```

its conceptual RGB components are approximations to spectral radiance.

---

**Why the cosine appears**

Consider an area patch $dA$.

When viewed head-on:

$$
\theta=0,\qquad\cos\theta=1.
$$

Its projected area is

$$
dA_\perp=dA.
$$

Turn it almost edge-on:

$$
\theta\rightarrow90^\circ.
$$

Then:

$$
\cos\theta\rightarrow0
$$

and its projected area tends toward zero.

This is fundamentally geometric.

The same cosine will reappear in the rendering equation:

$$
L_o=
L_e+
\int_\Omega
f_r
L_i
(\mathbf n\cdot\omega_i)
\,d\omega_i.
$$

Do not think of that cosine as “a Lambert lighting trick.”

It comes from projected area geometry.

---

**Irradiance from radiance**

Incoming radiance may arrive from many directions.

Irradiance is therefore:

$$
\boxed{
E(\mathbf x)
=
\int_{\Omega^+}
L_i(\mathbf x,\omega_i)
(\mathbf n\cdot\omega_i)
\,d\omega_i
}
$$

where

$$
\Omega^+
$$

is the hemisphere above the surface.

If incoming radiance is constant:

$$
L_i=L
$$

over the entire hemisphere, then:

$$
E=
L
\int_{\Omega^+}\cos\theta\,d\omega.
$$

Using

$$
d\omega=\sin\theta\,d\theta\,d\phi,
$$

we get

$$
E=
L
\int_0^{2\pi}
\int_0^{\pi/2}
\cos\theta\sin\theta
\,d\theta\,d\phi.
$$

The inner integral is:

$$
\int_0^{\pi/2}
\cos\theta\sin\theta\,d\theta
=
\frac12.
$$

Therefore:

$$
E=
L(2\pi)\frac12
$$

so:

$$
\boxed{
E=\pi L.
}
$$

That $\pi$ will become important when Lambertian BRDF appears next pass.

---

**Radiance invariance along a vacuum ray**

One of the most powerful properties in rendering is:

$$
\boxed{
L(\mathbf x,\omega)
=
L(\mathbf x+t\omega,\omega)
}
$$

for unobstructed propagation through vacuum.

Radiance does **not** fall with inverse square along a ray.

This initially sounds wrong because we know light intensity falls with distance.

The distinction is crucial.

A point emitter spreads its total flux across a growing sphere:

$$
A=4\pi r^2.
$$

Therefore irradiance on a receiving surface decreases approximately as:

$$
E\propto\frac1{r^2}.
$$

But radiance measures directional density.

As distance grows:

- apparent angular size shrinks,
- geometric projected area relationship changes,

in exactly the way required for radiance to remain unchanged along free propagation.

This is why a distant surface does not become visually dimmer merely because it is farther away, assuming no atmosphere and equal projected appearance.

It occupies fewer pixels, but its radiance per corresponding viewing ray remains the same.

---

**The rendering ray interpretation**

Suppose your camera ray hits a diffuse white wall.

A renderer eventually computes something like:

$$
L_o(\mathbf x,\omega_o).
$$

That radiance travels through empty space to the camera without changing.

Therefore:

```text
surface computes outgoing radiance
           ↓
vacuum transport
           ↓
camera receives same radiance
           ↓
sensor / framebuffer representation
```

This is precisely why radiance is such a convenient quantity for ray/path tracing.

---

**Human color perception**

The retina contains several photoreceptor classes.

For photopic color vision, three cone types dominate:

$$
L,\quad M,\quad S
$$

with different spectral response curves.

A spectral stimulus

$$
S(\lambda)
$$

generates cone responses roughly like:

$$
R_L=
\int S(\lambda)\bar l(\lambda)\,d\lambda,
$$

$$
R_M=
\int S(\lambda)\bar m(\lambda)\,d\lambda,
$$

$$
R_S=
\int S(\lambda)\bar s(\lambda)\,d\lambda.
$$

This maps an effectively enormous-dimensional spectral function to three response values.

Therefore many different spectra map to the same three responses.

Hence metamers.

Rendering with RGB exploits this dimensional reduction.

---

**Tristimulus representation**

A tristimulus system chooses three basis responses.

Common examples include:

$$
XYZ
$$

and

$$
RGB.
$$

CIE XYZ is especially important because it provides a standardized colorimetric space.

RGB spaces choose actual or conceptual red, green and blue primaries.

A color becomes coordinates relative to those basis primaries.

For example:

$$
\mathbf c=
\begin{bmatrix}
R\\G\\B
\end{bmatrix}.
$$

But these numbers are meaningless without knowing:

- primaries,
- white point,
- transfer function.

Thus:

$$
(1,0,0)
$$

in sRGB does not represent the same physical chromaticity as

$$
(1,0,0)
$$

in Display P3.

---

**Linear RGB**

A linear RGB space is one in which arithmetic corresponds linearly to light.

If:

$$
C_2=2C_1,
$$

then conceptually it represents twice the channel contribution.

That means operations such as:

$$
C=C_1+C_2
$$

and

$$
C=kC_1
$$

have sensible physical meaning.

This is what lighting needs.

Your own `KairoRenderer::PBRMaterial` explicitly documents:

```text
BaseColor = non-negative linear RGB
Emissive  = non-negative linear RGB
```

and `KairoRayTracer::Color3f` similarly establishes:

```text
internal color = linear RGB
values may exceed [0,1]
```

That is the correct internal convention.

---

**sRGB is not linear RGB**

sRGB uses the same primaries and D65 white point for both encoded and linear-light forms, but applies a nonlinear transfer function for stored/display-oriented values. Current W3C color specifications describe linear-sRGB as the same primaries/white point with a unity transfer function and sRGB as the encoded form. [W3C](https://www.w3.org/TR/css-color/?utm_source=chatgpt.com)

For encoded component $C_s$, decoding to linear light gives:

$$
\boxed{
C_l=
\begin{cases}
\frac{C_s}{12.92},
&
C_s\le0.04045
\\[6pt]
\left(
\frac{C_s+0.055}{1.055}
\right)^{2.4},
&
C_s>0.04045
\end{cases}
}
$$

The inverse conversion is:

$$
\boxed{
C_s=
\begin{cases}
12.92C_l,
&
C_l\le0.0031308
\\[6pt]
1.055C_l^{1/2.4}-0.055,
&
C_l>0.0031308.
\end{cases}
}
$$

These are the exact piecewise transfer functions used by KairoAssets.

---

**“Gamma 2.2” versus actual sRGB**

You often see:

$$
C_l\approx C_s^{2.2}.
$$

That is an approximation.

sRGB is not exactly a pure $2.2$ power function.

The lower segment is linear, and the upper exponent is 2.4 with offsets.

Interestingly, your repositories currently show both approaches.

`KairoAssets/TextureArtifact.cppm` uses the proper piecewise conversion:

```cpp
value <= 0.04045
    ? value / 12.92
    : pow((value + 0.055) / 1.055, 2.4);
```

while `KairoRayTracer/Color.cppm` currently encodes PPM output using:

```cpp
pow(linear, 1.0f / 2.2f);
```

So the asset pipeline is proper sRGB, while the simple ray-tracer PPM output uses a gamma-2.2 approximation.

That distinction is useful rather than merely academic.

---

**Why lighting in sRGB is wrong**

Suppose two lights independently contribute linear intensity

$$
0.25
$$

each.

Correct linear addition:

$$
0.25+0.25=0.5.
$$

Encode afterward.

But if 0.25 were first converted to an sRGB-like encoded value, roughly:

$$
sRGB(0.25)\approx0.537.
$$

Adding the encoded values gives:

$$
0.537+0.537=1.074.
$$

Clamping produces:

$$
1.
$$

That implies white instead of the correct encoded representation of linear 0.5:

$$
sRGB(0.5)\approx0.735.
$$

Massive error.

Therefore:

$$
\boxed{
\text{decode}
\rightarrow
\text{lighting in linear space}
\rightarrow
\text{tone map}
\rightarrow
\text{display encode}
}
$$

is the correct broad pipeline.

---

**The classic midpoint error**

Imagine averaging black and white texels.

Encoded values:

$$
0,\quad1.
$$

Naive encoded-space average:

$$
0.5.
$$

But decoding sRGB 0.5 gives approximately:

$$
0.214.
$$

So the “average” contains only about 21% linear light.

Correct procedure:

$$
0\rightarrow0_{\text{linear}}
$$

$$
1\rightarrow1_{\text{linear}}
$$

then:

$$
\frac{0+1}{2}=0.5_{\text{linear}}.
$$

Encoding linear 0.5 gives approximately:

$$
\boxed{0.735_{\text{sRGB}}}.
$$

That is a huge practical difference.

This applies to:

- texture filtering,
- mipmap generation,
- image resizing,
- compositing.

---

**Kairo gets this mipmap case right**

The current `BuildTextureMipChainRGBA8()` path explicitly:

1. decodes RGB texels from sRGB,
2. averages them in linear space,
3. re-encodes the mip texel to sRGB.

It also leaves alpha outside the sRGB transfer, which is correct because alpha is coverage/opacity data, not color energy.

This is exactly what we want.

---

**Hardware sRGB formats**

Kairo's Vulkan texture mapping chooses:

```cpp
VK_FORMAT_R8G8B8A8_SRGB
```

for sRGB artifacts and:

```cpp
VK_FORMAT_R8G8B8A8_UNORM
```

for linear ones.

The advantage is that the shader can write:

```glsl
vec3 color = texture(baseColorMap, uv).rgb;
```

and receive a linearized color when using an sRGB texture view.

The color transfer conversion happens as part of texture access.

This prevents every shader from manually implementing:

```glsl
pow(texture(...), 2.2);
```

which would be both wrong and repetitive.

---

**Data textures must not be sRGB**

Imagine roughness value:

$$
r=0.5.
$$

That number means:

> parameter value 0.5.

It does not mean:

> display-encoded gray corresponding to some radiometric intensity.

If you decode it as sRGB:

$$
0.5\rightarrow0.214.
$$

You have just changed the material parameter.

The same problem applies to:

- normal maps,
- metallic maps,
- roughness maps,
- occlusion,
- masks,
- lookup tables.

Thus your KairoAssets validation enforces:

```text
Normal → Linear
Data   → Linear
HDR    → Linear
```

and rejects sRGB for those semantics.

That is a very good invariant.

---

**Primaries**

An RGB color space is defined partly by three chromaticity primaries.

For sRGB they are approximately:

$$
R=(0.640,0.330),
$$

$$
G=(0.300,0.600),
$$

$$
B=(0.150,0.060).
$$

A Display-P3 red primary is different.

Therefore even if both values are linear:

```text
linear-sRGB   (1,0,0)
linear-P3     (1,0,0)
```

they represent different chromaticities.

A transfer function alone does not define a color space.

---

**White points**

The white point specifies which chromaticity should appear neutral.

sRGB uses D65.

Conceptually, white point defines the reference:

$$
R=G=B
$$

that should appear achromatic under the target viewing/colorimetric model.

When converting between spaces with different white points, chromatic adaptation may be required.

This matters in production pipelines involving:

```text
sRGB
Display P3
ACEScg
Rec.2020
XYZ
```

not merely in photo-editing software.

---

**HDR numbers**

Suppose a shader produces:

$$
C=(5.4,1.2,0.3).
$$

This is not “invalid RGB.”

In scene-linear HDR rendering:

$$
C>1
$$

is perfectly ordinary.

The value 1 is not the maximum amount of light in nature.

The constraint:

$$
0\le C\le1
$$

is typically associated with limited output encoding/storage, not with the lighting calculation itself.

This is why floating-point framebuffer formats are useful.

---

**HDR rendering is not the same as HDR display**

These are different concepts.

**Scene-linear HDR rendering**

The renderer allows values like:

$$
0.001,\quad0.5,\quad2,\quad100,\quad10000.
$$

**HDR intermediate buffer**

The framebuffer preserves a useful wide numerical range, usually using floating point.

**HDR display output**

The final output targets HDR transfer functions/gamuts such as PQ/HLG and actual HDR-capable displays.

You can have:

```text
HDR scene-linear rendering
→ tone map
→ normal SDR sRGB monitor
```

and that is still an HDR rendering pipeline internally.

---

**Exposure**

Camera exposure is fundamentally a scale applied before display mapping.

A convenient rendering model is:

$$
\boxed{
C_{\text{exposed}}
=
C_{\text{scene}}2^{EV}
}
$$

where increasing EV by 1 doubles brightness:

$$
EV+1
\Rightarrow
2C.
$$

Reducing by one stop:

$$
EV-1
\Rightarrow
\frac12C.
$$

Your Kairo shader currently performs exactly:

```glsl
hdrColor =
    sceneColor *
    exp2(camera.viewportParameters.y);
```

Since:

$$
\operatorname{exp2}(x)=2^x,
$$

that field is effectively an EV exposure control.

---

**Tone mapping**

Your screen cannot directly represent arbitrarily large scene radiance.

We need:

$$
\boxed{
T:
[0,\infty)
\rightarrow
[0,1]
}
$$

or toward whatever display-referred output range is appropriate.

A simple Reinhard curve is:

$$
\boxed{
T(x)=\frac{x}{1+x}
}
$$

Properties:

$$
T(0)=0,
$$

$$
T(1)=0.5,
$$

$$
T(9)=0.9,
$$

$$
T(x)\rightarrow1
\quad \text{as }x\rightarrow\infty.
$$

So highlights compress smoothly rather than clipping.

---

**Clipping versus tone mapping**

Hard clipping:

$$
T(x)=\min(x,1)
$$

maps:

$$
1\rightarrow1,
$$

$$
2\rightarrow1,
$$

$$
1000\rightarrow1.
$$

All highlight information collapses.

Reinhard:

$$
1\rightarrow0.5,
$$

$$
2\rightarrow0.667,
$$

$$
10\rightarrow0.909,
$$

$$
100\rightarrow0.990.
$$

Relative highlight structure remains.

---

**Filmic tone mapping**

Real film/camera response inspired curves tend to have:

- **toe** near black,
- reasonably linear midtone region,
- **shoulder** compressing highlights.

Conceptually:

```text
output
1 |                   ______
  |                __/
  |             __/
  |          __/
  |       __/
  |    __/
0 |___/
  +-------------------------- input
```

The point is not “make it cinematic.”

The objective is often to preserve:

- midtone contrast,
- highlight rolloff,
- shadow detail,
- acceptable colors.

Common production operators include ACES-inspired fits, Hable/Uncharted-style curves and custom display transforms.

---

**Per-channel tone mapping and hue shifts**

Kairo currently performs:

```glsl
outColor.rgb =
    hdrColor /
    (hdrColor + vec3(1.0));
```

independently per RGB component.

Consider:

$$
C=(100,10,1).
$$

Reinhard gives:

$$
(0.9901,0.9091,0.5).
$$

The channel ratios changed drastically.

Therefore saturation/hue can shift in highlights.

More sophisticated pipelines may operate using luminance/chroma or color-management transforms designed to preserve appearance better.

This will matter if you eventually pursue film-quality rendering.

---

**A subtle current Kairo architecture issue**

Your viewport uses:

```text
RGBA16_FLOAT
```

which is an HDR-capable storage format.

But `triangle.frag` does:

```glsl
hdrColor = ...;
outColor = hdrColor / (hdrColor + 1.0);
```

**before writing into that float target.**

Therefore the target receives approximately:

$$
0\le C<1.
$$

So Kairo has:

```text
HDR lighting arithmetic
        ↓
exposure
        ↓
Reinhard
        ↓
RGBA16F viewport
```

not:

```text
HDR lighting arithmetic
        ↓
RGBA16F scene target
        ↓
post-processing
        ↓
exposure
        ↓
tone mapping
        ↓
display
```

The latter is generally the more flexible production architecture because bloom, exposure adaptation and other HDR operations can consume the real HDR image.

This is an excellent future refactor candidate.

---

**Canonical exact sRGB code**

```cpp
float SRGBToLinear(float encoded)
{
    // sRGB transfer function.
    // Input/output here are normalized floating-point channels.
    if (encoded <= 0.04045f)
        return encoded / 12.92f;

    return std::pow(
        (encoded + 0.055f) / 1.055f,
        2.4f);
}

float LinearToSRGB(float linear)
{
    // Negative display values require a defined gamut/output policy.
    // This simple SDR helper assumes non-negative values.
    linear = std::max(linear, 0.0f);

    if (linear <= 0.0031308f)
        return 12.92f * linear;

    return
        1.055f *
        std::pow(linear, 1.0f / 2.4f)
        - 0.055f;
}
```

The current W3C definition uses this same piecewise relationship rather than a pure 2.2 power. [W3C](https://www.w3.org/TR/2026/CRD-css-color-4-20261009/?utm_source=chatgpt.com)

---


---


Color values depend on physical spectra and encoding conventions, so operations must respect measurement and display stages.

### Color as spectral measurement

Light is electromagnetic radiation described by wavelength and power. A spectrum can be emitted, absorbed, reflected, or transmitted by materials. The eye’s rods and cones and a camera’s sensors respond differently across wavelengths. The perceived or recorded color is a measurement of the spectrum by those responses, not an intrinsic label attached to a surface. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-14 - Color|CMU-14]]

Standard color-matching functions convert a spectrum $S(\lambda)$ into tristimulus values:

$$
X=\int S(\lambda)\bar x(\lambda)\,d\lambda,\quad
Y=\int S(\lambda)\bar y(\lambda)\,d\lambda,\quad
Z=\int S(\lambda)\bar z(\lambda)\,d\lambda.
$$

Different spectra can have the same three integrals and therefore look alike to a standard observer under those conditions. They are metamers; changing the illuminant or camera response can reveal their difference. Chromaticity removes overall intensity, with coordinates $x=X/(X+Y+Z)$ and $y=Y/(X+Y+Z)$. A display gamut is the set of colors obtainable from its primaries. CIE diagrams describe relationships among visible colors, while sRGB defines a practical display encoding. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-14 - Color|CMU-14]]

RGB is convenient for displays, CMYK for subtractive printing, HSV-like models for intuitive editing, and Y'CbCr for separating luma-like and chroma channels. Chroma can often be sampled more coarsely because visual acuity for color detail is lower than for brightness detail. These models solve different tasks; none is the original spectrum. Gamma encoding stores display values nonlinearly. Physical addition, interpolation, filtering, and compositing should use linear-light values, then encode for display. Averaging encoded black and white values directly yields a darker result than averaging their physical light. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-14 - Color|CMU-14]]

### Additive and subtractive mixtures

An additive display emits red, green, and blue light. Increasing all three toward maximum produces white. Printing is subtractive: cyan, magenta, and yellow inks absorb portions of incident white light; combining them ideally absorbs most visible wavelengths and produces dark output. In practice, black ink is added for deeper blacks and efficient neutral printing, giving CMYK. These models describe different physical mechanisms. A color slider that looks intuitive to a designer is not necessarily a model in which arithmetic represents physical mixing. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-14 - Color|CMU-14]]

### Chromaticity, gamut, and color conversion

A chromaticity plot can show the visible spectral locus and the triangle spanned by a display’s primaries. Colors outside that triangle are out of gamut. Color conversion between RGB spaces is a linear matrix transform only after values are in the appropriate linear-light representation and white-point assumptions are handled. A nonlinear encoding such as sRGB applies a transfer curve after the matrix conversion.

An image can store sRGB code values for display. If two stored values $c_1,c_2$ are averaged directly, the result does not represent the mean radiance unless the encoding is linear. The correct process is decode, compute in linear light, then re-encode. This applies to texture filtering, physically based light accumulation, image resampling, and alpha compositing. Y'CbCr uses a luma-like channel and color-difference channels. Since human vision resolves brightness detail more finely than chroma detail, chroma subsampling can reduce storage while preserving apparent sharpness. The apostrophe on $Y'$ signals that the quantity is formed from nonlinear encoded components, rather than strict linear luminance. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-14 - Color|CMU-14]]

### Color: device values are not perceived color

The human visual system has overlapping cone responses rather than one receptor for each display primary. A screen uses a small set of primaries whose combinations stimulate those cones; many distinct spectra can produce the same cone responses. Those spectra are metamers: perceptually matching colors under a given viewing condition despite different physical spectral distributions. RGB is therefore a device-oriented coordinate system, not a universal description of color. CIE spaces use standard observer response functions to describe color more independently of one particular display. The 2021 color lecture discusses rods and cones, wavelength responses, color matching, metamerism, and color storage [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-20 - Color|MITG-20]] (00:15:00–00:40:00; 00:45:00–01:05:00).

Color pipelines also need a transfer function. Most display RGB values are nonlinear encodings. If encoded channel values are averaged directly, the result is not generally the average light energy. For example, averaging black and white in a gamma-like encoded space gives a visibly different midpoint than averaging their linear-light values and encoding the result. Correct compositing, filtering, and illumination calculations should use linear-light values; the transfer function maps those values to the device's encoded range. A “gamma correction” is not an aesthetic knob but part of interpreting stored numbers as light. The lecture's discussion of what is stored in practice grounds the distinction between perceived color, physical intensity, and display encoding [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-20 - Color|MITG-20]] (00:55:00–01:05:00).

With the lecture's approximate gamma value of 2.2, a linear-light intensity of 0.5 is encoded as roughly 0.5^(1/2.2) ≈ 0.73 (about 186 on an 8-bit 0–255 scale). Conversely, interpreting the code value 0.5 as light gives about 0.5^2.2 ≈ 0.22. This is why averaging encoded black and white values to get 0.5 does not produce half the light; it produces only about 22% of full linear intensity. The exact sRGB transfer is piecewise, but the approximate gamma calculation makes the direction and size of the error tangible.

### Output devices close the loop

A pixel array is an abstract grid of color values; a physical output device has to turn those values into light or reflected light. That conversion varies sharply by technology. The 2021 output-device lecture surveys 2D displays, beginning with cathode-ray tubes (CRTs), whose electron beam scans across phosphors, and liquid-crystal displays (LCDs), which modulate light from a backlight. It then discusses LED and plasma displays, digital light processing (DLP) projectors, and electronic ink. These differ in how they generate, modulate, or reflect light, and in properties such as refresh behavior, contrast, viewing angle, and how a pixel is physically composed. The example of phosphor persistence makes the device's time response concrete: a displayed image is not only spatially sampled but emitted through material that may continue glowing after excitation. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-22 - Output devices|MITG-22]] (00:11:00–00:27:00; 00:30:00–00:38:00)

The lecture then asks what a “3D display” actually does. A lenticular display uses an optical layer to direct different image samples toward different viewing directions; separate views give each eye a slightly different image and can create a depth cue. This has a resolution cost: dividing a panel's output among multiple views reduces the detail available in any one view. The lecture compares this with projector-based and head-mounted display arrangements, including the Oculus / HMD example, where separate images are placed close to each eye and must be registered to the viewer's geometry. The lesson is not simply a catalog of screens: output technology constrains the image representation, the viewing zone, and the spatial detail that reaches a person. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-22 - Output devices|MITG-22]] (00:39:00–00:56:00; 01:00:00–01:08:00)

The OCW 2012 notes page marks its own Output Devices lecture notes as unavailable. That is an archive gap only; the Fall 2021 spoken discussion is available in the video transcript. The distinctions above are drawn from that 2021 transcript, not reconstructed from the 2012 course.

### Color is a three-number perceptual response to a spectrum

A small discrete example makes the compression concrete. Sample a spectrum at four wavelengths and suppose the cone-response matrix is

$$
Q=\begin{bmatrix}1&0.6&0.1&0\\0&0.4&1&0.5\\0.2&1&0.2&0\end{bmatrix}.
$$

For spectrum $s=(1,0,0,0)^T$, cone values are $Qs=(1,0,0.2)^T$. For $s'=(0,0,1,0)^T$, they are $(0.1,1,0.2)^T$, so these are not metamers. But any spectral change $d$ in the nullspace of $Q$ leaves perception unchanged: $Q(s+d)=Qs$ when $Qd=0$. The nullspace exists because four independent spectral samples are projected to only three measurements. Real sensitivity curves are continuous, but the same dimensionality argument applies. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-20 - Color|MITG-20]] (00:30:59–00:35:59)

The eye also allocates its sensors unevenly. The lecture connects greater sensitivity to luminance and fine edges than to rapid color changes with engineering choices such as multisample antialiasing, and with processing an image’s brightness channel while leaving chroma detail less finely filtered. A bilateral filter smooths brightness while preserving edges; if applied to a luminance-like component alone, high-frequency color differences can remain, often with a smaller computational cost than filtering three channels. The fovea is cone-dense and supports detailed color vision; the periphery is more rod-dominated and less color-sensitive. Foveated rendering uses eye tracking to spend shading and sampling effort near gaze and reduce it outside. Its practical constraint is latency: gaze can move before the high-quality region catches up. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-20 - Color|MITG-20]] (00:14:00–00:23:46)

Color depends on both the illuminant and the material. Their wavelength-dependent interaction determines the spectrum reaching the eye. The same two objects can match under one lamp and differ under another, because a lamp may lack wavelengths that distinguish their reflectances. This is why the “color of an object” cannot always be treated as a fixed RGB triplet independent of lighting. The lecture uses the ambiguous dress photograph to illustrate how the visual system infers illumination from context and can settle on different interpretations when the context is unclear. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-20 - Color|MITG-20]] (00:36:08–00:38:42)

### Output devices turn samples into emitted or reflected light

The graphics pipeline ends in hardware that transforms a numeric pixel into light reaching an observer. A CRT scans an electron beam across phosphors; magnetic fields deflect the electrons and therefore shift the beam to the wrong phosphor. An LCD instead modulates a backlight: a first polarizer selects an orientation, liquid-crystal molecules twist or preserve that polarization under an applied voltage, and a second polarizer passes or blocks the result. A color LCD overlays RGB filters on three spatially offset subpixel grids. This geometry explains why text benefits from subpixel antialiasing: ClearType-style rendering adjusts red, green, and blue subpixel coverage separately to make apparent horizontal edge placement finer than one full pixel, provided the display’s RGB order is known. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-22 - Output devices|MITG-22]] (00:15:00–00:24:56)

Other devices produce light differently. A direct-view LED wall places emitters at the image locations, giving high brightness but requiring physical space for each emitter. In a plasma panel, an electrical discharge excites gas; returning electrons emit ultraviolet photons that excite a phosphor subpixel. A DLP projector reflects a bright source from a grid of tiny tilting mirrors toward or away from the screen. Each mirror’s fast duty cycle controls average brightness. To produce color, a spinning RGB filter wheel synchronizes with mirror modulation. E-ink uses charged pigment particles in microcapsules: switching voltage moves dark/light particles, and the image persists with little energy until the next update. This makes it readable and power-efficient for static pages but too slow for smooth animation. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-22 - Output devices|MITG-22]] (00:25:18–00:37:22)

A static color sample is never independent of device physics. LCD filters spatially offset channels, plasma and LED emit light directly, and a projector’s mirrors redirect rather than create photons. Thus, a pixel is an instruction interpreted by a calibrated output pipeline, not a tiny abstract square that every device reproduces identically. The lecture’s device tour connects display hardware back to the earlier issues of RGB primaries, spatial sampling, persistence, refresh, and power. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-22 - Output devices|MITG-22]] (00:18:04–00:37:22)

Three-dimensional output is harder because a conventional flat panel emits essentially one view per location. Stereoscopic displays send a distinct image to each eye, creating binocular disparity while still reflecting from a flat screen. Head-mounted VR places a display close to each eye and updates the view with head motion; mismatched depth cues, latency, vergence, and focus can cause discomfort. Autostereoscopic or lenticular displays direct different pixel columns into different viewing directions, approximating a light field—the radiance $L(x,y,z,\theta,\phi)$ arriving at a position from a direction. A multi-view display must render and allocate resolution across several views, so each view becomes lower resolution and the effect works only from intended angles. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-22 - Output devices|MITG-22]] (00:38:30–00:52:05; 00:57:46–01:04:11)

A swept-volume display trades time for depth: a fast rotating mirror or screen shows successive two-dimensional slices at different positions, and persistence of vision fuses them into a volume. It must refresh fast enough to avoid flicker and temporal aliasing, and moving parts create safety and resolution constraints. A static-volume display stacks transparent image planes, but transparent layers make occlusion difficult because rear imagery can leak through. Pepper’s Ghost is a simpler optical illusion: a bright offstage image reflects from angled glass and overlays the real scene. Phone-based augmented reality uses a camera view as a window, then inserts rendered objects; believable registration, lighting and depth alignment are needed for the digital content to sit in the captured world. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-22 - Output devices|MITG-22]] (00:52:05–01:08:26)

The design lesson is to match display technology to the task. A static e-reader benefits from persistent reflective pigment; a stadium needs bright direct emitters; precise small text can exploit subpixel geometry; immersive VR needs low-latency per-eye views; and an all-around light-field display spends resolution and computation to support motion parallax. No format erases the physical tradeoffs in brightness, spatial resolution, temporal resolution, viewing angle, occlusion, and interaction. The final device is the last stage of rendering: a numerical image becomes a physical stimulus, with the observer completing the pipeline. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#MITG-22 - Output devices|MITG-22]] (00:25:18–00:37:22; 00:38:30–01:08:26)

### Color is a measurement and a convention

$$
L(\lambda)\propto E(\lambda)R(\lambda).
$$

A red object under a source that contains little red energy can appear dark because the object cannot reflect wavelengths the source does not provide. This is why an object’s color cannot be described by reflectance alone; illumination matters. A maroon vehicle under narrow sodium lighting can nearly disappear into black, while its appearance changes under daylight. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-12 - Photography session 12|PHOTO-12]]

Human color vision reduces a continuous spectrum to three cone responses, often labeled long-, medium-, and short-wavelength channels. For sensitivity functions $s_i(\lambda)$, each channel response is an integral:

$$
c_i=\int E(\lambda)R(\lambda)s_i(\lambda)\,d\lambda.
$$

Different spectra can produce the same three responses. Such stimuli are metamers: indistinguishable under one observer and viewing condition despite different physical spectra. Metamerism makes color reproduction possible with a few primaries, but it also creates failures when illumination or observer changes. Maxwell’s color-matching experiment used three adjustable primaries to match a single-wavelength test light. Some test wavelengths could not be matched by adding only nonnegative amounts of the chosen primaries; the experimenter had to add a primary to the test side. This “negative primary” is a mathematical way to express that the test color lies outside the triangle spanned by the display primaries. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-12 - Photography session 12|PHOTO-12]]

### From scene spectrum to display color

Use a simplified three-channel camera to follow color through the pipeline. At each wavelength, scene illumination and surface reflectance determine the light arriving at the sensor. Each color filter transmits part of that spectrum, then the photosensitive material converts some arriving photons into charge. The raw mosaic therefore gives sparse, differently filtered samples. Demosaicing estimates a red, green, and blue value at every pixel; white balance scales channels; a camera color matrix maps the camera’s sensor responses toward a standard RGB space; tone mapping and a nonlinear transfer curve make the values suitable for a display; JPEG may then subsample chroma and quantize transform coefficients. Errors at one stage can be hidden or magnified later.

Take an example with a white card under a warm indoor lamp. The card reflects broadly, but the lamp’s spectrum is weighted toward longer wavelengths. The raw red channel may be high relative to blue. If the photographer selects the card as neutral, white balance reduces red relative to blue so that the card appears near the chosen reference white. That operation also changes every other object’s channels. If the scene contains a deliberately warm candle flame, full neutralization can erase the warmth the viewer expects. Therefore the algorithm can estimate a neutral illuminant, but choosing how much to compensate is partly a scene interpretation and aesthetic decision. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-12 - Photography session 12|PHOTO-12]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-13 - Photography session 13|PHOTO-13]]

Now place a red object under a narrowband source with little red energy. The product $E(\lambda)R(\lambda)$ becomes small even though the object’s reflectance is high in red wavelengths. A camera cannot recover missing illumination by white balancing without amplifying noise dramatically. A source with a broad spectrum gives the object energy to reflect; a narrow sodium source does not. This makes light quality distinct from simple white balance. The same reason that some paints and fabrics change appearance between daylight and fluorescent lighting is metamerism: the source spectrum changes the three integrated responses. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-12 - Photography session 12|PHOTO-12]]

A display gamut example clarifies why color conversion is not just changing numeric labels. Suppose a saturated green object lies outside a printer’s reproducible gamut. There is no exact printer ink mixture for that chromaticity. A rendering intent may pull it toward the printer’s gamut boundary, preserving hue as much as possible, or compress a larger range of colors together so relationships are preserved. If the conversion is done in a perceptually uniform space, geometric compression better approximates a controlled visual change than a straight-line movement in a nonuniform chromaticity diagram. However, the result still depends on paper, ink, viewing illumination, profile, and the chosen intent. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-13 - Photography session 13|PHOTO-13]]

### Color matrices and nonlinear encodings

A color space conversion between two linear three-channel representations can often be written as a matrix multiplication:

$$
\begin{bmatrix}R_2\\G_2\\B_2\end{bmatrix}
=M\begin{bmatrix}R_1\\G_1\\B_1\end{bmatrix}.
$$

The matrix depends on the source and destination primaries and reference whites. White adaptation may be required when the reference white changes. A nonlinear transfer function is then applied to encode values efficiently for storage and display. Inverting the transfer function returns to a linear-light domain for operations such as physically meaningful blending or averaging. Blending encoded sRGB values directly can produce darker or otherwise incorrect results because equal code-value differences are not equal radiometric differences. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-13 - Photography session 13|PHOTO-13]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

Luma is a weighted combination of gamma-encoded RGB channels in a video or JPEG workflow; it approximates brightness for coding purposes but is not the same quantity as physical luminance. Chroma channels represent color differences and can be spatially subsampled because small color-detail losses are less noticeable than comparable luma losses. A 4:2:0 layout stores one luma value per pixel but one sample of each chroma component for a $2\times2$ block. This is an efficient storage trade, not a sensor architecture. Bayer sampling occurs at capture; JPEG chroma subsampling occurs later in encoding. Confusing them leads to poor diagnosis of color artifacts. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-08 - Photography session 08|PHOTO-08]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-13 - Photography session 13|PHOTO-13]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

### Color photographs are a sequence of measurements and reconstructions

Silicon sensors respond broadly across visible wavelengths; an electron count alone does not identify the photon’s wavelength. Color capture must make separate spectral measurements. In a Bayer camera, red, green, and blue filters select wavelengths at different sites, then demosaicing estimates missing channel values. Green is sampled more densely because spatial detail is especially important to perceived luminance. Other architectures make different compromises: a dichroic prism can route spectral bands by reflection and transmission to three sensors instead of absorbing unwanted bands at each photosite, though real coatings still have losses and the arrangement adds bulk and alignment cost; stacked silicon layers infer color from wavelength-dependent penetration depth but may have different noise behavior; sequential color capture measures filters at different times and can create motion artifacts. These RGB architectures measure a few spectral responses; they do not uniquely identify a continuous spectrum. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-08 - Photography session 08|PHOTO-08]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-12 - Photography session 12|PHOTO-12]]

The history of color photography makes those assumptions tangible. Maxwell’s 1861 demonstration recorded three filtered monochrome views and recombined them by projection. Ducos du Hauron patented a three-color process in 1868 and published its explanation in 1869. [The Metropolitan Museum’s object record](https://www.metmuseum.org/art/collection/search/286318) distinguishes these dates from later surviving prints. Prokudin-Gorsky made sequential red, green, and blue exposures of scenes in Russia; modern alignment and color rendering can recombine them, though moving subjects can separate into colored ghosts. Early Kinemacolor alternated red and green capture and projection; three-strip Technicolor used beam splitters and separate records; single-strip color film encoded several color layers together. Each method trades capture time, registration, light efficiency, equipment complexity, and color reproduction. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-08 - Photography session 08|PHOTO-08]]

### Additive and subtractive mixing are different operations

Superimposed lights add their spectral power distributions: two colored beams can combine to produce a brighter stimulus. Layered dyes and filters multiply their wavelength-dependent transmissions: a cyan filter passes much blue and green while suppressing red, and adding yellow suppresses more blue, leaving green. This is why screen primaries are commonly additive while print inks are subtractive. A set of three primaries reproduces only colors in its gamut. If an image color lies outside a printer’s gamut, color management must map it inward. A relative-colorimetric intent preserves in-gamut colorimetry after adapting the source white to the destination white, and clips or moves out-of-gamut colors; a perceptual intent compresses a broader range to preserve relationships. The choice changes the image’s color relationships and should match whether accurate isolated patches or a coherent overall rendering matters more. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-12 - Photography session 12|PHOTO-12]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-13 - Photography session 13|PHOTO-13]]


## Light measurement and radiometry

Radiometry gives light quantities units and conservation rules, connecting source geometry to energy arriving at surfaces and cameras.

### Radiometry and BRDF

For incoming radiance $L_i$, irradiance at surface point $x$ is

$$
E(x)=\int_\Omega L_i(x,\omega)|n\cdot\omega|\,d\omega.
$$

The projected-area cosine and the solid-angle measure use the quantities defined in the preceding radiometry section. The bidirectional reflectance distribution function, or BRDF, $f_r(x,\omega_i,\omega_o)$ describes how incoming light from direction $\omega_i$ is reflected toward outgoing direction $\omega_o$. It is nonnegative and must conserve energy; for reciprocal materials, swapping incoming and outgoing directions preserves its value. A Lambertian diffuse material has constant $f_r=\rho/\pi$. A mirror concentrates reflection into one direction, and glossy materials concentrate it around a lobe. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-16 - The Rendering Equation|CMU-16]]

When a surface point $x$ samples a point $y$ on an area light, direction $\omega=(y-x)/\|y-x\|$. An area element $dA_y$ subtends solid angle

$$
d\omega=\frac{|n_y\cdot(-\omega)|}{\|y-x\|^2}dA_y.
$$

This area-to-solid-angle Jacobian converts a light-point density into a directional density. The distance-squared and light-normal cosine cannot be omitted if the rendering integral is over directions. Probability density must be expressed with respect to the same measure as the integral. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-15 - Radiometry|CMU-15]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-17 - Numerical Integration|CMU-17]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-18 - Monte Carlo Rendering|CMU-18]]

### Measure, units, and conservation

Radiometric quantities answer different questions. Radiant energy measures an amount of light energy and is measured in joules. Radiant flux is energy per time and is measured in watts. Irradiance is flux arriving on a surface per area. Radiant intensity is flux emitted per solid angle by a point-like source. Radiance is flux per projected area per solid angle. The units track what is being integrated. Integrating radiance over solid angle with a projected-area cosine yields irradiance. Integrating flux over time yields energy.

Solid angle subtends area on a unit sphere. A cone that cuts out spherical area $A$ has solid angle $\Omega=A/r^2$ on a sphere of radius $r$. The full sphere has $4\pi$ steradians; a hemisphere has $2\pi$. A small disk far away subtends approximately its area divided by squared distance, multiplied by the cosine between its normal and the direction of view. This approximation is the same geometry behind inverse-square illumination and the area-to-solid-angle Jacobian.

Radiance is useful because, in a vacuum and absent participating media, it stays constant along a ray. Energy does not disappear merely because a ray changes direction. A surface can redirect radiance, absorb it, or emit it. A BRDF cannot create more outgoing energy than arrives when integrated over directions. These conservation principles catch physically impossible formulas. A diffuse BRDF constant over the hemisphere must be $\rho/\pi$, not just $\rho$, because integrating the cosine over a hemisphere yields $\pi$. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-15 - Radiometry|CMU-15]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-16 - The Rendering Equation|CMU-16]]

### Directional, hemispherical, and area lights

A directional light approximates a source infinitely far away: all rays share a direction and intensity does not vary with distance across a local scene. A point light emits from one location and obeys inverse-square falloff. A hemispherical source distributes light across a broad range of directions and can illuminate a surface more uniformly. An area source spans physical area, so different points on a receiver see different directions and distances. Area lights generally produce softer shadows because each point on the source creates a slightly different shadow boundary. These distinctions are consequences of source geometry and angular measure, not just different shading presets. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#CMU-15 - Radiometry|CMU-15]]

### Light in a scene: source size, reflection, and shape

Radiometry measures electromagnetic energy without weighting for human vision. Photometry weights light by the visual system’s spectral sensitivity. Radiant power is measured in watts; luminous flux in lumens. Luminous intensity is flux per solid angle, measured in candelas, where one candela is one lumen per steradian. A steradian is the solid angle that subtends an area of $r^2$ on a sphere of radius $r$; the full sphere is $4\pi$ steradians. Luminance describes light per projected area and direction, in candelas per square meter. Illuminance is luminous flux arriving per area, in lux, where one lux is one lumen per square meter. These quantities are not interchangeable: a source’s intensity, a surface’s luminance, and the illumination falling on a subject answer different questions. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-15 - Photography session 15|PHOTO-15]]

A point source illuminates a surface according to the inverse-square law. Doubling distance spreads the same energy across four times the area, so illuminance falls to one quarter. An extended area source behaves differently at close range. If a camera views a large, uniform surface with the same lens and field of view, doubling distance makes each pixel see four times as much surface area while the solid angle of each patch falls by four; the effects can approximately cancel. This distinction explains why moving a small flash back changes exposure dramatically, while moving away from a large uniformly luminous wall may change little. Real rooms add reflections, boundaries, and finite-source geometry. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-15 - Photography session 15|PHOTO-15]]

Light-source size and angular spread shape shadows. A small, distant source produces a crisp umbra with a narrow penumbra. A large source sends rays from a wider range of positions, so occlusion changes gradually and shadows soften. Leonardo da Vinci reasoned about umbra and penumbra as a painter’s tool: a region may be completely blocked, fully illuminated, or only partly covered by an extended source. A useful photographic model classifies a light by spatial extent (point, line, or area) and by the directions it emits from each part. A fluorescent tube is spatially long and diffuse; sunlight through a narrow crack is line-like but directional; a softbox is a large source with a controlled angular spread. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]]

Surfaces also distribute reflected light differently. A Lambertian diffuse surface appears similar from many viewing directions, although its emitted radiance and projected area must be distinguished carefully. A mirror sends light in one direction according to equal incident and reflected angles. Many objects combine diffuse and specular reflection. A microfacet model treats a surface as many tiny facets with slightly different normals; roughness spreads the specular lobe, while a polished surface concentrates it. A goniometric diagram plots reflected intensity as a function of viewing and illumination angles. Brushed metal is anisotropic: the reflection depends on direction along versus across the brushing. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-15 - Photography session 15|PHOTO-15]]

The BRDF (bidirectional reflectance distribution function) describes how an opaque surface reflects light from an incoming direction to an outgoing direction. A BSSRDF adds the possibility that light enters at one surface point, scatters beneath the surface, and exits at another. Skin, wax, fruit, and translucent materials need subsurface scattering to look natural; a purely surface-based model can make faces look hard and plastic. Fresnel reflection describes how the reflected fraction changes with angle and material. Dielectric highlights tend to take the light source’s color; conductor highlights can carry the metal’s color. At grazing angles, even a diffuse-looking dielectric can show a stronger specular reflection. Powdering translucent colored glass can make it appear pale because many small interfaces scatter light before it is absorbed through a long path. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-15 - Photography session 15|PHOTO-15]]

The luminance of a sunlit cloud can be several stops above the blue sky; clouds and snow can both be highly reflective, which helps create whiteouts with few reliable horizon cues. A bright scene can still be difficult to photograph if its luminance range exceeds what one exposure can represent. Light measurement is therefore not bookkeeping alone. It predicts shadow strength, required aperture and shutter, flash reach, and how highlights will compress in a finished image. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-15 - Photography session 15|PHOTO-15]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]]

### Lighting for a photograph

A portrait can be designed around a key light, the brightest or shaping light; a fill light, which controls shadow depth; a rim or accent light, which separates hair or shoulders from the background; and a background light. Moving a lamp changes its direction, apparent size, and shadow. Modifiers—softboxes, umbrellas, reflectors, grids, barn doors, and flags—change source size or the portion of the scene that receives light. A ratio close to one-to-one between key and fill makes a flatter, gentler portrait; a stronger key-to-fill difference creates more dramatic shadows. Broad lighting illuminates the face side turned toward the camera; short lighting emphasizes the less camera-facing side. Rembrandt lighting creates a small triangle of light on the shadowed cheek. Butterfly lighting places a central light above the camera and produces a symmetric glamour pattern. These names are practical shorthand for geometry and mood, not rigid recipes. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]]

Available light can be shaped without adding a lamp. A wall can bounce light into the shadow side of a face; a white card can fill a hand or cheek. Window light can make a directional key. A large white background can isolate a portrait while leaving texture in the subject. Light can also act as a pictorial or narrative element: a diagonal shaft may structure the composition or imply revelation, danger, or attention. A photographer can read a scene by inspecting shadow direction and softness, specular highlights, catchlights, and brightness ratios before deciding whether to supplement it. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]]

Special subjects need special control. Shiny objects reflect the room and photographer, so a tent or large diffuse enclosure can create a clean continuous reflection. Food highlights and moisture may need careful arrangement. Grazing light reveals shallow relief and embossed writing because it creates small directional shadows. Architectural interiors often require balancing a dim interior with a bright window; the photographer may make a long exposure for the outside while controlling interior lamps, accepting or managing clipped flames. These are lighting problems intertwined with exposure and composition. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]]

Flash adds a brief pulse. It can freeze motion if the flash duration is shorter than the subject’s movement interval, fill shadows while exposing for the background, or balance an ambient key with controlled fill. A guide number estimates flash exposure: for a given flash, aperture is approximately the guide number divided by subject distance, with units and zoom setting held consistent. A guide number of 58 at 10 feet suggests about $f/5.8$, near $f/5.6$. The guide number falls with flash zoom because the beam is spread over a wider angle. Flash on the camera axis can flatten shadows and create red-eye; off-camera, bounced, or umbrella-modified flash gives more dimensional control. A focal-plane shutter has a maximum sync speed when the sensor is fully uncovered. High-speed sync emits a rapid pulse train while the shutter slit travels, but reduces effective flash power. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]]

Flash and ambient light can have different color temperatures. A daylight-balanced flash used at sunset may appear blue unless warmed with an orange gel; a tungsten white balance may make an unfiltered flash blue, while a flash white balance can make tungsten lamps orange. Second-curtain sync fires near the end of a long exposure so motion trails follow a moving subject. Combining ambient and flash can preserve a room’s context while freezing a nearby subject. Computational flash/no-flash methods compare paired images to borrow color or detail, but reflections, shadows, scene motion, and alignment make robust fusion difficult. A burst of same-exposure underexposed frames is often more reliable for static or modest-motion scenes. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-17 - Photography session 17|PHOTO-17]]

### Read the shadows to estimate a light source

A shadow contains information about source angular size. Consider a small object casting a shadow on a nearby wall. A point source gives a crisp boundary because rays come from nearly one direction. A large softbox gives a penumbra because some source positions are blocked while others remain visible. Move the softbox closer while keeping its physical size unchanged and it occupies a larger angular extent; the shadow softens and the subject-to-background falloff changes. Move it farther away and it appears smaller, giving harder shadows. A photographer can therefore alter both shadow softness and illuminance by moving a source, even before changing power. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-15 - Photography session 15|PHOTO-15]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]]

The direction of a specular highlight tells a different story from diffuse shading. On a matte face, turning toward the key light changes broad tonal regions. On a glossy eye, the catchlight reflects the shape and position of the source. On a polished car, the highlight can show a stripbox, ceiling, or flag even if that modifier is outside the frame. On brushed metal, highlight shape elongates along the surface texture because the BRDF is anisotropic. In a tent-lit trumpet, the photographer chooses what the metal should reflect, effectively using the room itself as a painted illumination pattern. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-15 - Photography session 15|PHOTO-15]] · [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-16 - Photography session 16|PHOTO-16]]

A reflected-light meter reading from an outdoor scene also folds source, surface, and geometry together. Direct sun is strong, while the blue sky fills shadows from a broad hemisphere. The lit side and the shadow side may differ by only a few stops, less than a viewer intuitively guesses because dark shadows feel dramatic. Under cloud cover, the whole visible sky becomes a giant source; shadows nearly disappear, but local contrast and color saturation change. On snow beneath a bright cloudy sky, sky and ground can have similar luminance and visual cues to the horizon weaken, producing a whiteout. These are consequences of source extent and reflectance, not just exposure. [[Chapter Edition/02 - Graphics Vision and XR/36 - Source Atlas and Reconstruction Checkpoints#PHOTO-15 - Photography session 15|PHOTO-15]]

### Environment illumination and differential compositing

*Connect to:* light measurement and radiometry; reflectance and BRDFs; the rendering equation and light transport; image formation and compositing.

Debevec's 1998 method begins with a useful division of the scene. Put faraway illumination in an **environment**: the distant world is represented by a high-dynamic-range (HDR) radiance map. Put nearby surfaces in a **local scene**: they need geometry and reflectance estimates because the inserted object can cast shadows on them, reflect from them, or light them. This division is physical, not merely organizational. A distant light source is not blocked by a small inserted object in the same way a nearby wall is. If the new object can substantially change a surface's illumination, that surface belongs in the local model.

A mirror-ball photograph records directions around the probe as reflected directions on the ball. The method places the ball near the object insertion point, captures several exposures, and combines them into an HDR environment map. The map must represent radiance, not the display values in one ordinary exposure. Camera response, exposure, probe location, camera pose, and the synthetic scene's coordinate frame must be aligned well enough that a rendered pixel refers to the same direction and place as the photograph. The probe is a compact measurement of incoming light; it does not by itself describe local walls, their reflectance, or occlusion.

The key compositing step is to predict a *change*. Let $L_b$ be the observed background pixel. Let $L_0$ be the rendered local scene without the inserted object, and let $L_1$ be the same render with it. Then add the modeled change to the photographed pixel:

$$L_{final}=L_b+(L_1-L_0).$$

The difference $L_1-L_0$ includes effects caused by the new object: its visible radiance, its shadow on modeled surfaces, and changes in interreflection that the renderer captures. Subtracting the no-object render removes the model's estimate of the old local contribution; adding the difference preserves the photographed background's original detail. This is **differential rendering**. It is especially useful when the photograph is more detailed than the scene model. The model only has to predict how the insertion changes the image, rather than recreate every pixel in the photograph.

A tiny numerical example makes the bookkeeping concrete. Suppose a photograph has pixel value $L_b=0.40$ in linear radiance units. The local-scene render predicts $L_0=0.10$ before insertion and $L_1=0.22$ after insertion. The modeled change is $0.12$, so the composite becomes $0.52$. The method does not replace the photographed value by $0.22$; that would discard the rest of the photograph's contribution at that pixel. In practice, values must be in compatible linear radiometric units. Applying this arithmetic directly to gamma-encoded display values is not physically correct.

The subtraction can expose model errors. If $L_1-L_0$ is too negative, the composite can fall below zero even though radiance should not. The paper describes a relative alternative, $L_{final}=L_b(L_1/L_0)$, which scales the photographed contribution by the modeled ratio. This avoids some negative results, but it becomes unstable when $L_0$ is near zero and still depends on the model getting the ratio right. It is a compensation strategy, not a substitute for a good local model.

Reflectance estimation is also tied to the same measurement logic. For a diffuse patch with known incident illumination, compare a photograph to a render of the same patch assigned unit diffuse reflectance. In the paper's Lambertian update, if $L_{r1}$ is the observed patch radiance and $L_{r2}$ is the rendered unit-reflectance radiance, update the diffuse reflectance estimate by

$$\rho'_d=\frac{L_{r1}}{L_{r2}}.$$

The render includes indirect illumination, so the ratio can be iterated as the scene's reflectance estimates change. This is a model-fitting loop: render a prediction, compare it with a measurement, and adjust the parameter that scales the diffuse response. It relies on a sufficiently accurate lighting estimate and on the patch being adequately described by a diffuse model.

**Limits and assumptions.** The method needs a registered camera, probe, and local scene. A mirror ball has finite resolution and may include the camera, support, seams, or other artifacts. A probe far from the insertion point may measure the wrong illumination. Incorrect geometry or reflectance produces incorrect shadows and differences; differential compositing can preserve the plate but cannot make the predicted change correct. The paper's demonstrations used the Radiance renderer and two diffuse bounces. Treat the results as evidence for this measured-light compositing setup, not proof that a sparse local model captures every modern material or transport effect.

**Source:** Paul Debevec, “Rendering Synthetic Objects into Real Scenes: Bridging Traditional and Image-based Graphics with Global Illumination and High Dynamic Range Photography” (1998), complete paper, PDF pp. 1–10. [Primary PDF](https://www.pauldebevec.com/Research/IBL/debevec-siggraph98.pdf).

## Reflectance, BRDFs, and shading

<!-- CHAPTER-SOURCE-END -->

---

[[Chapter Edition/02 - Graphics Vision and XR/00 - Reading Map|Reading map]] · [[Chapter Edition/02 - Graphics Vision and XR/03 - Lens Optics and Image Formation|Previous chapter]] · [[Chapter Edition/02 - Graphics Vision and XR/05 - Reflectance, Materials and Light Transport|Next chapter]]
