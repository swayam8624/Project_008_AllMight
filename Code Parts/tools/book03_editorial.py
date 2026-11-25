"""Reader-facing editing and subject index for Book 03.

The original source Markdown remains authoritative. Every relocated or rewritten
paragraph is retained in a staging receipt. Only explicit prose rules are applied;
code blocks, figure assets and tagged derivations are not silently rewritten.
"""
from __future__ import annotations
import re
from pathlib import Path

REPO = "https://github.com/swayam8624/Project_008_AllMight"
# One short orientation sentence per chapter: a navigation aid, not a new claim.
TAKEAWAYS = [
"Distinguish an image address, its sample location and the signal it represents; build the smallest possible drawing machine.",
"Use points, vectors, origins, bases and units consistently before composing coordinate changes.",
"Derive edge tests and barycentric weights, then distinguish an exact geometric decision from its numerical representation.",
"Compose affine maps in the declared order; transform normals by the inverse transpose when the map is invertible.",
"Use an explicit numerical domain and certify uncertain orientation decisions rather than trusting an arbitrary epsilon.",
"Work in three dimensions with planes, rays, intersections and normals; separate a supporting-plane hit from a primitive hit.",
"Keep mesh identity and adjacency separate from positions; test winding, boundaries and manifold conditions independently.",
"Construct curves and tensor-product patches, evaluate derivatives and preserve shared seams when sampling.",
"Choose an explicit parametric or implicit representation, then measure geometric and topological errors in its sampled mesh.",
"Validate asset bytes, indexing, geometry and topology separately; repair only under a declared policy.",
"Separate a reusable local asset from its world instances and derive the transformation chain before rendering.",
"Represent rigid orientation with consistent conventions; verify rotation composition and interpolation.",
"Derive the pinhole projection and inverse ray, with a declared optical frame and continuous image coordinates.",
"Clip homogeneous coordinates before division; map to the viewport and declare the stored-depth convention.",
"Recover depth from a calibrated baseline and correct correspondence; account for the nonlinear uncertainty in disparity.",
"Track raw vertex attributes, reciprocal w, divided depth and the different interpolation contracts through the pipeline.",
"Clip primitives and varyings together; use conservative geometric bounds without falsely rejecting visible geometry.",
"Assign every sample exactly one edge owner under the declared raster rules; compare accelerated coverage against the exact mask.",
"Resolve opaque visibility using a depth convention, conservative bounds and a tie policy; distinguish transparency.",
"Treat image formation as sampling and reconstruction; estimate projected texture footprints and filter in the right signal space.",
"Trace a sample through the whole CPU renderer and check every stage against independently derived values.",
"Connect physical lens dimensions, geometric focus and blur; understand the limits of the paraxial approximation.",
"Model forward distortion and inverse rays over a valid domain; distinguish coordinate accuracy from image resampling.",
"Connect collected photons to charge, noise and digital codes; treat saturation as censored measurement.",
"Declare color spaces, reference whites, linear-light operations and information loss before interpreting RGB.",
"Use calibration geometry and Jacobian sensitivity to distinguish a small residual from a well-constrained camera.",
"Relate two views using calibrated geometry, rectify when appropriate and keep matching uncertainty separate from epipolar incidence.",
"Evaluate feature repeatability, descriptor retrieval and geometric verification using their own denominators.",
"Triangulate tracks, reason about similarity gauge and understand why a lower image residual need not improve metric geometry.",
"Fuse supported depth observations into a signed field and extract only what the observations justify.",
"Compose the reference modules with explicit frame, identity, revision and dependency contracts.",
"Turn a technical claim into a reproducible experiment with independent witnesses, failure cases and a declared measurement plan.",
]

