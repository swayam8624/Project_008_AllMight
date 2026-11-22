#!/usr/bin/env python3
"""Book Two EPUB reader finishing pass.

Source-preserving normalization of the *generated* EPUB stage artifact:
- Improve exactly the 51 originally generic/filename-only figure descriptions
  using an author-reviewed, SHA-guarded mapping of the actual raster images.
- Reduce over-deep 853-entry navigation to a readable two-level TOC without
  removing any real heading or source section.
- Repair the legacy NCX cover-id and declared-depth metadata.

Only the Book Two EPUB is processed. Never modify the frozen 16 chapters,
source figures, C++23 workbench, or the original Pandoc AST cache.
"""
from __future__ import annotations

import copy
import hashlib
import html
import os
import posixpath
import re
import tempfile
import urllib.parse
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path

XHTML="http://www.w3.org/1999/xhtml"
EPUB="http://www.idpf.org/2007/ops"
NCX="http://www.daisy.org/z3986/2005/ncx/"
ET.register_namespace("",XHTML)
ET.register_namespace("epub",EPUB)

ALT_BY_FILENAME = {
  "file0.png": [
    "8aadb57529448c71",
    "Illustration of a robot arm examining a triangular metal panel under an overhead inspection camera."
  ],
  "file1.png": [
    "5fa698bd0907e5b6",
    "Flow diagram showing the same vector represented in world coordinates and in a slanted basis, then reconstructed."
  ],
  "file2.png": [
    "7e09b28b406c527b",
    "Projection of movement (3,4,0) onto a rail along the x-axis, separating rail travel from sideways travel."
  ],
  "file3.png": [
    "1865081ae0d2903f",
    "Three corner sensors with barycentric weights combine to locate P=(1,1,0) and interpolate a temperature of 35."
  ],
  "file4.png": [
    "7056fe61febcd80c",
    "Matrix-vector multiplication: coordinates (4,2) weight two basis vectors, producing components that sum to (6,10)."
  ],
  "file5.png": [
    "35ce719782189114",
    "Applying scale then a quarter-turn and doing the operations in reverse order move the same point to different locations."
  ],
  "file6.png": [
    "5905654e69a864ef",
    "A determinant summarizes volume scaling, orientation reversal, or collapse to a noninvertible dimension."
  ],
  "file7.png": [
    "9e0dc147179a4e60",
    "Nearly parallel matrix columns amplify a small observation change, shifting the recovered coefficients substantially."
  ],
  "file8.png": [
    "68a8fcd507587d48",
    "Flowchart for transforming a surface normal with an inverse-transpose and testing that it remains perpendicular to a tangent."
  ],
  "file9.png": [
    "0bb4424a78cf8994",
    "Vector reflection diagram showing an incident direction, surface normal, and reflected direction with the normal component reversed."
  ],
  "file10.png": [
    "1ef16abcf7628645",
    "Homogeneous coordinates distinguish a point, affected by rotation and translation, from a direction, affected by rotation only."
  ],
  "file12.png": [
    "f5495fd0ca3aa58a",
    "A single geometric displacement expressed using two rotated coordinate charts with different numerical components."
  ],
  "file13.png": [
    "f4602c76ebc25edf",
    "Coordinate pipeline maps an object point through model, view, clip, and perspective division to normalized device coordinates."
  ],
  "file14.png": [
    "01ef1d1332d51d33",
    "Axis-angle rotation becomes a unit quaternion and yields matching vector and matrix rotation results."
  ],
  "file15.png": [
    "48f76816ebb8a66b",
    "Quaternion interpolation decision diagram: choose equivalent signs, check near-parallel cases, then select interpolation."
  ],
  "file16.png": [
    "e39935e0aed22077",
    "Transform representation workflow from editable orientation and scale through local transforms to GPU matrix layout."
  ],
  "file17.png": [
    "2d5857c8e8f34a88",
    "Dependency diagram for invalidating a door object's world transform, inverse, normal matrix, and derived scene caches after an edit."
  ],
  "file18.png": [
    "5c2cf72fcaa4d10d",
    "Linear-system decision flow: test if the right-hand side lies in the column space, then use the null space to characterize solutions."
  ],
  "file19.png": [
    "f3dbd4dd34496fe2",
    "Pivoted LU decomposition workflow: permute rows, factor a matrix, solve the lower system, then back-substitute."
  ],
  "file21.png": [
    "d4c4a32f576456e1",
    "Factorization decision tree selecting pivoted LU, Cholesky, QR or SVD, eigensolvers, or sparse iterative methods by matrix structure."
  ],
  "file22.png": [
    "e4e424d7b45ba80a",
    "Cholesky whitening transforms a correlated Gaussian covariance ellipse into a circle in whitened coordinates."
  ],
  "file23.png": [
    "5a607ac918da6fcd",
    "Symmetric eigendecomposition separates orthonormal basis rotation, diagonal stretching, and matrix-function operations."
  ],
  "file24.png": [
    "49aff0f566229019",
    "Plots of a function, its local tangent approximation, and linearization error near the expansion point."
  ],
  "file25.png": [
    "bb0c8271ad7e58cd",
    "Plots showing accumulated area under a curve and the resulting integral function with its derivative."
  ],
  "file26.png": [
    "68273cf85d8c50b9",
    "Log-scale error plots compare quadrature methods and observed convergence against a reference solution."
  ],
  "file27.png": [
    "9185c9f2e87eb15d",
    "Two-dice outcome grid and a histogram showing that sums near seven are more likely than extreme sums."
  ],
  "file28.png": [
    "69f351b65a9e518c",
    "Four-panel graphic contrasting vector measurements, coordinate transformations, distances, and covariance geometry."
  ],
  "file29.png": [
    "fafa129749b155c9",
    "Plots demonstrating that zero covariance can coexist with nonlinear dependence, unlike independence."
  ],
  "file30.png": [
    "2b6fb3ca5f8094cc",
    "Singular value decomposition workflow: rotate by V-transpose, stretch by singular values, then rotate by U."
  ],
  "file31.png": [
    "b681e335dbaa5c1c",
    "Four plots show how a unit circle and a marked direction change through each stage of a two-dimensional singular value decomposition."
  ],
  "file33.png": [
    "b377e10662d8f98e",
    "Diagram showing singular values, amplification of weak directions, truncated pseudoinverse, and low-rank compression."
  ],
  "file35.png": [
    "0eb2e4bcad19d67d",
    "Heat equation workflow from a rod, material properties, and boundary conditions to eigenmodes and their time decay."
  ],
  "file36.png": [
    "f091d8a7295ced47",
    "Triangular finite-element workflow from mesh geometry through stiffness and mass matrices to generalized eigenmodes and time stepping."
  ],
  "file37.png": [
    "29a27e523c6c86d5",
    "Plots comparing bounded and accumulating energy error for numerical oscillator time-stepping methods."
  ],
  "file38.png": [
    "616d7ef2ae45639c",
    "Finite-difference error plot showing truncation error at large step sizes and rounding error at very small steps."
  ],
  "file43.png": [
    "0839cb8dc8c8edf6",
    "Matrix-function workflow: diagonalize a matrix, apply a scalar function to its eigenvalues, and transform back."
  ],
  "file44.png": [
    "a7892eb1c1abdfe5",
    "Objective and iteration plots illustrating how a full Newton step can leave a safe region and backtracking controls progress."
  ],
  "file47.png": [
    "78c90b4d742a4824",
    "Numerical accuracy workflow linking data scaling, discretization, stable arithmetic, conditioning, and error-budget tests."
  ],
  "file48.png": [
    "21d825e2ad7ae2b2",
    "Comparison of summation algorithms showing how rounding can discard small increments in a large total."
  ],
  "file49.png": [
    "57f4483dea652306",
    "Error curves comparing a cancellation-prone algebraic formula with a numerically stable alternative."
  ],
  "file50.png": [
    "ac8266cf6a7ca90b",
    "Polynomial interpolation plots show oscillations with equispaced nodes and improvement using Chebyshev extrema."
  ],
  "file51.png": [
    "98958b76dbb46cca",
    "Cubic Bezier and Hermite curve plots showing matching shapes and the Bernstein blending weights."
  ],
  "file52.png": [
    "940b20bf4fa71686",
    "Root-finding convergence plots compare bisection, Newton, and secant-style updates alongside bracket safety."
  ],
  "file53.png": [
    "9930530b8a23dbcf",
    "Toy linear-congruential random stream showing a long state cycle but visible dependence and repeated structure."
  ],
  "file54.png": [
    "fb735717c9a34580",
    "Disk sampling plots compare area-uniform points with radius-uniform points and their radial distributions."
  ],
  "file56.png": [
    "ac6d605a659dda4f",
    "Monte Carlo error plot contrasts decreasing random sampling error with a fixed bias floor."
  ],
  "file57.png": [
    "93e08da0f114a0c6",
    "Importance-sampling plots show how matching a proposal to the integrand can make the weighted estimate nearly constant."
  ],
  "file59.png": [
    "f7eaef90cb24795f",
    "Sampling-support diagram shows why an estimator must not assign zero proposal probability where the integrand contributes."
  ],
  "file62.png": [
    "34687a32cc68d8ea",
    "Statistical inference pipeline from assumptions and observations to an estimator, uncertainty assessment, and action policy."
  ],
  "file63.png": [
    "37b96c42be97b460",
    "Reservoir sampling illustration comparing candidate weights and their probabilities of being selected after replacement."
  ],
  "file64.png": [
    "6e7fbd9e4d57cc86",
    "Rasterization pipeline from vertex data and transforms through clipping, viewport, coverage, shading, blending, and storage."
  ]
}


