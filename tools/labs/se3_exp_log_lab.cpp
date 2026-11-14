#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>

struct Vec3 {
    // Rotation vectors use radians; rho and t use the same length unit.
    double x{}, y{}, z{};
};

Vec3 operator+(Vec3 a, Vec3 b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
Vec3 operator*(double s, Vec3 v) { return {s*v.x, s*v.y, s*v.z}; }
double dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
double norm(Vec3 v) { return std::sqrt(dot(v,v)); }

struct Mat3 {
    // Row-major storage; multiplication consistently treats vectors as columns.
    std::array<std::array<double,3>,3> a{};
};

Mat3 identity() {
    // Zero initialization supplies off-diagonal zeros; set the identity diagonal.
    Mat3 M{};
    for (int i=0;i<3;++i) M.a[i][i]=1.0;
    return M;
}
Mat3 operator+(Mat3 A, Mat3 B) {
    Mat3 C{};
    for(int r=0;r<3;++r) for(int c=0;c<3;++c) C.a[r][c]=A.a[r][c]+B.a[r][c];
    return C;
}
Mat3 operator-(Mat3 A, Mat3 B) {
    Mat3 C{};
    for(int r=0;r<3;++r) for(int c=0;c<3;++c) C.a[r][c]=A.a[r][c]-B.a[r][c];
    return C;
}
Mat3 operator*(double s, Mat3 A) {
    for(auto& row:A.a) for(double& v:row) v*=s;
    return A;
}
Mat3 operator*(Mat3 A, Mat3 B) {
    // Standard row-by-column composition; k sums the shared dimension.
    Mat3 C{};
    for(int r=0;r<3;++r)
        for(int c=0;c<3;++c)
            for(int k=0;k<3;++k)
                C.a[r][c]+=A.a[r][k]*B.a[k][c];
    return C;
}
Vec3 operator*(Mat3 A, Vec3 v) {
    return {
        A.a[0][0]*v.x + A.a[0][1]*v.y + A.a[0][2]*v.z,
        A.a[1][0]*v.x + A.a[1][1]*v.y + A.a[1][2]*v.z,
        A.a[2][0]*v.x + A.a[2][1]*v.y + A.a[2][2]*v.z
    };
}
Mat3 transpose(Mat3 A) {
    Mat3 T{};
    for(int r=0;r<3;++r) for(int c=0;c<3;++c) T.a[r][c]=A.a[c][r];
    return T;
}
double trace(Mat3 A) { return A.a[0][0]+A.a[1][1]+A.a[2][2]; }

Mat3 hat(Vec3 w) {
    // hat(w) is skew-symmetric and maps v to the cross product w cross v.
    Mat3 W{};
    W.a[0][1] = -w.z;
    W.a[0][2] =  w.y;
    W.a[1][0] =  w.z;
    W.a[1][2] = -w.x;
    W.a[2][0] = -w.y;
    W.a[2][1] =  w.x;
    return W;
}
Vec3 vee(Mat3 W) {
    // Read the three independent skew entries, inverse to hat() on skew matrices.
    return {W.a[2][1], W.a[0][2], W.a[1][0]};
}

Mat3 so3_exp(Vec3 w) {
    // Rodrigues' formula evaluates exp(hat(w)); theta is measured in radians.
    const double theta=norm(w);
    const Mat3 W=hat(w);
    const Mat3 W2=W*W;
    if(theta < 1e-8) {
        // The direct fractions cancel near zero; use their limiting series.
        return identity() + W + 0.5*W2;
    }
    const double A=std::sin(theta)/theta;
    const double B=(1.0-std::cos(theta))/(theta*theta);
    return identity() + A*W + B*W2;
}

Vec3 so3_log(Mat3 R) {
    // Precondition: R is a valid proper rotation. This compact witness does
    // not project or repair malformed matrices before applying the log.
    // Trace yields cos(theta); clamp roundoff into acos's valid domain [-1,1].
    const double cos_theta=std::clamp((trace(R)-1.0)*0.5,-1.0,1.0);
    const double theta=std::acos(cos_theta);
    if(theta < 1e-8) {
        // The antisymmetric part is 2*hat(w) to first order near identity.
        return vee(0.5*(R-transpose(R)));
    }
    // The principal logarithm has a delicate axis branch near pi; reject that
    // region rather than divide by a nearly zero sin(theta) and fake support.
    if (std::abs(std::numbers::pi-theta) <= 1e-5)
        throw std::domain_error("compact SO(3) log witness does not support theta near pi");
    return vee((theta/(2.0*std::sin(theta)))*(R-transpose(R)));
}

Mat3 left_jacobian_so3(Vec3 w) {
    // V maps tangent translation rho to the finite transform translation t.
    const double theta=norm(w);
    const Mat3 W=hat(w);
    const Mat3 W2=W*W;
    if(theta < 1e-8) {
        // Taylor limit avoids subtracting nearly equal trigonometric values.
        return identity() + 0.5*W + (1.0/6.0)*W2;
    }
    const double t2=theta*theta, t3=t2*theta;
    return identity()
         + ((1.0-std::cos(theta))/t2)*W
         + ((theta-std::sin(theta))/t3)*W2;
}

Mat3 left_jacobian_so3_inverse(Vec3 w) {
    // Invert V to recover rho after the logarithm has recovered omega.
    const double theta=norm(w);
    const Mat3 W=hat(w);
    const Mat3 W2=W*W;
    if(theta < 1e-8) {
        // Use the inverse series because the closed-form coefficient is fragile.
        return identity() - 0.5*W + (1.0/12.0)*W2;
    }
    const double s=std::sin(theta), c=std::cos(theta);
    const double coeff=1.0/(theta*theta) - (1.0+c)/(2.0*theta*s);
    return identity() - 0.5*W + coeff*W2;
}

struct SE3 {
    // This transform maps a point p to R*p + t.
    Mat3 R;
    Vec3 t;
};

SE3 se3_exp(Vec3 omega, Vec3 rho) {
    // The group exponential couples translation to rotation through V(omega).
    const Mat3 R=so3_exp(omega);
    const Mat3 V=left_jacobian_so3(omega);
    return {R, V*rho};
}

std::pair<Vec3,Vec3> se3_log(SE3 T) {
    // Assume T contains a valid SE(3) rotation. Rotation is recovered first
    // because its Jacobian determines translation rho; near-pi input throws.
    const Vec3 omega=so3_log(T.R);
    const Mat3 Vinv=left_jacobian_so3_inverse(omega);
    return {omega, Vinv*T.t};
}

bool near(double a,double b,double eps=1e-9){ return std::abs(a-b)<=eps; }
bool near(Vec3 a,Vec3 b,double eps=1e-9){
    return near(a.x,b.x,eps)&&near(a.y,b.y,eps)&&near(a.z,b.z,eps);
}
bool near(Mat3 A,Mat3 B,double eps=1e-9){
    for(int r=0;r<3;++r) for(int c=0;c<3;++c)
        if(!near(A.a[r][c],B.a[r][c],eps)) return false;
    return true;
}

void require(bool condition, const char* message) {
    // Unlike assert, these checks remain active in optimized NDEBUG builds.
    if (!condition) throw std::runtime_error(message);
}

int main() {
    {
        // Ordinary-angle round trip checks both the recovered twist and SO(3) shape.
        const Vec3 w{0.2,-0.3,0.1};
        const Mat3 R=so3_exp(w);
        require(near(transpose(R)*R, identity(), 1e-10), "R must remain orthogonal");
        require(near(so3_log(R), w, 1e-10), "ordinary SO(3) exp/log round trip");
    }
    {
        // Select the small-angle Taylor branch explicitly.
        const Vec3 w{1e-10,-2e-10,3e-10};
        const Mat3 R=so3_exp(w);
        require(near(so3_log(R), w, 1e-9), "small-angle SO(3) exp/log round trip");
    }
    {
        // Both angular and translational tangent coordinates round-trip.
        const Vec3 omega{0.25,-0.15,0.05};
        const Vec3 rho{1.2,-0.4,0.7};
        const SE3 T=se3_exp(omega,rho);
        const auto [omega2,rho2]=se3_log(T);
        require(near(omega2,omega,1e-10), "SE(3) rotational coordinate round trip");
        require(near(rho2,rho,1e-10), "SE(3) translational coordinate round trip");
    }
    {
        // A nonzero omega makes t differ from rho, exposing the coupling.
        // Translation and rotation are coupled by V(omega): t is not generally rho.
        const Vec3 omega{0.0,0.0,0.6};
        const Vec3 rho{1.0,0.0,0.0};
        const SE3 T=se3_exp(omega,rho);
        require(!near(T.t,rho,1e-6), "finite rigid motion must expose V(omega) translation coupling");
        const auto [w2,r2]=se3_log(T);
        require(near(w2,omega,1e-10), "coupled example rotational round trip");
        require(near(r2,rho,1e-10), "coupled example translational round trip");
    }
    {
        // The compact logarithm promises to reject its unsupported near-pi
        // branch before its sin(theta) denominator becomes numerically unsafe.
        const Vec3 near_pi{std::numbers::pi - 5e-6, 0.0, 0.0};
        bool rejected = false;
        try {
            (void)so3_log(so3_exp(near_pi));
        } catch (const std::domain_error&) {
            rejected = true;
        }
        require(rejected, "near-pi logarithm must take its documented rejection branch");
    }
    std::cout << "se3 exp/log: 5 suites PASS\n";
}
