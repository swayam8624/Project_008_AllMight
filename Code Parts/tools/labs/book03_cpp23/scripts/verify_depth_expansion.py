#!/usr/bin/env python3
"""Compile the exact new teaching snippets and check independent numeric witnesses.

Scope: standalone extension examples, not full new production implementations.
Run from a checkout with Python 3 and a C++23 compiler; no NumPy needed.
"""
import argparse
import math
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[5]
CHAPTERS = ROOT / "Notes/Book Production/Seven-Book Curriculum/Book03/Chapters"
STARTS = {
    18: "## From individual samples to blocks of the image",
    20: "## Design a reconstruction filter",
    21: "## Trace one triangle",
    22: "## A lens has a useful",
    23: "## Different lenses",
    24: "## A photon-transfer experiment",
    25: "## A camera's three measurements",
    28: "## A corner has a useful",
    30: "## A depth value must pass",
    31: "## A studio needs a coordinate contract",
}
HARNESS = r"""
#include <cassert>
#include <iostream>
#include <random>

bool close(double a, double b, double tol = 1e-11) {
    return std::abs(a-b) <= tol;
}

int main() {
    // Tile bounds: enumerate all samples independently of classifier.
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> value(-30,30);
    int full = 0, empty = 0, uncertain = 0;
    for (int trial=0; trial<10000; ++trial) {
        std::array<EdgeTile,3> edges{};
        for (auto& e : edges)
            e = {value(rng), value(rng), value(rng), trial%2};
        const int w = 1 + trial%9, h = 1 + (trial/9)%7;
        int covered = 0;
        for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
            bool inside = true;
            for (const auto& e : edges)
                inside &= e.first+e.step_x*x+e.step_y*y >= e.threshold;
            covered += inside;
        }
        switch (classify(edges,w,h)) {
        case TileClass::full: assert(covered==w*h); ++full; break;
        case TileClass::empty: assert(covered==0); ++empty; break;
        case TileClass::uncertain: ++uncertain; break;
        }
    }
    assert(full && empty && uncertain);
    std::cout << "tile cases 10000: full=" << full << " empty=" << empty
              << " uncertain=" << uncertain << '\n';

    const std::array<double,5> constant{2,2,2,2,2}, impulse{0,0,1,0,0};
    assert(smooth_linear(constant)==std::vector<double>(5,2));
    assert(smooth_linear(impulse)==std::vector<double>({0,.25,.5,.25,0}));
    assert(smooth_linear(std::span<const double>{}).empty());

    // Uniform-area aperture: a deterministic stratified radial quadrature.
    double moment=0;
    for (int i=0;i<1000;++i) {
        auto [x,y]=aperture_point(2.,(i+.5)/1000.,.37);
        assert(x*x+y*y <= 4.+1e-12);
        moment += x*x+y*y;
    }
    assert(close(moment/1000.,2.));
    auto ray=equidistant_ray(std::numbers::pi_v<double>/3,0);
    assert(ray && close((*ray)[2],.5));
    assert(close((*ray)[0],std::sqrt(3.)/2));
    assert(!equidistant_ray(2,0));
    assert(equidistant_ray(0,0).value()==(std::array<double,3>{0,0,1}));

    const std::array<double,3> a{10,10,10}, b{12,10,8};
    assert(close(paired_variance(a,b),2.));
    const std::array<double,3> white{.95,1.,1.09};
    auto lab=lab_from_xyz(white,white);
    assert(close(lab[0],100) && close(lab[1],0) && close(lab[2],0));
    auto half=lab_from_xyz({.475,.5,.545},white);
    assert(close(half[0],116*std::cbrt(.5)-16));

    assert(close(refine_peak(7,10,9).value(),.25));
    assert(!refine_peak(1,1,1));
    assert(!refine_peak(2,0,2));
    const std::array<std::uint64_t,1> da{0b10110010},db{0b10100011};
    assert(hamming_words(da,db)==2);

    FusionCell cell;
    assert(integrate(cell,1.04,1.,.1,1.));
    assert(integrate(cell,.98,1.,.1,3.));
    assert(close(cell.phi,-.05) && close(cell.weight,4));
    assert(!integrate(cell,.5,1.,.1,1.));
    assert(close(cell.phi,-.05) && close(cell.weight,4));
    FusionCell reversed;
    assert(integrate(reversed,.98,1.,.1,3.));
    assert(integrate(reversed,1.04,1.,.1,1.));
    assert(close(reversed.phi,cell.phi));
    PixelTrace trace{};
    OutputRecord record{};
    assert(trace.surface_id==0 && record.inputs.frame_id==0);
    std::cout << "all extracted standalone C++23 snippet witnesses passed\n";
}
"""