IMG_RE=re.compile(rb"<img(?=[\s/>])[^>]*>",re.DOTALL)
SRC_RE=re.compile(rb'(?<![\w:-])src\s*=\s*(["\x27])([^"\x27]+)\1')
ALT_RE=re.compile(rb'(?<![\w:-])alt\s*=\s*(["\x27])([^"\x27]*)\1')


def _tag(el):
    return el.tag.rsplit("}",1)[-1]


def _collect_nav(root):
    nav=[x for x in root.iter() if _tag(x)=="nav" and x.get("id")=="toc"]
    if len(nav)!=1:
        raise RuntimeError("Book Two EPUB has no unique toc nav")
    return nav[0]


def _prune_ul(ordered_list, depth=0):
    removed=0
    for item in list(ordered_list):
        if _tag(item)!="li":
            continue
        if depth>=2:
            ordered_list.remove(item)
            removed+=1
        else:
            for nested in list(item):
                if _tag(nested)=="ol":
                    removed+=_prune_ul(nested,depth+1)
                    if not len(nested):
                        item.remove(nested)
    return removed


def _prune_ncx(parent,depth=0):
    removed=0
    for item in list(parent):
        if _tag(item)=="navPoint":
            if depth>=2:
                parent.remove(item)
                removed+=1
            else:
                removed+=_prune_ncx(item,depth+1)
    return removed