# The prefix identifies an inspected provenance paragraph. The replacement keeps
# the useful reader-facing technical scope. Complete original paragraphs are
# written to the proof's provenance receipt, not discarded from the repository.
NOTES = {
1: [
("This is an original introductory explanation.", "The P6 portable pixmap experiment uses a deliberately small documented subset of the format. It is not a general PNM parser or a color-management pipeline. See the bibliography for Netpbm and introductory graphics resources."),
("Sources to reconcile during release editing:", None)],
2: [
("The original long-form AllMight chapter", "For a deeper treatment of frames and projection, consult the companion notes Geometry Becomes an Image and an Inverse Problem and Camera Extrinsics and Projection Geometry. Here we stay in two-dimensional affine space before introducing a calibrated 3D camera."),
("Suggested source families to verify", "Further reading: the Berkeley CS 184 graphics course, Scratchapixel's coordinate-frame lessons and conventional linear algebra texts on affine spaces.")],
3: [
("The original AllMight", "The companion notes Robust Predicates and Camera Sensitivity develop numerical filtering and geometric decision errors in greater depth. Chapter 05 introduces exact orientation decisions; Chapter 07 develops mesh connectivity."),
("The companion implementation is under", "The integer oracle is exact only in its explicit bounded integer domain; it is not a general adaptive-precision predicate. See Shewchuk's robust-predicate research in the bibliography. The triangle laboratory builds on the image and frame laboratories.")],
4: [
("The original AllMight material", "Further reading: Coordinate Frames and Linear Systems and Camera Contracts and Transform-Chain Debugging in the companion notes. The laboratory uses its declared matrix, storage and clipping conventions; other engines may make different choices.")],
5: [
("The established long-form source", "The companion notes Robust Predicates and Camera Sensitivity extend this discussion of sign decisions, filtering, exact arithmetic and camera uncertainty. The laboratory is an independent exact-dyadic teaching reference.")],
6: [
("The original long graphics manuscript, especially", "For additional intersection methods, see Ray–Triangle Intersection and Mesh Queries, Classification and Clipping in the companion notes. This laboratory provides bounded primitive implementations rather than a complete spatial query engine."),
("The long Computing Foundations", "The oblique diagrams in this chapter are illustrative maps, not calibrated camera projections. Chapter 13 develops the optical camera model.")],
7: [
("The substantial original chapter", "The companion notes Mesh Topology and Connectivity develop indexed incidence, adjacency, winding, half-edge data structures and manifold conditions in greater depth. Ray queries and clipping are addressed in Chapters 06 and 17.")],
8: [
("Our canonical legacy mathematics and graphics notes", "Further reading: Surfaces, Topology and Parameterization in the companion notes develops bicubic patches, partial derivatives, seams, subdivision and chart distortion. The bibliography lists established curve and surface texts.")],
9: [
("This is an additive reader chapter", "Further reading: Surfaces, Topology and Parameterization and Learned 3D Shapes and Scene Representations in the companion notes distinguish parametric, implicit, sampled and learned surface representations.")],
10: [
("This is an additive chapter.", "The companion notes Mesh Topology and Connectivity and Per-Vertex and Per-Corner Mesh Attributes provide additional detail on topology, attribute seams, asset formats and animation. The laboratory implements a restricted asset reader, not every feature of glTF.")],
11: [
("The original reading-copy", "Further reading: Per-Vertex and Per-Corner Mesh Attributes and Camera Contracts and Transform-Chain Debugging in the companion notes cover scene hierarchy, transform diagnostics and normal transport."),
("The mathematical treatment here is independently authored", None)],
12: [
("The existing legacy chapter", "Further reading: Per-Vertex and Per-Corner Mesh Attributes and Camera Contracts and Transform-Chain Debugging in the companion notes cover animation, quaternion interpolation and pose conventions.")],
13: [
("The original 32 - Geometry", "Further reading: Geometry Becomes an Image and an Inverse Problem and Camera Contracts and Transform-Chain Debugging in the companion notes cover projection, reconstruction and camera diagnostics.")],
14: [
("The original 33 - Projection", "Further reading: Projection, Clipping, and Raster Sampling and Robust Predicates and Camera Sensitivity in the companion notes cover graphics API conventions, homogeneous frustum boundaries and depth precision."),
("This additive chapter and its laboratory", "The projection laboratory establishes the stated scalar conventions, not the behavior of every graphics API or GPU.")],
15: [
("Original Computing Foundations reading copies", "Further reading: Geometry Becomes an Image and an Inverse Problem and Camera Extrinsics and Projection Geometry in the companion notes cover stereo geometry, off-axis displays, IPD, rectification and depth sensitivity.")],
16: [
("The original Computing Foundations chapter 33", "Further reading: Projection, Clipping, and Raster Sampling and Per-Vertex and Per-Corner Mesh Attributes in the companion notes cover homogeneous interpolation, vertex attributes, skinning and animation.")],
17: [
("The original Computing Foundations chapters 33", "Further reading: Projection, Clipping, and Raster Sampling and Robust Predicates and Camera Sensitivity in the companion notes cover clipping and robust geometric classification. This laboratory uses the zero-to-one clip convention and preserves raw varyings during clipping.")],
18: [
("The exact committed coverage header and test source", "The coverage laboratory has standalone numerical tests and a cumulative integration suite. The tile extension is tested separately against exhaustive sample enumeration; neither test establishes a GPU performance result."),
("The original Computing Foundations chapter 33", "Further reading: Projection, Clipping, and Raster Sampling and Robust Predicates and Camera Sensitivity in the companion notes develop sample-center and top-left edge conventions.")],
19: [
("Both the standalone test executable and studio", "The visibility laboratory checks the per-sample opaque reference and generates a controlled winner mask. The conservative hierarchy and transparent rendering extensions require their own verification."),
("The original Computing Foundations chapter 33", "Further reading: Projection, Clipping, and Raster Sampling in the companion notes covers depth precision and reversed depth; Robust Predicates and Camera Sensitivity covers conservative occlusion classification.")],
20: [
("The original Computing Foundations chapter 33", "Further reading: Projection, Clipping, and Raster Sampling in the companion notes covers sample centers and temporal jitter. The native experiment isolates scalar sampling and quadrature.")],
21: [
("The new integration reuses", "The CPU renderer integrates the earlier geometry, camera, clipping, coverage and visibility laboratories. Its documented tests and reproducible inputs are available in the companion repository; the hardware adapter remains an extension.")],
22: [
("The original Graphics Vision and XR chapter 03", "Further reading: Lens Optics and Image Formation in the companion notes develops focus intervals, principal rays and diffraction-scale comparisons. The next chapters address distortion and sensors.")],
23: [
("Computing Foundations chapter 35", "Further reading: Camera Extrinsics and Projection Geometry and Lens Optics and Image Formation in the companion notes develop distortion Jacobians, inverse methods and angular models. The laboratory implements a bounded distortion map and inversion.")],
31: [
("The standalone C++23 laboratory and studio compiled", "The companion studio includes numerical tests and independently checkable image, mesh and track exports. Its controlled fixture is not a general photographic reconstruction system.")],
32: [
("These are commands for the reader to run", "Run the companion tests on your own compiler and hardware. Preserve the exact inputs, source revision and verifier outputs with every research claim. A passing controlled test establishes only its declared contract.")],
}

