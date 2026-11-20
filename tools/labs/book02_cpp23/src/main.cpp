// Book Two: self-contained, documented, standard C++23 numerical experiments.
#include <algorithm>
#include <array>
#ifdef NDEBUG
#undef NDEBUG  // The explicit test command must check contracts even in Release.
#endif
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <optional>
#include <random>
#include <span>
#include <string_view>
#include <vector>

namespace book02 {
struct Vec2 { double x{}, y{}; };
struct Mat2 { double a00{}, a01{}, a10{}, a11{}; };
Vec2 apply(Mat2 a, Vec2 x) {
    return {a.a00*x.x+a.a01*x.y, a.a10*x.x+a.a11*x.y};
}
std::optional<Vec2> solve(Mat2 a, Vec2 b) {
    // Scale and pivot. This 2x2 reference is not a general-purpose LAPACK solver.
    const double s=std::max({std::abs(a.a00),std::abs(a.a01),
                             std::abs(a.a10),std::abs(a.a11)});
    if (!(s>0) || !std::isfinite(s)) return std::nullopt;
    double p=a.a00/s, q=a.a01/s, r=a.a10/s, t=a.a11/s;
    double u=b.x/s, v=b.y/s;
    if (std::abs(r)>std::abs(p)) {
        std::swap(p,r);std::swap(q,t);std::swap(u,v);
    }
    constexpr double tol=64*std::numeric_limits<double>::epsilon();
    if (std::abs(p)<=tol) return std::nullopt;
    const double factor=r/p, pivot=t-factor*q;
    if (!std::isfinite(pivot) || std::abs(pivot)<=tol) return std::nullopt;
    const double y=(v-factor*u)/pivot, x=(u-q*y)/p;
    if (!std::isfinite(x)||!std::isfinite(y)) return std::nullopt;
    return Vec2{x,y};
}
enum class Sign { negative=-1, zero=0, positive=1, uncertain=2 };
struct Grid { std::int32_t x{}, y{}; };
std::optional<Sign> orient_exact_grid(Grid a, Grid b, Grid c) {
    // Bound each input to +/-1e9: then the signed determinant fits int64_t.
    constexpr std::int64_t lim=1000000000LL;
    auto valid=[](Grid p){
        return std::abs(std::int64_t{p.x})<=lim &&
               std::abs(std::int64_t{p.y})<=lim;
    };
    if (!valid(a)||!valid(b)||!valid(c)) return std::nullopt;
    const std::int64_t ax=std::int64_t{a.x}-c.x, ay=std::int64_t{a.y}-c.y;
    const std::int64_t bx=std::int64_t{b.x}-c.x, by=std::int64_t{b.y}-c.y;
    const std::int64_t det=ax*by-ay*bx;
    return det>0?Sign::positive:det<0?Sign::negative:Sign::zero;
}
Sign orient_filtered(Vec2 a, Vec2 b, Vec2 c) {
    // Floating-point filter. An uncertain result requires an exact fallback.
    if (!std::isfinite(a.x)||!std::isfinite(a.y)||!std::isfinite(b.x)||
        !std::isfinite(b.y)||!std::isfinite(c.x)||!std::isfinite(c.y))
        return Sign::uncertain;
    const double p=(a.x-c.x)*(b.y-c.y);
    const double q=(a.y-c.y)*(b.x-c.x);
    const double det=p-q;
    if (!std::isfinite(p)||!std::isfinite(q)||!std::isfinite(det))
        return Sign::uncertain;
    const double bound=16*std::numeric_limits<double>::epsilon()*
                       (std::abs(p)+std::abs(q))+
                       std::numeric_limits<double>::min();
    return det>bound?Sign::positive:det< -bound?Sign::negative:Sign::uncertain;
}
struct Sample { double x{}, y{}; };
struct Fit { double slope{}, intercept{}, rms{}; };
std::optional<Fit> fit_line(std::span<const Sample> pts) {
    // Centered least squares avoids cancellation in naive normal equations.
    if (pts.size()<2) return std::nullopt;
    double mx=0,my=0;
    for (const auto p:pts) {
        if (!std::isfinite(p.x)||!std::isfinite(p.y)) return std::nullopt;
        mx+=p.x/pts.size();my+=p.y/pts.size();
    }
    double xx=0,xy=0;
    for (const auto p:pts) {
        xx+=(p.x-mx)*(p.x-mx);
        xy+=(p.x-mx)*(p.y-my);
    }
    if (!(xx>0)||!std::isfinite(xx)||!std::isfinite(xy))
        return std::nullopt;
    const double slope=xy/xx,intercept=my-slope*mx;
    double residual=0;
    for (const auto p:pts)
        residual+=std::pow((slope*p.x+intercept)-p.y,2);
    const double rms=std::sqrt(residual/pts.size());
    if (!std::isfinite(slope)||!std::isfinite(intercept)||!std::isfinite(rms))
        return std::nullopt;
    return Fit{slope,intercept,rms};
}
std::optional<std::vector<double>>
heat_step(std::span<const double> u,double alpha,double dx,double dt) {
    // Explicit Euler with zero Dirichlet endpoints; r=alpha*dt/dx^2 <= 1/2.
    if (u.size()<3||!std::isfinite(alpha)||alpha<0||
        !std::isfinite(dx)||dx<=0||!std::isfinite(dt)||dt<=0)
        return std::nullopt;
    const double r=alpha*dt/(dx*dx);
    if (!std::isfinite(r)||r>0.5) return std::nullopt;
    std::vector<double> result(u.size(),0);
    for (std::size_t i=1;i+1<u.size();++i) {
        if (!std::isfinite(u[i-1])||!std::isfinite(u[i])||
            !std::isfinite(u[i+1])) return std::nullopt;
        result[i]=(1-2*r)*u[i]+r*(u[i-1]+u[i+1]);
    }
    return result;
}
struct Estimate { double mean{}, stderr{}, exact{}; };
std::optional<Estimate> integrate(std::uint64_t seed,std::size_t samples) {
    // Welford sample variance, independent analytic reference 2/pi.
    if (samples<2) return std::nullopt;
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> unit(0,1);
    double mean=0,m2=0;
    for (std::size_t i=1;i<=samples;++i) {
        const double y=std::sin(std::numbers::pi*unit(rng));
        const double d=y-mean;
        mean+=d/i;
        m2+=d*(y-mean);
    }
    return Estimate{mean,std::sqrt(m2/((samples-1)*double(samples))),
                    2/std::numbers::pi};
}
void self_test() {
    auto near=[](double a,double b,double e=1e-12) {
        assert(std::isfinite(a)&&std::abs(a-b)<=e);
    };
    auto x=solve({2,1,1,3},{1,-4.5});
    assert(x);near(x->x,1.5);near(x->y,-2);
    assert(!solve({1,2,2,4},{1,2}));
    assert(orient_exact_grid({0,0},{2,0},{0,2})==Sign::positive);
    assert(orient_exact_grid({0,2},{2,0},{0,0})==Sign::negative);
    assert(orient_exact_grid({0,0},{1,1},{2,2})==Sign::zero);
    assert(!orient_exact_grid({1500000000,0},{0,0},{1,1}));
    assert(orient_filtered({0,0},{1,1},{2,2})==Sign::uncertain);
    std::array<Sample,4> samples{{{0,1},{1,3},{2,5},{3,7}}};
    auto fit=fit_line(samples);
    assert(fit);near(fit->slope,2);near(fit->intercept,1);near(fit->rms,0);
    std::array<Sample,2> singular{{{1,2},{1,3}}};
    assert(!fit_line(singular));
    std::vector<double> pulse{0,0,1,0,0};
    auto h=heat_step(pulse,1,1,0.25);
    assert(h);near((*h)[1],0.25);near((*h)[2],0.5);near((*h)[3],0.25);
    assert(!heat_step(pulse,1,1,0.51));
    auto mc=integrate(12345,100000);
    assert(mc);
    near(mc->exact,2/std::numbers::pi);
    assert(std::abs(mc->mean-mc->exact)<0.015);
}
}
int main(int argc,char**argv) {
    using namespace book02;
    const std::string_view cmd=argc>1?argv[1]:"help";
    std::cout<<std::setprecision(17);
    if (cmd=="test") {
        self_test();std::cout<<"ALL_CXX23_CONTRACTS_OK\n";return 0;
    }
    if (cmd=="matrix") {
        const Mat2 m{2,1,1,3};
        const Vec2 b=apply(m,{1.5,-2});
        auto x=solve(m,b);
        if (!x) return 2;
        std::cout<<"component,solved\nx,"<<x->x<<"\ny,"<<x->y<<"\n";
    } else if (cmd=="orientation") {
        std::cout<<"method,sign\ninteger,"
                 <<int(*orient_exact_grid({0,0},{2,0},{0,2}))
                 <<"\nfiltered,"
                 <<int(orient_filtered({0,0},{2,0},{0,2}))<<"\n";
    } else if (cmd=="fit") {
        std::array<Sample,4> data{{{0,1},{1,3},{2,5},{3,7}}};
        auto f=fit_line(data);
        if (!f) return 2;
        std::cout<<"slope,intercept,rms\n"
                 <<f->slope<<","<<f->intercept<<","<<f->rms<<"\n";
    } else if (cmd=="heat") {
        std::vector<double> u{0,.25,.5,.75,1,.75,.5,.25,0};
        std::cout<<"step,x,temperature\n";
        for (int step=0;step<=20;++step) {
            for (std::size_t i=0;i<u.size();++i)
                std::cout<<step<<","<<i*.125<<","<<u[i]<<"\n";
            if (step<20) {
                auto next=heat_step(u,1,.125,.004);
                if (!next) return 2;
                u=*next;
            }
        }
    } else if (cmd=="monte-carlo") {
        std::cout<<"draws,estimate,stderr,reference\n";
        for (const std::size_t n:{100U,1000U,10000U,100000U}) {
            auto r=integrate(20261120,n);
            if (!r) return 2;
            std::cout<<n<<","<<r->mean<<","<<r->stderr<<","<<r->exact<<"\n";
        }
    } else {
        std::cerr<<"Usage: book02_workbench "
                    "{test|matrix|orientation|fit|heat|monte-carlo}\n";
        return 2;
    }
}
