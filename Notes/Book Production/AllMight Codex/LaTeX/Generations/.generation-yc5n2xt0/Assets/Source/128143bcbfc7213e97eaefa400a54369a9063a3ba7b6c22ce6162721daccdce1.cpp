// Rigid-body contact chapter: central, frictionless 1D impulse witness.
// Standalone C++20: c++ -std=c++20 -O2 -Wall -Wextra -pedantic
//   tools/labs/rigid_body_contact_lab.cpp -o /tmp/rigid_body_contact_lab
//   && /tmp/rigid_body_contact_lab
// This intentionally does not implement 3D inertia or collision detection.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

struct Body {
    double mass;
    double velocity;
};

void require(bool ok, const char* what) {
    if (!ok) throw std::runtime_error(what);
}
bool close(double a, double b, double eps = 1e-12) {
    return std::abs(a-b) <= eps;
}

// Normal points from A to B. A negative relative velocity is approaching.
// Central contacts have zero lever arms; effective inverse mass reduces to
// inverse_mass_A + inverse_mass_B.
double collide(Body& a, Body& b, double restitution) {
    require(a.mass > 0 && b.mass > 0, "finite positive masses required");
    require(restitution >= 0 && restitution <= 1, "invalid restitution");
    const double vn = b.velocity - a.velocity;
    if (vn >= 0) return 0.0; // no attractive impulse for separating bodies
    const double k = 1.0/a.mass + 1.0/b.mass;
    const double j = -(1.0+restitution)*vn/k;
    a.velocity -= j/a.mass;
    b.velocity += j/b.mass;
    return j;
}

// An iterative solver clamps the total accumulated impulse, not the
// requested increment. The returned delta is what must be applied.
double projected_increment(double& lambda, double requested) {
    require(lambda >= 0, "invalid cached normal impulse");
    const double next = std::max(0.0, lambda+requested);
    const double applied = next-lambda;
    lambda = next;
    return applied;
}

int main() {
    try {
        Body a{2.0, 3.0}, b{2.0, -1.0};
        const double before_momentum = a.mass*a.velocity+b.mass*b.velocity;
        const double j = collide(a,b,0.5);
        require(close(j,6.0), "wrong impulse");
        require(close(a.velocity,0.0) && close(b.velocity,2.0),
                "wrong post-collision velocities");
        require(close(before_momentum,
                      a.mass*a.velocity+b.mass*b.velocity),
                "momentum not conserved");
        require(close(b.velocity-a.velocity,2.0),
                "restitution does not match relative separation speed");

        // Separating bodies must not be pulled together.
        Body c{2.0,-1.0}, d{2.0,1.0};
        require(close(collide(c,d,0.5),0.0), "attractive collision impulse");
        require(close(c.velocity,-1.0) && close(d.velocity,1.0),
                "separating state unexpectedly changed");

        // Solver must be able to back out an excessive warm-start impulse.
        double lambda=4.0;
        require(close(projected_increment(lambda,-1.5),-1.5),
                "failed to remove excess accumulated impulse");
        require(close(lambda,2.5), "incorrect new accumulated impulse");
        require(close(projected_increment(lambda,-8.0),-2.5),
                "clamp should apply only the remaining negative delta");
        require(close(lambda,0.0), "normal impulse became negative");
        require(close(projected_increment(lambda,-1.0),0.0),
                "normal impulse should not become attractive");

        std::cout << "PASS: central collision, momentum, restitution, "
                     "separation, accumulated impulse clamp\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
