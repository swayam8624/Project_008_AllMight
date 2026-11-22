#pragma once
// Book Two: deterministic-input 2D consensus fitting, written in C++23.
// This models Fischler--Bolles' core idea, not the original camera experiments.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <span>
#include <vector>
#include <utility>

namespace book02::geometry {
struct Point { double x{},y{}; };
// ax+by+c=0; hypot(a,b)=1; normal is canonically oriented for CSV comparison.
struct Line { double a{},b{},c{}; };
inline bool valid(Point p) { return std::isfinite(p.x)&&std::isfinite(p.y); }
inline std::optional<Line> canonical_line(double a,double b,double c) {
    if (!std::isfinite(a)||!std::isfinite(b)||!std::isfinite(c)) return std::nullopt;
    const double norm=std::hypot(a,b);
    if (!(norm>0)||!std::isfinite(norm)) return std::nullopt;
    a/=norm; b/=norm; c/=norm;
    if (a<0 || (a==0 && b<0)) { a=-a; b=-b; c=-c; }
    return Line{a,b,c};
}
inline double signed_distance(Line l,Point p) { return l.a*p.x+l.b*p.y+l.c; }
inline std::optional<Line> through(Point p,Point q) {
    if (!valid(p)||!valid(q)) return std::nullopt;
    const double dx=q.x-p.x,dy=q.y-p.y;
    const double norm=std::hypot(dx,dy);
    if (!(norm>0)||!std::isfinite(norm)) return std::nullopt;
    return canonical_line(-dy/norm,dx/norm,(dy*p.x-dx*p.y)/norm);
}
// Orthogonal total least squares: a normal to the largest-eigenvalue
// covariance direction. Isotropic or zero scatter has no unique line.
inline std::optional<Line> total_least_squares(std::span<const Point> pts) {
    if (pts.size()<2) return std::nullopt;
    double mx=0,my=0;
    for (const auto p:pts) {
        if (!valid(p)) return std::nullopt;
        mx+=p.x/static_cast<double>(pts.size());
        my+=p.y/static_cast<double>(pts.size());
    }
    if (!std::isfinite(mx)||!std::isfinite(my)) return std::nullopt;
    double xx=0,xy=0,yy=0;
    for (const auto p:pts) {
        const double x=p.x-mx,y=p.y-my;
        xx+=x*x; xy+=x*y; yy+=y*y;
    }
    const double trace=xx+yy, separation=std::hypot(xx-yy,2*xy);
    if (!(trace>0)||!std::isfinite(trace)||!std::isfinite(separation)||
        separation<=64*std::numeric_limits<double>::epsilon()*trace)
        return std::nullopt;
    const double theta=.5*std::atan2(2*xy,xx-yy);
    const double a=-std::sin(theta),b=std::cos(theta);
    return canonical_line(a,b,-a*mx-b*my);
}
inline std::optional<std::size_t> ideal_trials(double w,double confidence) {
    // Idealized: independent size-two draws, with-replacement approximation.
    if (!(w>0 && w<=1)||!(confidence>0 && confidence<1)||
        !std::isfinite(w)||!std::isfinite(confidence)) return std::nullopt;
    if (w==1) return 1;
    const double k=std::ceil(std::log1p(-confidence)/std::log1p(-w*w));
    if (!std::isfinite(k)||k>static_cast<double>(std::numeric_limits<std::size_t>::max()))
        return std::nullopt;
    return std::max<std::size_t>(1,static_cast<std::size_t>(k));
}
struct Result {
    Line line;
    std::vector<std::size_t> inliers;
    std::size_t trials{};
    double inlier_rms{};
};
inline std::optional<Result> ransac_line(
    std::span<const Point> pts,double threshold,std::size_t trials,std::uint64_t seed) {
    if (pts.size()<2 || !(threshold>0) || !std::isfinite(threshold) || !trials)
        return std::nullopt;
    for (const auto p:pts) if (!valid(p)) return std::nullopt;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::size_t> first(0,pts.size()-1);
    std::uniform_int_distribution<std::size_t> second(0,pts.size()-2);
    std::vector<std::size_t> best;
    double best_squared=std::numeric_limits<double>::infinity();
    for (std::size_t run=0;run<trials;++run) {
        const auto i=first(rng);
        auto j=second(rng);
        if (j>=i) ++j; // two distinct point indices
        const auto line=through(pts[i],pts[j]);
        if (!line) continue; // duplicate coordinates
        std::vector<std::size_t> support;
        double squared=0;
        for (std::size_t k=0;k<pts.size();++k) {
            const double d=signed_distance(*line,pts[k]);
            if (std::abs(d)<=threshold) {
                support.push_back(k);
                squared+=d*d;
            }
        }
        if (support.size()>best.size() ||
            (support.size()==best.size() && squared<best_squared)) {
            best=std::move(support);
            best_squared=squared;
        }
    }
    if (best.size()<2) return std::nullopt;
    std::vector<Point> selected;
    selected.reserve(best.size());
    for (auto i:best) selected.push_back(pts[i]);
    const auto refined=total_least_squares(selected);
    if (!refined) return std::nullopt;
    std::vector<std::size_t> after;
    double squared=0;
    for (std::size_t i=0;i<pts.size();++i) {
        const double d=signed_distance(*refined,pts[i]);
        if (std::abs(d)<=threshold) { after.push_back(i); squared+=d*d; }
    }
    if (after.size()<2 || !std::isfinite(squared)) return std::nullopt;
    const double rms=std::sqrt(squared/static_cast<double>(after.size()));
    return Result{*refined,std::move(after),trials,rms};
}
// The truth label is disclosed, not inferred by the fit. Inliers follow
// y=2*x+1 with bounded deterministic perturbation; outliers form a different
// structured line. The 110 rows are deliberately not a real camera dataset.
struct Labelled { Point p; bool true_inlier; };
inline std::vector<Labelled> synthetic_line_data() {
    std::vector<Labelled> data;
    data.reserve(110);
    for (std::size_t i=0;i<80;++i) {
        const double x=-4+8*static_cast<double>(i)/79;
        const double y=2*x+1+.015*std::sin(.27*static_cast<double>(i)+.4);
        data.push_back({{x,y},true});
    }
    for (std::size_t i=0;i<30;++i) {
        const double x=-3.8+7.6*static_cast<double>(i)/29;
        const double y=-7+.4*x+.06*std::cos(.51*static_cast<double>(i));
        data.push_back({{x,y},false});
    }
    return data;
}
} // namespace book02::geometry