def _alt_fix_bytes(source,epub_path,z,seen):
    def replace_image(match):
        tag=match.group(0)
        s=SRC_RE.search(tag);a=ALT_RE.search(tag)
        if not s or not a:
            return tag
        old=html.unescape(a.group(2).decode("utf-8"))
        if old!="Diagram" and "/" not in old and not old.lower().endswith(".png"):
            return tag
        src=s.group(2).decode("utf-8")
        location=posixpath.normpath(posixpath.join(posixpath.dirname(epub_path),
                         urllib.parse.unquote(src)))
        ident=location.rsplit("/",1)[-1]
        definition=ALT_BY_FILENAME.get(ident)
        if not definition:
            raise RuntimeError(f"Unreviewed weak image ALT in {epub_path}: {ident} {old!r}")
        sha_prefix,description=definition
        actual=hashlib.sha256(z.read(location)).hexdigest()
        if not actual.startswith(sha_prefix):
            raise RuntimeError(f"Book Two image {ident} changed since its description review: {actual}")
        if location in seen:
            raise RuntimeError(f"Repeated ambiguous raster in Book Two EPUB: {location}")
        seen.add(location)
        new_alt=html.escape(description,quote=True).encode("utf-8")
        return tag[:a.start(2)]+new_alt+tag[a.end(2):]
    return IMG_RE.sub(replace_image,source)


