#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>

namespace {

bool close(double a, double b, double tolerance) {
    return std::abs(a - b) <= tolerance;
}

double trapezoid(double a, double b, int n) {
    assert(n > 0);
    const double h = (b - a) / n;
    double sum = 0.5 * (std::exp(a) + std::exp(b));
    for (int i = 1; i < n; ++i)
        sum += std::exp(a + i * h);
    return h * sum;
}

double simpson(double a, double b, int n) {
    assert(n > 0 && n % 2 == 0);
    const double h = (b - a) / n;
    double sum = std::exp(a) + std::exp(b);
    for (int i = 1; i < n; ++i)
        sum += (i % 2 == 0 ? 2.0 : 4.0) * std::exp(a + i * h);
    return h * sum / 3.0;
}

double centralDerivative(double (*f)(double), double x, double h) {
    return (f(x + h) - f(x - h)) / (2.0 * h);
}

double sine(double x) { return std::sin(x); }

double eulerDecay(double h, int steps) {
    double y = 1.0;
    for (int i = 0; i < steps; ++i)
        y += h * (-2.0 * y);
    return y;
}

double rk4Decay(double h, int steps) {
    double y = 1.0;
    for (int i = 0; i < steps; ++i) {
        const double k1 = -2.0 * y;
        const double k2 = -2.0 * (y + 0.5 * h * k1);
        const double k3 = -2.0 * (y + 0.5 * h * k2);
        const double k4 = -2.0 * (y + h * k3);
        y += h * (k1 + 2.0 * k2 + 2.0 * k3 + k4) / 6.0;
    }
    return y;
}

struct State { double q; double p; };

double energy(State s) { return 0.5 * (s.q * s.q + s.p * s.p); }

State explicitEulerOscillator(State s, double h) {
    return {s.q + h * s.p, s.p - h * s.q};
}

State semiImplicitEulerOscillator(State s, double h) {
    const double qNext = s.q + h * s.p;
    const double pNext = s.p - h * qNext;
    return {qNext, pNext};
}

double logCosh(double x) {
    const double ax = std::abs(x);
    return ax + std::log1p(std::exp(-2.0 * ax)) - std::log(2.0);
}

double firstArmijoNewtonStep(double x, int& backtracks) {
    const double value = logCosh(x);
    const double gradient = std::tanh(x);
    const double hessian = 1.0 / std::pow(std::cosh(x), 2.0);
    assert(std::isfinite(value) && hessian > 0.0);
    const double direction = -gradient / hessian;
    assert(gradient * direction < 0.0);

    // A deliberately conservative valid Armijo constant makes the first
    // accepted dyadic trial easy to inspect in this teaching example.
    constexpr double c1 = 0.5;
    double alpha = 1.0;
    backtracks = 0;
    for (int trial = 0; trial < 64; ++trial) {
        const double candidate = x + alpha * direction;
        if (std::isfinite(candidate)
            && logCosh(candidate) <= value + c1 * alpha * gradient * direction)
            return alpha;
        alpha *= 0.5;
        ++backtracks;
    }
    return 0.0;
}
} // namespace

int main() {
    std::cout << std::setprecision(12);

    // Integrate exp(x) on [0,1]; the exact answer is e - 1.
    constexpr int n = 100;
    const double exactIntegral = std::exp(1.0) - 1.0;
    const double trap = trapezoid(0.0, 1.0, n);
    const double simp = simpson(0.0, 1.0, n);
    assert(std::abs(trap - exactIntegral) < 1.5e-5);
    assert(std::abs(simp - exactIntegral) < 1.0e-9);
    assert(std::abs(simp - exactIntegral) < std::abs(trap - exactIntegral));
    std::cout << "quadrature exact=" << exactIntegral << " trapezoid=" << trap
              << " Simpson=" << simp << '\n';

    // Check a numerical derivative against an independent analytic value.
    constexpr double x = 0.7;
    constexpr double hDiff = 1.0e-4;
    const double fd = centralDerivative(sine, x, hDiff);
    assert(std::abs(fd - std::cos(x)) < 1.0e-8);
    std::cout << "central derivative=" << fd << " exact=" << std::cos(x) << '\n';

    // Solve y'=-2y from y(0)=1 to t=1 with h=0.001.
    constexpr int odeSteps = 1000;
    constexpr double hOde = 1.0 / odeSteps;
    const double exactDecay = std::exp(-2.0);
    const double euler = eulerDecay(hOde, odeSteps);
    const double rk4 = rk4Decay(hOde, odeSteps);
    assert(std::abs(euler - exactDecay) < 3.0e-4);
    assert(std::abs(euler - exactDecay) > 1.0e-5);
    assert(std::abs(rk4 - exactDecay) < 1.0e-12);
    assert(std::abs(rk4 - exactDecay) < std::abs(euler - exactDecay));
    std::cout << "decay exact=" << exactDecay << " Euler=" << euler
              << " RK4=" << rk4 << '\n';

    // Compare energy behavior for q'=p, p'=-q over t=100.
    constexpr double hOsc = 0.1;
    constexpr int oscSteps = 1000;
    State explicitState{1.0, 0.0};
    State semiState{1.0, 0.0};
    double explicitFinalEnergy = energy(explicitState);
    double semiMinEnergy = energy(semiState);
    double semiMaxEnergy = energy(semiState);
    for (int i = 0; i < oscSteps; ++i) {
        explicitState = explicitEulerOscillator(explicitState, hOsc);
        semiState = semiImplicitEulerOscillator(semiState, hOsc);
        explicitFinalEnergy = energy(explicitState);
        semiMinEnergy = std::min(semiMinEnergy, energy(semiState));
        semiMaxEnergy = std::max(semiMaxEnergy, energy(semiState));
    }
    assert(explicitFinalEnergy > 100.0);
    assert(semiMinEnergy > 0.46 && semiMaxEnergy < 0.54);
    assert(semiMaxEnergy > semiMinEnergy);
    std::cout << "oscillator E0=0.5 explicit E(100)=" << explicitFinalEnergy
              << " semi-implicit range=[" << semiMinEnergy << ',' << semiMaxEnergy
              << "] (bounded here, not exact)\n";

    // A safeguarded Newton step for f(x)=log(cosh(x)), starting at x=3.
    int backtracks = 0;
    const double alpha = firstArmijoNewtonStep(3.0, backtracks);
    assert(close(alpha, 1.0 / 32.0, 0.0));
    assert(backtracks == 5);
    const double gradientAtStart = std::tanh(3.0);
    const double hessianAtStart = 1.0 / std::pow(std::cosh(3.0), 2.0);
    const double newtonDirection = -gradientAtStart / hessianAtStart;
    const double nextX = 3.0 + alpha * newtonDirection;
    assert(logCosh(nextX) < logCosh(3.0));
    std::cout << "Newton x0=3 alpha=" << alpha << " backtracks=" << backtracks
              << " x1=" << nextX << " f1=" << logCosh(nextX) << '\n';

    std::cout << "all quantitative assertions passed\n";
}