# Conservative inline-math copyediting of exact newly authored plain expressions.
# Match only outside existing math and code; keep a receipt of each substitution.
MATH_EDITS = {
18: [("E(x,y)=ax+by+c",r"E(x,y)=ax+by+c"),("S(W-1), S(H-1)",r"S(W-1),\,S(H-1)"),("E=3x-2y+4",r"E=3x-2y+4"),("f=2x+3y",r"f=2x+3y")],
19: [("q=0.4+0.02x-0.01y",r"q=0.4+0.02x-0.01y"),("n=0.1 m and f=1000 m",r"n=0.1\,\mathrm{m},\ f=1000\,\mathrm{m}"),("q=n/d",r"q=n/d"),("delta_d=d² delta_q/B",r"\Delta d=d^2\Delta q/B")],
20: [("h(x,y)=h_x(x)h_y(y)",r"h(x,y)=h_x(x)h_y(y)"),("h(x)=max(0,1-|x|)",r"h(x)=\max(0,1-|x|)"),("L=log2(max(1,rho))",r"L=\log_2(\max(1,\rho))"),("J=diag(8,1)",r"J=\operatorname{diag}(8,1)")],
21: [("n=1 and f=10",r"n=1,\ f=10"),("d=1/0.3725",r"d=1/0.3725")],
22: [("v=f s/(s-f)",r"v=fs/(s-f)"),("m=v/s=f/(s-f)",r"m=v/s=f/(s-f)"),("A=f/N",r"A=f/N"),("r=R sqrt(U)",r"r=R\sqrt{U}"),("r=RU",r"r=RU")],
23: [("r=2f sin(theta/2)",r"r=2f\sin(\theta/2)"),("r=f theta",r"r=f\theta"),("x_d=x(1-0.2x²)",r"x_d=x(1-0.2x^2)"),("x_d=x-x³",r"x_d=x-x^3")],
24: [("Y=gN+e",r"Y=gN+e"),("M=gS",r"M=gS"),("D=Y_1-Y_2",r"D=Y_1-Y_2"),("Var(D)=2 Var(Y)",r"\operatorname{Var}(D)=2\operatorname{Var}(Y)")],
25: [("A=(C^TC)^{-1}C^TX",r"A=(C^TC)^{-1}C^TX"),("delta=6/29",r"\delta=6/29")],
26: [("Y=RX+t",r"Y=RX+t"),("J=diag(1,0.001)",r"J=\operatorname{diag}(1,0.001)"),("rho=0.99",r"\rho=0.99"),("u_i=f_x x_i+c_x",r"u_i=f_xx_i+c_x")],
27: [("Z=fB/d",r"Z=fB/d"),("n^T X=h",r"n^TX=h"),("X=h x/(n^T x)",r"X=h x/(n^Tx)")],
28: [("H delta=-g",r"H\delta=-g"),("g=(r-l)/2",r"g=(r-l)/2"),("H=l-2c+r",r"H=l-2c+r")],
29: [("rho=1/r",r"\rho=1/r"),("X=C+d/rho",r"X=C+d/\rho"),("eta=1/Z",r"\eta=1/Z"),("Q_i=I-d_i d_i^T",r"Q_i=I-d_id_i^T")],
30: [("Y=RX+t=(x,y,z)",r"Y=RX+t=(x,y,z)"),("phi_0/(phi_0-phi_1)",r"\phi_0/(\phi_0-\phi_1)")],
31: [("Y=RX+t",r"Y=RX+t"),("X=R^T Y-R^T t",r"X=R^TY-R^Tt")],
32: [("y=f(x)",r"y=f(x)"),("y=x_1-x_2",r"y=x_1-x_2"),("Var(y)=2-2rho",r"\operatorname{Var}(y)=2-2\rho")],
}