def numeric_witnesses():
    def check(got, expected, tol=1e-10):
        if not math.isclose(got, expected, abs_tol=tol, rel_tol=tol):
            raise AssertionError((got, expected))

    # 18: affine edge extrema.
    edge = [3*x-2*y+4 for y in range(1,4) for x in range(2,5)]
    assert min(edge)==4 and max(edge)==14
    # 19: finite forward depth spacing and premultiplied composition.
    B = .1*1000/(1000-.1)
    check(1/(B*(2**24-1)), .000000596, 1e-10)
    check(100**2/(B*(2**24-1)), .00596, 1e-6)
    # 20: all mip texels in square power-of-two chain.
    assert sum(4**i for i in range(11))==1398101
    # 21: independently recover the sample's 3D point.
    a,b,c=.24,.25,.51
    H=a/2+b/2+c/4
    check(H,.3725)
    check((b/2+c*.5/4)/H,.5067114093959731)
    check((c/4)/H,.3422818791946309)
    q=(a+b)*5/9+c*5/6
    check(10/(10-9*q),1/H)
    # 22: thin-lens image distance and tolerance endpoints.
    v=lambda s:50*s/(s-50)
    check(12.5*abs(1-v(1000)/v(1010)),.00651380927566536,1e-10)
    # 23: distinct angular laws and inverse covariance scales.
    theta=math.pi/3
    check(2*math.sin(theta/2),1)
    check(2*math.tan(theta/2),2/math.sqrt(3))
    check(.5/.85,.5882352941176471)
    # 24: photon transfer slope and read variance.
    gain=(204-54)/(400-100)
    check(gain,.5)
    check(math.sqrt(54-gain*100)/gain,4)
    # 25: reference-relative lightness.
    check(116*math.cbrt(.5)-16,76.06926101415557)
    # 26: finite-difference pose derivative, same optical convention.
    def project(t):
        x=.5*math.cos(t)-.25*math.sin(t)
        y=.5*math.sin(t)+.25*math.cos(t)
        return 400*x,400*y
    p,m=project(1e-5),project(-1e-5)
    check((p[0]-m[0])/2e-5,-100,1e-8)
    check((p[1]-m[1])/2e-5,200,1e-8)
    # 27: disparity uncertainty and cost-volume payload.
    check(80/20**2*math.sqrt(2)*.25,.07071067811865475)
    assert 1280*720*128*4==450*2**20
    # 28: normalized Gaussian blob response peaks at its own scale.
    s=3
    response=lambda t:2*s*s*t*t/(s*s+t*t)**2
    check(response(s),.5)
    assert response(s)>response(s/2) and response(s)>response(2*s)
    # 29: check the Schur solution against all dense normal equations.
    c,p,q=2.5,.75,2.25
    check(2*c+p+q,8)
    check(c+2*p,4)
    check(c+2*q,7)
    # 30: dense and sparse memory and signed floor indexing.
    assert 512**3*8==2**30
    assert divmod(-1,8)==(-1,7)
    check(.3/(.3+.1),.75)
    # 31: explicit scale before world translation.
    check(1000*.001+2,3)
    # 32: geometric versus elapsed-sum speedups.
    check(math.sqrt((10/5)*(1000/1100)),1.348399724926484)
    check((10+1000)/(5+1100),.9140271493212669)
    print("15 chapter-specific analytic witness families passed")


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler",default="c++")
    parser.add_argument("--chapters",type=Path,default=CHAPTERS)
    args=parser.parse_args()
    snippets=[]
    for number, marker in STARTS.items():
        path,=args.chapters.glob(f"{number:02d} - *.md")
        text=path.read_text(encoding="utf-8")
        start=text.index(marker)
        end=re.search(r"\n## (?:Your turn|Exercises)",text[start:]).start()+start
        snippets.extend(re.findall(r"~~~cpp\n([\s\S]*?)\n~~~",text[start:end]))
    assert len(snippets)==11, f"expected 11 exact snippets, got {len(snippets)}"
    numeric_witnesses()
    with tempfile.TemporaryDirectory(prefix="book03-depth-") as tmp:
        cpp=Path(tmp)/"witness.cpp"
        exe=Path(tmp)/"witness"
        cpp.write_text("\n\n".join(snippets)+HARNESS,encoding="utf-8")
        subprocess.run([args.compiler,"-std=c++23","-O2","-Wall","-Wextra","-Wpedantic",
                        "-Werror",str(cpp),"-o",str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
    print(f"PASS {len(snippets)} exact manuscript snippets compiled and exercised")


if __name__=="__main__":
    main()
