// CPU rasterization reference lab: edge functions, top-left ownership,
// barycentrics, perspective-correct interpolation and reversed depth sanity.

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#ifdef assert
#undef assert
#endif
#define ALLMIGHT_REQUIRE(expr) do { if (!(expr)) throw std::runtime_error("check failed: " #expr); } while (false)
#define assert(expr) ALLMIGHT_REQUIRE(expr)

namespace allmight {

struct V2 { double x{}, y{}; };
struct Vertex {
    V2 p;
    double clip_w = 1.0;
    double attr = 0.0;
    double depth = 0.0;
};

double edge(V2 a, V2 b, V2 p) {
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

bool top_left(V2 a, V2 b) {
    const double dy = b.y - a.y;
    const double dx = b.x - a.x;
    return dy > 0.0 || (dy == 0.0 && dx < 0.0);
}

struct Sample {
    bool covered = false;
    std::array<double,3> lambda{};
    double perspective_attr = 0.0;
};

Sample raster_sample(std::array<Vertex,3> v, V2 p) {
    // Ensure positive signed area under this edge-function convention.
    double area = edge(v[0].p, v[1].p, v[2].p);
    if (std::abs(area) < 1.0e-12) return {};
    if (area < 0.0) {
        std::swap(v[1], v[2]);
        area = -area;
    }

    const std::array<double,3> e{
        edge(v[1].p, v[2].p, p),
        edge(v[2].p, v[0].p, p),
        edge(v[0].p, v[1].p, p)
    };
    const std::array<bool,3> tl{
        top_left(v[1].p, v[2].p),
        top_left(v[2].p, v[0].p),
        top_left(v[0].p, v[1].p)
    };

    for (int i=0;i<3;++i) {
        if (e[static_cast<std::size_t>(i)] < 0.0) return {};
        if (e[static_cast<std::size_t>(i)] == 0.0 && !tl[static_cast<std::size_t>(i)]) return {};
    }

    Sample s;
    s.covered = true;
    for (int i=0;i<3;++i) s.lambda[static_cast<std::size_t>(i)] = e[static_cast<std::size_t>(i)] / area;

    double numerator = 0.0;
    double denominator = 0.0;
    for (int i=0;i<3;++i) {
        const double inv_w = 1.0 / v[static_cast<std::size_t>(i)].clip_w;
        numerator += s.lambda[static_cast<std::size_t>(i)] * v[static_cast<std::size_t>(i)].attr * inv_w;
        denominator += s.lambda[static_cast<std::size_t>(i)] * inv_w;
    }
    if (!(std::abs(denominator) > 1.0e-15)) throw std::runtime_error("invalid perspective denominator");
    s.perspective_attr = numerator / denominator;
    return s;
}

double reversed_depth_infinite(double near_plane, double positive_view_distance) {
    if (!(near_plane > 0.0) || !(positive_view_distance >= near_plane)) {
        throw std::invalid_argument("invalid reversed depth inputs");
    }
    // A useful infinite-far reversed-Z mapping for a [0,1] depth convention.
    return near_plane / positive_view_distance;
}

void tests() {
    {
        const std::array<Vertex,3> tri{{
            {{1.0,1.0}, 1.0, 0.0, 0.2},
            {{6.0,1.0}, 2.0, 1.0, 0.5},
            {{1.0,6.0}, 4.0, 0.0, 0.8}
        }};
        const V2 p{2.5,2.5};
        const Sample s = raster_sample(tri,p);
        assert(s.covered);
        const double sum=s.lambda[0]+s.lambda[1]+s.lambda[2];
        assert(std::abs(sum-1.0)<1.0e-12);

        const double affine=s.lambda[1];
        assert(std::abs(s.perspective_attr-affine)>1.0e-3);
        assert(s.perspective_attr>=0.0 && s.perspective_attr<=1.0);
    }

    {
        // Two triangles share the diagonal of a square. At sample centers the
        // top-left rule must assign every covered sample to exactly one owner.
        const std::array<Vertex,3> a{{
            {{0.0,0.0},1.0,0.0,0.0},
            {{4.0,0.0},1.0,0.0,0.0},
            {{4.0,4.0},1.0,0.0,0.0}
        }};
        const std::array<Vertex,3> b{{
            {{0.0,0.0},1.0,0.0,0.0},
            {{4.0,4.0},1.0,0.0,0.0},
            {{0.0,4.0},1.0,0.0,0.0}
        }};
        int covered=0;
        for(int y=0;y<4;++y){
            for(int x=0;x<4;++x){
                const V2 p{static_cast<double>(x)+0.5,static_cast<double>(y)+0.5};
                const int owners=(raster_sample(a,p).covered?1:0)+(raster_sample(b,p).covered?1:0);
                assert(owners==1);
                covered += owners;
            }
        }
        assert(covered==16);
    }

    {
        const double n=0.1;
        const double d_near=reversed_depth_infinite(n,0.1);
        const double d_mid=reversed_depth_infinite(n,10.0);
        const double d_far=reversed_depth_infinite(n,10000.0);
        assert(std::abs(d_near-1.0)<1.0e-12);
        assert(d_near>d_mid && d_mid>d_far && d_far>0.0);
    }
}

} // namespace allmight

int main(){
    allmight::tests();
    std::cout<<"rasterizer_reference_lab: PASS\n";
    return 0;
}