def text_of(v):
    if isinstance(v, list):
        return "".join(text_of(z) for z in v)
    if not isinstance(v, dict):
        return ""
    t = v.get("t")
    if t == "Str": return v["c"]
    if t in ("Space","SoftBreak","LineBreak"): return " "
    if t in ("Code","Math"): return v["c"][-1]
    return text_of(v.get("c", []))

def words(s):
    result=[]
    for i,w in enumerate(s.split()):
        if i: result.append({"t":"Space"})
        result.append({"t":"Str","c":w})
    return result

def para(s):
    return {"t":"Para","c":words(s)}

def format_math(source, n):
    edits = MATH_EDITS.get(n, [])
    changes = []
    fence = None
    display = False
    result = []
    for line in source.splitlines(keepends=True):
        m=re.match(r"^\s*(~~~+|"+chr(96)+r"{3,})(.*)$",line)
        if m:
            token=m.group(1)
            if fence is None: fence=token
            elif token[0]==fence[0] and len(token)>=len(fence) and not m.group(2).strip(): fence=None
            result.append(line)
            continue
        if line.strip() in (r"\[","$$"):
            display=not display if line.strip()=="$$" else True
            result.append(line); continue
        if line.strip()==r"\]":
            display=False; result.append(line); continue
        if not fence and not display:
            # Do not touch existing inline code or delimited mathematics.
            parts=re.split(r"("+chr(96)+r"[^"+chr(96)+r"\n]*"+chr(96)+r"|\\\([^\n]*?\\\)|\$[^$\n]+\$)",line)
            for j in range(0,len(parts),2):
                for old,tex in edits:
                    count=parts[j].count(old)
                    if count:
                        parts[j]=parts[j].replace(old,r"\("+tex+r"\)")
                        changes.append({"old":old,"math":tex,"count":count})
            line="".join(parts)
        result.append(line)
    return "".join(result), changes