def polish_book02_epub(path: str|Path) -> dict:
    """Atomically polish an already built Book Two EPUB and return proof metrics.

    All unchanged zip members remain byte-identical, with the required
    uncompressed first 'mimetype' entry and original member ordering.
    """
    path=Path(path)
    updates={};seen=set()
    with zipfile.ZipFile(path,"r") as archive:
        infos=archive.infolist()
        if not infos or infos[0].filename!="mimetype" or infos[0].compress_type!=zipfile.ZIP_STORED:
            raise RuntimeError("EPUB mimetype ordering/compression is invalid")
        for info in infos:
            fname=info.filename
            if fname.startswith("EPUB/text/") and fname.endswith(".xhtml"):
                original=archive.read(fname)
                changed=_alt_fix_bytes(original,fname,archive,seen)
                if changed!=original:
                    updates[fname]=changed
        if len(seen)!=len(ALT_BY_FILENAME):
            raise RuntimeError(f"Only {len(seen)} of {len(ALT_BY_FILENAME)} reviewed image descriptions applied")
        source_nav=archive.read("EPUB/nav.xhtml")
        nav=ET.fromstring(source_nav)
        toc=_collect_nav(nav)
        list_nodes=[n for n in toc if _tag(n)=="ol"]
        if len(list_nodes)!=1:
            raise RuntimeError("Unexpected Book Two EPUB navigation hierarchy")
        removed=_prune_ul(list_nodes[0])
        navigation_count=sum(_tag(n)=="li" for n in toc.iter())
        if navigation_count!=397:
            raise RuntimeError(f"Unexpected top-two-level nav count: {navigation_count}")
        if removed:
            updates["EPUB/nav.xhtml"]=ET.tostring(nav,encoding="utf-8",xml_declaration=True)
        opf=ET.fromstring(archive.read("EPUB/content.opf"))
        cover_ids=[n.get("id") for n in opf.iter()
                   if _tag(n)=="item" and "cover-image" in n.get("properties","").split()]
        if len(cover_ids)!=1:
            raise RuntimeError("Book Two EPUB missing unique cover image")
        ncx=ET.fromstring(archive.read("EPUB/toc.ncx"))
        maps=[el for el in ncx.iter() if _tag(el)=="navMap"]
        if len(maps)!=1:
            raise RuntimeError("Book Two EPUB missing unique fallback navMap")
        removed_ncx=_prune_ncx(maps[0])
        fallback_count=sum(_tag(n)=="navPoint" for n in ncx.iter())
        if fallback_count!=398:
            raise RuntimeError(f"Unexpected fallback NCX count: {fallback_count}")
        metadata_changed=False
        for node in ncx.iter():
            if _tag(node)!="meta":continue
            if node.get("name")=="dtb:depth" and node.get("content")!="2":
                node.set("content","2");metadata_changed=True
            if node.get("name")=="cover" and node.get("content")!=cover_ids[0]:
                node.set("content",cover_ids[0]);metadata_changed=True
        if removed_ncx or metadata_changed:
            updates["EPUB/toc.ncx"]=ET.tostring(ncx,encoding="utf-8",xml_declaration=True)
        if updates:
            with tempfile.NamedTemporaryFile(prefix=path.name+".a11y-",
                    suffix=".tmp",dir=path.parent,delete=False) as handle:
                tmp=Path(handle.name)
            try:
                with zipfile.ZipFile(tmp,"w") as output:
                    for info in infos:
                        # Critical: zipfile mutates ZipInfo.header_offset.
                        output.writestr(copy.copy(info),
                                        updates.get(info.filename,archive.read(info.filename)))
                with zipfile.ZipFile(tmp,"r") as check:
                    if check.testzip() is not None or check.namelist()!=archive.namelist():
                        raise RuntimeError("Repacked EPUB failed CRC/member ordering")
                    for info in infos:
                        if info.filename not in updates and check.read(info.filename)!=archive.read(info.filename):
                            raise RuntimeError("Unexpected Book Two EPUB member changed: "+info.filename)
                os.replace(tmp,path)
            finally:
                tmp.unlink(missing_ok=True)
    return {
        "descriptive_image_alts":len(seen),
        "html_nav_items":navigation_count,
        "ncx_nav_items":fallback_count,
        "removed_deep_nav_items":removed,
        "removed_deep_ncx_items":removed_ncx,
        "correct_cover_id":cover_ids[0],
        "changed_members":list(updates),
    }
