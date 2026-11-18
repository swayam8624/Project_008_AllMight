// Mathematics continuous-note C++20 first-contact oracle; code and tested source stay identical.
#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>
struct Vec2 { double x{}, y{}; };
struct Disk2 { Vec2 centre{}, velocity{}; double radius{}; };
double dot(Vec2 a, Vec2 b) { return a.x*b.x+a.y*b.y; }

std::optional<double> firstTouch(Disk2 a, Disk2 b, double h) {
    const auto valid = [](Disk2 d) {
        return std::isfinite(d.centre.x)&&std::isfinite(d.centre.y)&&
               std::isfinite(d.velocity.x)&&std::isfinite(d.velocity.y)&&
               std::isfinite(d.radius)&&d.radius>0;
    };
    if (!valid(a)||!valid(b)||!std::isfinite(h)||h<0)
        throw std::invalid_argument("invalid disk or horizon");
    Vec2 p{b.centre.x-a.centre.x,b.centre.y-a.centre.y};
    Vec2 v{b.velocity.x-a.velocity.x,b.velocity.y-a.velocity.y};
    const double R=a.radius+b.radius;
    const double aa=dot(v,v),bb=2*dot(p,v),cc=dot(p,p)-R*R;
    if (!std::isfinite(aa)||!std::isfinite(bb)||!std::isfinite(cc))
        throw std::overflow_error("nonfinite collision coefficients");
    if (cc<0) throw std::domain_error("existing overlap");
    if (cc==0) return 0.0; // Touching, not automatically a new impact.
    if (aa==0||bb>=0) return std::nullopt;
    const double discriminant=std::fma(bb,bb,-4*aa*cc);
    if (!std::isfinite(discriminant))
        throw std::overflow_error("discriminant overflow");
    if (discriminant<0) return std::nullopt;
    const double q=-0.5*(bb-std::sqrt(discriminant));
    if (!(q>0)) return std::nullopt;
    const double t=cc/q;
    if (!std::isfinite(t)||t<0||t>h) return std::nullopt;
    return t;
}
int main() {
    auto near=[](double actual,double expected) {
        if (!std::isfinite(actual)||std::abs(actual-expected)>1e-10)
            throw std::runtime_error("unexpected first-contact time");
    };
    const Disk2 fixed{{0,0},{0,0},1};
    const Disk2 moving{{10,0},{-100,0},1};
    const auto impact=firstTouch(fixed,moving,0.1);
    if (!impact) throw std::runtime_error("fast contact was missed");
    near(*impact,0.08);
    if (firstTouch(fixed,moving,0.05))
        throw std::runtime_error("collision outside horizon was accepted");
    if (firstTouch(fixed,{{10,0},{100,0},1},1.0))
        throw std::runtime_error("separating disks reported impact");
    if (firstTouch(fixed,{{10,0},{0,0},1},1.0))
        throw std::runtime_error("static separated disks reported impact");
    if (firstTouch(fixed,{{10,2.1},{-10,0},1},2.0))
        throw std::runtime_error("geometric near miss reported impact");
    const auto tangent=firstTouch(fixed,{{10,2},{-10,0},1},2.0);
    if (!tangent) throw std::runtime_error("tangency not found");
    near(*tangent,1.0);
    const auto galilean=firstTouch({{0,0},{55,7},1},
                                   {{10,0},{-45,7},1},0.1);
    if (!galilean) throw std::runtime_error("relative motion invariance failed");
    near(*galilean,0.08);
    bool rejected=false;
    try { (void)firstTouch(fixed,{{1,0},{0,0},1},1); }
    catch(const std::domain_error&) { rejected=true; }
    if (!rejected) throw std::runtime_error("initial overlap not rejected");
    rejected=false;
    try { (void)firstTouch(fixed,moving,-1); }
    catch(const std::invalid_argument&) { rejected=true; }
    if (!rejected) throw std::runtime_error("negative time accepted");
    std::cout<<"math first contact: entry, no-hit, tangency, invariance and validation PASS\n";
}