def edit_chapter(blocks, n):
    result=[]
    history=[]
    for b in blocks:
        t=b["t"]
        if t=="Header":
            level,attr,inlines=b["c"]
            title=text_of(inlines)
            if re.match(r"^(Source custody|Source lineage|Reader references and scope)",title,re.I):
                history.append({"reason":"reader-facing heading","original":b})
                b={"t":"Header","c":[level,attr,words("Further reading")]}
            elif title=="Glossary and source boundary":
                history.append({"reason":"reader-facing heading","original":b})
                b={"t":"Header","c":[level,attr,words("Glossary")]}
            result.append(b); continue
        if t=="Para":
            original=text_of(b)
            if original.startswith("From Geometry to Images Part ") and re.search(r"draft|additive|manuscript|Reader edition",original,re.I):
                history.append({"reason":"internal draft banner","original":b})
                continue
            match=next(((prefix,new) for prefix,new in NOTES.get(n,[]) if original.startswith(prefix)),None)
            if match:
                history.append({"reason":"reader-facing source/evidence note","original":b})
                if match[1]: result.append(para(match[1]))
                continue
        result.append(b)
    # The new chapter-opening panel is shared by print and ebook, and
    # does not introduce a heading, TOC entry or extra forced page.
    summary={"t":"Div","c":[["",["book-callout","concept"],[]],
             [{"t":"Para","c":[{"t":"Strong","c":words("Chapter at a glance.")},
                              {"t":"Space"},*words(TAKEAWAYS[n-1])]}]]}
    return result[:1]+[summary]+result[1:],history

