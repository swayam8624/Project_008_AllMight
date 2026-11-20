#pragma once
// Book Two: inspectable C++23 numerical experiments. Educational reference,
// not a replacement for LAPACK, Eigen, Ceres, or a production-grade solver.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <vector>
#include <utility>

namespace book02::advanced {
struct LeastSquares {
    std::vector<double> coefficients;
    double rms{};
    // Ratio of smallest to largest |R_ii|: a diagnostic, NOT kappa_2(A)^-1.
    double qr_diagonal_ratio{};
};

// Solve a row-major, overdetermined (m >= n) full-column-rank system by
// Householder QR with column pivoting. A relative rank policy is made explicit.
inline std::optional<LeastSquares> qr_least_squares(
    std::span<const double> input, std::size_t m, std::size_t n,
    std::span<const double> rhs) {
    if (!n || m < n || n > std::numeric_limits<std::size_t>::max()/m ||
        input.size() != m*n || rhs.size() != m) return std::nullopt;
    for (double x : input) if (!std::isfinite(x)) return std::nullopt;
    for (double y : rhs) if (!std::isfinite(y)) return std::nullopt;
    std::vector<double> a(input.begin(),input.end()), b(rhs.begin(),rhs.end());
    std::vector<std::size_t> permutation(n);
    std::iota(permutation.begin(),permutation.end(),0);
    auto column_norm = [&](std::size_t col,std::size_t start) {
        double s=0;
        for (std::size_t i=start;i<m;++i) s=std::hypot(s,a[i*n+col]);
        return s;
    };
    double original_scale=0;
    for (std::size_t j=0;j<n;++j) original_scale=std::max(original_scale,column_norm(j,0));
    if (!(original_scale>0) || !std::isfinite(original_scale)) return std::nullopt;
    const double tolerance=64*std::numeric_limits<double>::epsilon()*
                           static_cast<double>(std::max(m,n))*original_scale;
    double min_diagonal=std::numeric_limits<double>::infinity(),max_diagonal=0;
    for (std::size_t k=0;k<n;++k) {
        std::size_t pivot=k;
        double largest=column_norm(k,k);
        for (std::size_t j=k+1;j<n;++j) {
            const double v=column_norm(j,k);
            if (v>largest) { largest=v; pivot=j; }
        }
        if (!std::isfinite(largest) || largest<=tolerance) return std::nullopt;
        if (pivot!=k) {
            for (std::size_t i=0;i<m;++i) std::swap(a[i*n+k],a[i*n+pivot]);
            std::swap(permutation[k],permutation[pivot]);
        }
        // Choose the opposite sign to avoid cancellation forming v[0].
        const double alpha=std::copysign(largest,-a[k*n+k]);
        std::vector<double> v(m-k);
        v[0]=a[k*n+k]-alpha;
        for (std::size_t i=k+1;i<m;++i) v[i-k]=a[i*n+k];
        double norm_v=0;
        for (double z:v) norm_v=std::hypot(norm_v,z);
        if (!(norm_v>0) || !std::isfinite(norm_v)) return std::nullopt;
        for (double &z:v) z/=norm_v;
        for (std::size_t j=k+1;j<n;++j) {
            double dot=0;
            for (std::size_t i=k;i<m;++i) dot+=v[i-k]*a[i*n+j];
            for (std::size_t i=k;i<m;++i) a[i*n+j]-=2*v[i-k]*dot;
        }
        double dot=0;
        for (std::size_t i=k;i<m;++i) dot+=v[i-k]*b[i];
        for (std::size_t i=k;i<m;++i) b[i]-=2*v[i-k]*dot;
        a[k*n+k]=alpha;
        for (std::size_t i=k+1;i<m;++i) a[i*n+k]=0;
        min_diagonal=std::min(min_diagonal,std::abs(alpha));
        max_diagonal=std::max(max_diagonal,std::abs(alpha));
    }
    std::vector<double> permuted(n,0),answer(n,0);
    for (std::size_t jj=n;jj-->0;) {
        double sum=b[jj];
        for (std::size_t k=jj+1;k<n;++k) sum-=a[jj*n+k]*permuted[k];
        permuted[jj]=sum/a[jj*n+jj];
        if (!std::isfinite(permuted[jj])) return std::nullopt;
    }
    for (std::size_t j=0;j<n;++j) answer[permutation[j]]=permuted[j];
    double squared=0;
    for (std::size_t i=0;i<m;++i) {
        double predicted=0;
        for (std::size_t j=0;j<n;++j) predicted+=input[i*n+j]*answer[j];
        const double error=predicted-rhs[i];
        squared+=error*error;
    }
    const double rms=std::sqrt(squared/static_cast<double>(m));
    if (!std::isfinite(rms)) return std::nullopt;
    return LeastSquares{std::move(answer),rms,min_diagonal/max_diagonal};
}

struct Observation { double x{}, y{}; };
struct Parameters { double a{},b{},c{}; };
struct RobustFit {
    Parameters parameters;
    double objective{};
    double gradient_infinity{};
    std::size_t iterations{}, accepted_steps{};
    bool converged{};
};
inline std::optional<std::array<double,4>> residual_and_jacobian(
    Observation p,Parameters theta) {
    if (!std::isfinite(p.x)||!std::isfinite(p.y)||
        !std::isfinite(theta.a)||!std::isfinite(theta.b)||!std::isfinite(theta.c))
        return std::nullopt;
    const double e=std::exp(theta.b*p.x);
    const std::array<double,4> row{theta.a*e+theta.c-p.y,e,theta.a*p.x*e,1};
    for (double v:row) if (!std::isfinite(v)) return std::nullopt;
    return row; // residual, d/da, d/db, d/dc
}
inline std::optional<double> huber_objective(
    std::span<const Observation> obs,Parameters p,double delta) {
    if (!(delta>0)||!std::isfinite(delta)) return std::nullopt;
    double cost=0;
    for (const auto o:obs) {
        const auto r=residual_and_jacobian(o,p);
        if (!r) return std::nullopt;
        const double e=std::abs((*r)[0]);
        cost+=e<=delta?0.5*e*e:delta*(e-0.5*delta);
    }
    if (!std::isfinite(cost)) return std::nullopt;
    return cost;
}
// Huber IRLS + adaptive Levenberg-style damping. The QR solve operates on
// an AUGMENTED system, not J^T W J, so conditioning is not needlessly squared.
// Acceptance compares true Huber costs; the IRLS quadratic is only a model.
inline std::optional<RobustFit> fit_exponential_huber(
    std::span<const Observation> obs,Parameters initial,double delta,
    std::size_t max_iterations=100) {
    if (obs.size()<4 || max_iterations==0 || !(delta>0) ||
        !std::isfinite(delta)) return std::nullopt;
    Parameters current=initial;
    auto cost=huber_objective(obs,current,delta);
    if (!cost) return std::nullopt;
    double lambda=1e-3,gradient=std::numeric_limits<double>::infinity();
    std::size_t accepted=0,iterations=0;
    bool converged=false;
    for (std::size_t it=0;it<max_iterations;++it) {
        iterations=it+1;
        std::vector<double> a((obs.size()+3)*3,0), b(obs.size()+3,0);
        std::array<double,3> g{};
        for (std::size_t i=0;i<obs.size();++i) {
            const auto row=residual_and_jacobian(obs[i],current);
            if (!row) return std::nullopt;
            const double r=(*row)[0];
            const double weight=std::abs(r)<=delta?1.0:delta/std::abs(r);
            const double sw=std::sqrt(weight);
            for (std::size_t j=0;j<3;++j) {
                a[i*3+j]=sw*(*row)[j+1];
                g[j]+=weight*r*(*row)[j+1];
            }
            b[i]=-sw*r;
        }
        gradient=std::max({std::abs(g[0]),std::abs(g[1]),std::abs(g[2])});
        if (!std::isfinite(gradient)) return std::nullopt;
        if (gradient<1e-9) { converged=true; break; }
        const double damping=std::sqrt(lambda);
        for (std::size_t j=0;j<3;++j) a[(obs.size()+j)*3+j]=damping;
        const auto step=qr_least_squares(a,obs.size()+3,3,b);
        if (!step) return std::nullopt;
        const auto &d=step->coefficients;
        const Parameters candidate{current.a+d[0],current.b+d[1],current.c+d[2]};
        const auto proposed=huber_objective(obs,candidate,delta);
        if (proposed && *proposed<*cost) {
            const double change=*cost-*proposed;
            current=candidate; cost=proposed; ++accepted;
            lambda=std::max(lambda*0.35,1e-12);
            const double maxchange=std::max({std::abs(d[0]),std::abs(d[1]),std::abs(d[2])});
            if (change<=1e-12*(1+*cost) && maxchange<1e-7) {
                converged=true; break;
            }
        } else {
            lambda=std::min(lambda*8,1e12);
            if (lambda>=1e12) break;
        }
    }
    // Report the gradient at the RETURNED parameters, not an earlier trial.
    std::array<double,3> final_g{};
    for (const auto o:obs) {
        const auto row=residual_and_jacobian(o,current);
        if (!row) return std::nullopt;
        const double r=(*row)[0];
        const double psi=std::clamp(r,-delta,delta);
        for (std::size_t j=0;j<3;++j) final_g[j]+=psi*(*row)[j+1];
    }
    gradient=std::max({std::abs(final_g[0]),std::abs(final_g[1]),std::abs(final_g[2])});
    if (!std::isfinite(gradient)) return std::nullopt;
    return RobustFit{current,*cost,gradient,iterations,accepted,converged};
}
} // namespace book02::advanced
