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
#include "advanced.hpp"
#include "geometry.hpp"
#include "exact_predicates.hpp"
#include <bit>

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
    // Householder QR: known solution, full-rank and rank-deficient examples.
    const std::array<double,10> tall{{1,0,1,1,1,2,1,3,1,4}};
    const std::array<double,5> targets{{1,3,5,7,9}};
    const auto qr=advanced::qr_least_squares(tall,5,2,targets);
    assert(qr);
    near(qr->coefficients[0],1); near(qr->coefficients[1],2);
    near(qr->rms,0);
    const std::array<double,6> deficient{{1,1,2,2,3,3}};
    const std::array<double,3> deficient_rhs{{2,4,6}};
    assert(!advanced::qr_least_squares(deficient,3,2,deficient_rhs));
    const std::array<double,2> impossible{{1,std::numeric_limits<double>::infinity()}};
    const std::array<double,2> tiny_rhs{{0,1}};
    assert(!advanced::qr_least_squares(impossible,2,1,tiny_rhs));
    // Independently differentiate the analytic residual by central differences.
    const advanced::Parameters p{2,-.7,.5};
    const advanced::Observation observation{1.2,.9};
    const auto jac=advanced::residual_and_jacobian(observation,p);
    assert(jac);
    const double epsilon=1e-6;
    for (std::size_t k=0;k<3;++k) {
        auto lo=p,hi=p;
        if (k==0) { lo.a-=epsilon; hi.a+=epsilon; }
        if (k==1) { lo.b-=epsilon; hi.b+=epsilon; }
        if (k==2) { lo.c-=epsilon; hi.c+=epsilon; }
        const auto minus=advanced::residual_and_jacobian(observation,lo);
        const auto plus=advanced::residual_and_jacobian(observation,hi);
        assert(minus && plus);
        near((*jac)[k+1],((*plus)[0]-(*minus)[0])/(2*epsilon),1e-8);
    }
    // Gross-outlier experiment; do not compare costs of DIFFERENT objectives.
    std::array<advanced::Observation,35> observations{};
    for (std::size_t i=0;i<observations.size();++i) {
        const double position=static_cast<double>(i)/10;
        observations[i]={position,2*std::exp(-.7*position)+.5+
                         .008*std::sin(.77*static_cast<double>(i))};
    }
    observations[15].y+=3.5;
    const auto robust=advanced::fit_exponential_huber(
        observations,{1.5,-.4,.3},.05);
    const auto ordinary=advanced::fit_exponential_huber(
        observations,{1.5,-.4,.3},1e6);
    assert(robust && ordinary && robust->converged && ordinary->converged);
    auto parameter_error=[](advanced::Parameters v) {
        return std::hypot(std::hypot(v.a-2,v.b+.7),v.c-.5);
    };
    assert(parameter_error(robust->parameters)<.05);
    assert(parameter_error(robust->parameters)<parameter_error(ordinary->parameters));
    assert(robust->accepted_steps>0 && ordinary->accepted_steps>0);
    assert(!advanced::fit_exponential_huber(observations,{1.5,-.4,.3},0));
    // Native geometric consensus: validate exact geometry, support, and RMS.
    const auto implied=geometry::ideal_trials(.5,.99);
    assert(implied && *implied==17); // idealized pair-sampling probability
    const auto direct=geometry::through({0,1},{2,5});
    assert(direct);
    near(geometry::signed_distance(*direct,{1,3}),0);
    assert(!geometry::through({1,1},{1,1}));
    const std::array<geometry::Point,4> square{{{1,0},{-1,0},{0,1},{0,-1}}};
    assert(!geometry::total_least_squares(square));
    const auto labelled=geometry::synthetic_line_data();
    assert(labelled.size()==110);
    std::vector<geometry::Point> points;
    for (const auto sample:labelled) points.push_back(sample.p);
    const auto consensus=geometry::ransac_line(points,.04,400,73013);
    const auto all_tls=geometry::total_least_squares(points);
    assert(consensus && all_tls);
    std::size_t true_positive=0,false_positive=0;
    for (auto i:consensus->inliers) {
        if (labelled[i].true_inlier) ++true_positive;
        else ++false_positive;
    }
    assert(true_positive>=78 && false_positive<=2);
    assert(std::abs(consensus->line.a-direct->a)<.02);
    assert(std::abs(consensus->line.c-direct->c)<.02);
    assert(std::isfinite(consensus->inlier_rms) && consensus->inlier_rms<.01);
    assert(!geometry::ransac_line(points,-1,400,73013));
    assert(!geometry::ransac_line(points,.04,0,73013));
    // Exact binary64 oracle: compare cases that a floating determinant cannot
    // classify reliably, including overflow and the smallest subnormal.
    using ES=exact::Sign;
    assert(exact::orientation({0,0},{2,0},{0,2})==ES::positive);
    assert(exact::orientation({0,0},{0,2},{2,0})==ES::negative);
    assert(exact::orientation({0,0},{1,1},{2,2})==ES::zero);
    const double maxfloat=std::numeric_limits<double>::max();
    assert(exact::orientation({maxfloat,maxfloat},{-maxfloat,maxfloat},
                              {0,-maxfloat})==ES::positive);
    const double subnormal=std::numeric_limits<double>::denorm_min();
    assert(exact::orientation({0,0},{subnormal,0},{0,subnormal})==ES::positive);
    assert(exact::orientation({0,0},{subnormal,0},{2*subnormal,0})==ES::zero);
    const double epsilon=std::numeric_limits<double>::epsilon();
    const double perturbed=std::nextafter(2*epsilon,
                                          std::numeric_limits<double>::infinity());
    assert(exact::orientation({0,0},{1,epsilon},{2,perturbed})==ES::positive);
    assert(!exact::orientation({0,0},{1,
        std::numeric_limits<double>::infinity()},{2,1}));
    const std::array<Grid,3> gridpoints{{{1000000000,1000000000},
        {-1000000000,1000000000},{0,-1000000000}}};
    const auto grid_sign=orient_exact_grid(gridpoints[0],gridpoints[1],gridpoints[2]);
    assert(grid_sign && *grid_sign==Sign::positive);
    assert(exact::orientation(
        {double(gridpoints[0].x),double(gridpoints[0].y)},
        {double(gridpoints[1].x),double(gridpoints[1].y)},
        {double(gridpoints[2].x),double(gridpoints[2].y)})==ES::positive);
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
    } else if (cmd=="orientation-exact" || cmd=="orientation-audit") {
        const double huge=std::numeric_limits<double>::max();
        const double tiny=std::numeric_limits<double>::denorm_min();
        const double eps=std::numeric_limits<double>::epsilon();
        using Triangle=std::array<exact::Point,3>;
        std::vector<Triangle> cases{
            Triangle{{{0,0},{2,0},{0,2}}},
            Triangle{{{0,0},{0,2},{2,0}}},
            Triangle{{{0,0},{1,1},{2,2}}},
            Triangle{{{huge,huge},{-huge,huge},{0,-huge}}},
            Triangle{{{0,0},{tiny,0},{0,tiny}}},
            Triangle{{{0,0},{1,eps},{2,std::nextafter(2*eps,huge)}}}
        };
        if (cmd=="orientation-audit") {
            std::mt19937_64 rng(20261120);
            for (std::size_t i=0;i<500;++i) {
                Triangle t{};
                for (auto &point:t) {
                    for (double *coordinate:{&point.x,&point.y}) {
                        do { *coordinate=std::bit_cast<double>(rng()); }
                        while (!std::isfinite(*coordinate));
                    }
                }
                cases.push_back(t);
            }
            std::cout<<"ax,ay,bx,by,cx,cy,exact_sign,filter_sign\n";
            for (const auto& t:cases) {
                const auto sign=exact::orientation(t[0],t[1],t[2]);
                if (!sign) return 2;
                for (const auto p:t)
                    std::cout<<std::hexfloat<<p.x<<","<<p.y<<",";
                const auto filtered=orient_filtered(
                    {t[0].x,t[0].y},{t[1].x,t[1].y},{t[2].x,t[2].y});
                std::cout<<std::defaultfloat<<int(*sign)<<","
                         <<int(filtered)<<"\n";
            }
        } else {
            std::cout<<"case,exact_sign,filter_sign\n";
            for (std::size_t i=0;i<cases.size();++i) {
                const auto& t=cases[i];
                const auto sign=exact::orientation(t[0],t[1],t[2]);
                if (!sign) return 2;
                const auto filtered=orient_filtered(
                    {t[0].x,t[0].y},{t[1].x,t[1].y},{t[2].x,t[2].y});
                std::cout<<i<<","<<int(*sign)<<","<<int(filtered)<<"\n";
            }
        }
    } else if (cmd=="ransac-summary" || cmd=="ransac-data") {
        const auto labelled=geometry::synthetic_line_data();
        std::vector<geometry::Point> points;
        for (const auto item:labelled) points.push_back(item.p);
        constexpr double threshold=.04;
        const auto ransac=geometry::ransac_line(points,threshold,400,73013);
        const auto all=geometry::total_least_squares(points);
        if (!ransac || !all || std::abs(ransac->line.b)<1e-12 ||
            std::abs(all->b)<1e-12) return 2;
        std::vector<bool> selected(points.size(),false);
        std::size_t tp=0,fp=0;
        for (auto i:ransac->inliers) {
            selected[i]=true;
            if (labelled[i].true_inlier) ++tp;
            else ++fp;
        }
        auto y_at=[](double x,geometry::Line l) {
            return -(l.a*x+l.c)/l.b;
        };
        if (cmd=="ransac-summary") {
            const double precision=double(tp)/double(tp+fp);
            const double recall=double(tp)/80.;
            std::cout<<"trials,threshold,inliers,true_positive,false_positive,"
                        "precision,recall,inlier_rms,a,b,c,all_tls_a,all_tls_b,all_tls_c\n";
            std::cout<<ransac->trials<<","<<threshold<<","
                     <<ransac->inliers.size()<<","<<tp<<","<<fp<<","
                     <<precision<<","<<recall<<","<<ransac->inlier_rms<<","
                     <<ransac->line.a<<","<<ransac->line.b<<","
                     <<ransac->line.c<<","<<all->a<<","<<all->b<<","
                     <<all->c<<"\n";
        } else {
            std::cout<<"x,y,true_inlier,selected_inlier,true_y,ransac_y,"
                        "all_tls_y,ransac_distance,all_tls_distance\n";
            for (std::size_t i=0;i<points.size();++i) {
                const auto p=points[i];
                std::cout<<p.x<<","<<p.y<<","
                         <<int(labelled[i].true_inlier)<<","<<int(selected[i])<<","
                         <<2*p.x+1<<","<<y_at(p.x,ransac->line)<<","
                         <<y_at(p.x,*all)<<","
                         <<geometry::signed_distance(ransac->line,p)<<","
                         <<geometry::signed_distance(*all,p)<<"\n";
            }
        }
    } else if (cmd=="qr-sensitivity") {
        // C++23 is the source of truth for these rows; Python may plot them.
        std::cout<<"delta,coefficient_error,residual_rms,qr_diagonal_ratio\n";
        for (const double delta:{1e-1,1e-2,1e-3,1e-4,1e-5,1e-6,1e-7,1e-8}) {
            const std::array<double,8> a{{1,1,1,1+delta,1,1-delta,2,2}};
            std::array<double,4> y{{1,1-delta,1+delta,2}};
            y[1]+=1e-10; // specified observation perturbation
            const auto q=advanced::qr_least_squares(a,4,2,y);
            if (!q) return 2;
            const double error=std::hypot(q->coefficients[0]-2,q->coefficients[1]+1);
            std::cout<<delta<<","<<error<<","<<q->rms<<","
                     <<q->qr_diagonal_ratio<<"\n";
        }
    } else if (cmd=="robust-fit" || cmd=="robust-data") {
        std::array<advanced::Observation,35> observations{};
        for (std::size_t i=0;i<observations.size();++i) {
            const double position=static_cast<double>(i)/10;
            observations[i]={position,2*std::exp(-.7*position)+.5+
                             .008*std::sin(.77*static_cast<double>(i))};
        }
        observations[15].y+=3.5;
        const auto robust=advanced::fit_exponential_huber(
            observations,{1.5,-.4,.3},.05);
        const auto ordinary=advanced::fit_exponential_huber(
            observations,{1.5,-.4,.3},1e6);
        if (!robust || !ordinary) return 2;
        auto model=[](double x,advanced::Parameters p) {
            return p.a*std::exp(p.b*x)+p.c;
        };
        if (cmd=="robust-fit") {
            std::cout<<"method,a,b,c,objective,gradient_inf,iterations,"
                        "accepted,converged,parameter_error\n";
            auto report=[&](const char* method,const advanced::RobustFit& fit) {
                const auto p=fit.parameters;
                std::cout<<method<<","<<p.a<<","<<p.b<<","<<p.c<<","
                         <<fit.objective<<","<<fit.gradient_infinity<<","
                         <<fit.iterations<<","<<fit.accepted_steps<<","
                         <<int(fit.converged)<<","
                         <<std::hypot(std::hypot(p.a-2,p.b+.7),p.c-.5)<<"\n";
            };
            report("huber",*robust); report("least_squares",*ordinary);
        } else {
            std::cout<<"x,observed,truth,fit_huber,fit_least_squares,is_outlier\n";
            for (std::size_t i=0;i<observations.size();++i) {
                const auto o=observations[i];
                std::cout<<o.x<<","<<o.y<<","
                         <<model(o.x,{2,-.7,.5})<<","
                         <<model(o.x,robust->parameters)<<","
                         <<model(o.x,ordinary->parameters)<<","
                         <<int(i==15)<<"\n";
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
                    "{test|matrix|orientation|orientation-exact|orientation-audit|fit|heat|monte-carlo|qr-sensitivity|robust-fit|robust-data|ransac-summary|ransac-data}\n";
        return 2;
    }
}