# A curated alphabetical subject index, with exact existing section IDs.
# Chapter-only destinations are used only for broad subjects. No generated
# print page numbers are guessed: the LaTeX writer uses pageref at typesetting.
INDEX = {
"Ablation studies":["c32-an-ablation-needs-a-meaningful-comparison"],
"Affine transformations":["c04-why-the-word-affine","c04-how-two-affine-maps-compose"],
"Aliasing":["c20-a-sample-a-footprint-and-a-reconstruction","c20-derive-the-box-filter-response"],
"Alpha compositing":["c19-opaque-visibility-is-different-from-transparency"],
"Angular projection":["c23-different-lenses-can-produce-similar-pictures-for-different-reasons"],
"Anisotropic filtering":["c20-choose-a-texture-level-from-a-projected-footprint"],
"Aperture":["c22-the-aperture-is-a-gate-not-a-focal-plane","c22-aperture-samples-should-represent-area"],
"Asset identity":["c11-asset-identity-is-not-object-identity"],
"B-splines":["c08-b-splines-when-a-garden-path-has-many-control-points"],
"Barycentric coordinates":["c03-three-invisible-portions-of-one-point","c16-derive-the-reciprocal-w-rule"],
"Baseline":["c15-the-little-shift-becomes-a-very-large-distance","c27-disparity-is-a-measurement-with-its-own-uncertainty"],
"Benchmarking":["c32-which-speedup-are-we-reporting","c32-turn-a-research-claim-into-a-measurement-plan"],
"Bézier curves":["c08-a-deeper-secret-the-blossom-behind-de-casteljau"],
"Binary64":["c05-the-binary64-bit-decoding-is-a-little-mechanical-but-worth-seeing"],
"Bundle adjustment":["c29-bundle-adjustment-adjust-the-entire-puzzle-together","c29-make-the-schur-complement-tangible"],
"Calibration":["c26-the-calibration-jacobian-tells-us-which-knobs-we-can-actually-turn","c26-guided-workshop-design-a-well-conditioned-calibration-experiment"],
"Camera center":["chapter-13","chapter-15"],
"Camera distortion":["c23-derive-the-jacobian","c23-solve-the-inverse-with-a-small-linear-problem"],
"Camera intrinsics":["c13-from-a-metric-sensor-to-image-coordinates","c26-the-calibration-jacobian-tells-us-which-knobs-we-can-actually-turn"],
"Chromatic adaptation":["c25-white-adaptation-is-a-change-of-viewing-condition-not-a-universal-correction"],
"CIELAB":["c25-what-is-a-color-difference-measuring"],
"Clipping":["c14-why-clipping-comes-before-division","c17-why-the-dragon-must-be-clipped-before-the-divide"],
"Color calibration":["c25-a-cameras-three-measurements-need-a-calibration-model"],
"Color encoding":["c01-numbers-that-stand-for-color","chapter-25"],
"Conditioning":["c04-when-a-map-almost-forgets-where-things-came-from","c26-uncertainty-can-be-a-matrix-not-just-a-reprojection-number"],
"Coordinate frames":["c02-what-is-a-coordinate-frame","c16-follow-the-corner-through-four-coordinate-frames"],
"Coverage":["c18-coverage-bias-must-not-change-the-attribute-field","c18-a-conservative-tile-classifier-you-can-implement"],
"Cross product":["c06-the-cross-product-an-arrow-that-stands-away-from-a-surface"],
"Depth buffer":["c14-the-mountain-and-the-depth-buffer","c19-the-depth-notebook-can-answer-questions-about-a-whole-region"],
"Depth of field":["c22-depth-of-field-is-a-promise-about-tolerance"],
"Depth precision":["c19-why-depth-precision-changes-across-the-picture"],
"Descriptors":["c28-a-descriptor-is-a-choice-about-what-to-forget"],
"Determinant":["c04-the-determinant-is-a-tiny-machine-for-telling-geometrys-fate"],
"Diffraction":["c22-a-smaller-aperture-does-not-mean-unlimited-sharpness"],
"Disparity":["c27-disparity-is-a-measurement-with-its-own-uncertainty"],
"DLT":["chapter-26"],
"Edge function":["c18-the-edge-function-remembers-which-side-you-are-on"],
"Epipolar geometry":["c15-the-epipolar-plane","c27-learn-the-fundamental-matrix-from-eight-correspondences"],
"Error budget":["c32-a-numerical-claim-needs-an-error-budget"],
"Essential matrix":["c15-from-a-plane-to-the-essential-matrix","c27-from-e-to-relative-motion-then-a-point"],
"Euler characteristic":["c07-counting-the-square-four-corners-five-edges-two-faces"],
"Exposure":["c24-aperture-and-time-two-controls-that-change-the-collected-light"],
"Feature matching":["c28-separate-four-sources-of-matching-failure"],
"Fisheye projection":["c23-a-fisheye-is-not-a-broken-pinhole"],
"Floating-point precision":["c05-the-subtraction-that-eats-information","c31-the-worlds-biggest-coordinate-can-hide-its-smallest-movement"],
"Focal length":["c13-what-happens-when-we-change-focal-length","c22-focal-length-is-not-a-pixel-count"],
"Frustum":["chapter-17"],
"Fundamental matrix":["c27-learn-the-fundamental-matrix-from-eight-correspondences"],
"Gauge freedom":["c29-an-optimizer-must-distinguish-gauge-from-a-weak-measurement"],
"glTF":["c10-enter-gltf-when-the-mesh-is-a-graph-of-metadata-pointing-at-bytes"],
"Half-edge mesh":["c07-giving-every-edge-an-address-of-its-own-half-edges"],
"Harris detector":["c28-harris-without-an-eigensolver"],
"Heightfield":["c09-our-first-living-landscape-a-heightfield"],
"Hermite curves":["c08-hermite-curves-ask-for-endpoint-directions-directly"],
"Hierarchical depth":["c19-guided-workshop-the-conservative-depth-pyramid"],
"Homogeneous coordinates":["c02-what-about-homogeneous-coordinates","c14-the-fourth-number-is-not-decoration"],
"Homography":["c27-a-plane-can-explain-the-data-surprisingly-well"],
"Implicit surfaces":["c09-combining-implicit-shapes-is-powerful-and-dangerous"],
"Interpolation, perspective-correct":["c16-derive-the-reciprocal-w-rule","c21-trace-one-triangle-through-the-entire-machine"],
"Inverse depth":["c29-a-landmark-is-a-parameterization-choice"],
"Inverse transpose":["c04-what-about-normals-they-are-not-ordinary-arrows","chapter-11"],
"Jacobian":["c23-derive-the-jacobian","c26-the-calibration-jacobian-tells-us-which-knobs-we-can-actually-turn"],
"Laplacian":["c07-why-your-neighbors-influence-your-shape-sparse-adjacency-and-the-graph-laplacian","c28-a-corner-has-a-useful-size-as-well-as-a-position"],
"Lens model":["c22-a-lens-has-a-useful-geometric-approximation-and-a-physical-limit"],
"Manifoldness":["c07-boundary-interior-conflict-and-nonmanifold"],
"Marching cubes":["c09-what-about-marching-cubes-dual-contouring-and-neural-implicit-surfaces"],
"Mesh extraction":["c30-extracting-a-mesh-is-a-second-inference"],
"Metamorphic testing":["c32-metamorphic-tests-are-useful-when-the-answer-is-expensive"],
"Mipmaps":["c20-choose-a-texture-level-from-a-projected-footprint"],
"Noise":["c24-noise-is-not-always-a-mistake","c24-a-photon-transfer-experiment-separates-two-kinds-of-noise"],
"Normals":["c06-the-cross-product-an-arrow-that-stands-away-from-a-surface","c10-normals-geometry-shading-and-the-danger-of-averaging-everything"],
"OBJ":["c10-the-smallest-useful-obj-file"],
"Occlusion":["c19-coverage-testing-shading-and-work","c27-why-rectified-matching-can-fail-even-in-a-perfect-camera"],
"Orientation predicate":["c05-how-do-we-make-an-exact-orientation-decision"],
"Parameterization":["c08-a-patch-can-stretch-even-when-its-outline-looks-correct"],
"Photon transfer":["c24-a-photon-transfer-experiment-separates-two-kinds-of-noise"],
"Pinhole camera":["c13-the-native-pinhole-simulator"],
"Pixel center":["c01-a-pixel-has-an-address-and-a-sample-has-a-position","c18-a-square-is-not-a-sample"],
"Plane intersection":["c06-where-a-ray-meets-a-plane"],
"Point-spread function":["c22-a-camera-records-a-spread-not-a-perfect-point"],
"Pose":["c26-the-calibration-jacobian-tells-us-which-knobs-we-can-actually-turn"],
"Projection":["chapter-13","chapter-14"],
"Quaternion":["c12-quaternions-four-numbers-with-a-rule-for-multiplying-them"],
"RANSAC":["c28-let-geometry-argue-with-appearance"],
"Rasterization":["c18-from-individual-samples-to-blocks-of-the-image","chapter-21"],
"Ray-triangle intersection":["c06-ask-the-ray-and-the-triangle-the-same-question"],
"Reconstruction filters":["c20-design-a-reconstruction-filter-instead-of-hoping-for-one"],
"Rectification":["c27-rectification-chooses-convenient-images-not-new-observations"],
"Reprojection error":["c29-ask-the-photographs-directly","c32-a-smaller-residual-can-conceal-a-worse-world"],
"Reverse-Z":["c14-reverse-the-depth-ordering","c19-why-depth-precision-changes-across-the-picture"],
"Rolling shutter":["c24-the-sensor-measures-an-area-and-a-time-interval"],
"Saturation":["c24-a-saturated-value-carries-an-inequality"],
"Scale space":["c28-a-corner-has-a-useful-size-as-well-as-a-position"],
"Schur complement":["c29-make-the-schur-complement-tangible"],
"Sensor calibration":["c24-guided-workshop-separate-bias-temporal-noise-and-saturation"],
"Signed distance field":["chapter-09","c30-a-depth-value-must-pass-through-a-camera-before-entering-a-volume"],
"Singular values":["c26-a-small-singular-value-is-not-enough"],
"SLERP":["c12-from-one-pose-to-another-without-a-sudden-jump"],
"Snapshot identity":["c31-give-a-snapshot-a-stable-meaning"],
"Sparse volume":["c30-a-sparse-volume-saves-memory-only-by-changing-its-representation"],
"Stereo":["chapter-15","c27-guided-workshop-make-the-stereo-answer-key-before-matching"],
"Structure from motion":["chapter-29"],
"Subpixel refinement":["c28-localize-a-feature-between-samples"],
"Temporal sampling":["c20-the-image-changes-over-time-too"],
"Texture footprint":["c20-choose-a-texture-level-from-a-projected-footprint"],
"Tolerance":["c32-tolerance-is-a-statement-about-a-quantity"],
"Topology":["chapter-07","chapter-10"],
"Transform composition":["c04-how-two-affine-maps-compose","chapter-11"],
"Triangulation":["c29-derive-the-weighted-triangulation-system"],
"TSDF":["c30-a-depth-value-must-pass-through-a-camera-before-entering-a-volume","c30-guided-workshop-follow-a-surface-through-fusion-and-extraction"],
"Uncertainty":["c26-uncertainty-can-be-a-matrix-not-just-a-reprojection-number","c32-a-numerical-claim-needs-an-error-budget"],
"Vertex attributes":["chapter-16"],
"Viewport":["c14-the-viewport-brings-ndc-home"],
"Visibility":["chapter-19"],
"White balance":["c25-white-adaptation-is-a-change-of-viewing-condition-not-a-universal-correction"],
}

def subject_index(blocks):
    ids={b["c"][1][0] for b in blocks if b["t"]=="Header"}
    index=[{"t":"Header","c":[1,["subject-index",[],[]],words("Subject index")]},
           para("Select a chapter or section to return directly to the corresponding discussion. Printed page references are resolved by the typesetter.")]
    initial=None
    for term,targets in sorted(INDEX.items(),key=lambda x:x[0].casefold()):
        letter=term[0].upper()
        if letter!=initial:
            initial=letter
            index.append({"t":"Header","c":[2,["index-"+letter.lower(),[],[]],words(letter)]})
        line=[{"t":"Strong","c":words(term)},{"t":"Str","c":"."},{"t":"Space"}]
        for i,target in enumerate(targets):
            if target not in ids: raise ValueError(f"subject index target missing: {term}: {target}")
            chapter=re.search(r"\d{2}",target).group(0)
            if i: line.extend([{"t":"Str","c":";"},{"t":"Space"}])
            line.append({"t":"Link","c":[["",["index-reference"],[["data-chapter",chapter]]],
                         words("Ch. "+chapter),["#"+target,""]]})
        index.append({"t":"Para","c":line})
    return index
