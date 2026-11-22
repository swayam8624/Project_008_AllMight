#pragma once

// From Geometry to Images — Chapter 04: 2D affine transform reference.
//
// Algebra: column vectors. The rightmost transform acts first: (F * G)(p)
// is F(G(p)). The in-memory representation below is six named coefficients;
// "row-major storage" is never used to infer the algebraic convention.
//
// [m00 m01 tx]   [x]   [m00*x + m01*y + tx]
// [m10 m11 ty] * [y] = [m10*x + m11*y + ty]
// [  0   0  1]   [1]   [          1          ]
//
// Points use homogeneous w=1; displacement vectors use w=0. A normal is a
// *covector*, transformed by the inverse transpose of the linear part,
// not the same matrix as an edge vector under a nonuniform scale/shear.
//
// This classroom implementation rejects nonfinite parameters and severely
// ill-conditioned inverses according to a documented *conservative policy*;
// it is not a proven backward-stable matrix library or arbitrary precision
// numerical-geometry engine. Units are chosen by calling code; the types
// distinguish points/vectors but do not encode frame identities in the type.

#include "book03/frame_math.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace book03 {

struct Affine2 {
    // Row/column subscripts describe the mathematical matrix, not the
    // physical memory order. The last row is always [0 0 1].
    double m00{1.0};
    double m01{0.0};
    double m10{0.0};
    double m11{1.0};
    double tx{0.0};
    double ty{0.0};

    [[nodiscard]] static constexpr Affine2 identity() noexcept { return {}; }

    [[nodiscard]] static Affine2 translation(double x, double y) {
        Affine2 result{1, 0, 0, 1, x, y};
        result.validate();
        return result;
    }

    [[nodiscard]] static Affine2 scale(double x, double y) {
        // Singular scales are allowed for forward drawing: they just
        // collapse some dimension. inverse() will reject them.
        Affine2 result{x, 0, 0, y, 0, 0};
        result.validate();
        return result;
    }

    [[nodiscard]] static Affine2 rotation_degrees(double degrees) {
        const double theta = degrees_to_radians(degrees);
        const double c = std::cos(theta);
        const double s = std::sin(theta);
        Affine2 result{c, -s, s, c, 0, 0};
        result.validate();
        return result;
    }

    [[nodiscard]] static Affine2 shear_x(double k) {
        Affine2 result{1, k, 0, 1, 0, 0};
        result.validate();
        return result;
    }

    [[nodiscard]] static Affine2 shear_y(double k) {
        Affine2 result{1, 0, k, 1, 0, 0};
        result.validate();
        return result;
    }

    [[nodiscard]] static Affine2 reflect_about_world_y() {
        return {-1, 0, 0, 1, 0, 0};
    }

    // A Frame2's origin/basis are already in world coordinates. This
    // representation converts between mathematically equivalent APIs.
    [[nodiscard]] static Affine2 from_frame(const Frame2& frame) {
        frame.validate();
        Affine2 result{frame.axis_x.x, frame.axis_y.x,
                       frame.axis_x.y, frame.axis_y.y,
                       frame.origin.x, frame.origin.y};
        result.validate();
        return result;
    }

    [[nodiscard]] Frame2 as_frame() const {
        validate();
        Frame2 result{{tx,ty},{m00,m10},{m01,m11}};
        // Frame2 also enforces its own conservative inverse policy.
        result.validate();
        return result;
    }

    // Compact, named coefficients printed as an ordinary homogeneous
    // 3x3 matrix. This is *row-major array storage*, distinct from the
    // column-vector algebraic rule used by transform_point().
    [[nodiscard]] std::array<double,9> row_major_3x3() const {
        validate();
        return {m00,m01,tx, m10,m11,ty, 0,0,1};
    }

    void validate() const {
        if (!std::isfinite(m00) || !std::isfinite(m01) ||
            !std::isfinite(m10) || !std::isfinite(m11) ||
            !std::isfinite(tx) || !std::isfinite(ty)) {
            throw std::invalid_argument("nonfinite affine matrix component");
        }
    }

    [[nodiscard]] double determinant() const {
        validate();
        const double d = m00*m11 - m01*m10;
        if (!std::isfinite(d)) {
            throw std::overflow_error("affine determinant is nonfinite");
        }
        return d;
    }

    [[nodiscard]] Point2 transform_point(Point2 point) const {
        validate();
        if (!finite(point)) {
            throw std::invalid_argument("affine input point nonfinite");
        }
        const Point2 out{
            m00*point.x + m01*point.y + tx,
            m10*point.x + m11*point.y + ty};
        if (!finite(out)) {
            throw std::overflow_error("affine point result nonfinite");
        }
        return out;
    }

    [[nodiscard]] Vector2 transform_vector(Vector2 v) const {
        validate();
        if (!finite(v)) {
            throw std::invalid_argument("affine input displacement nonfinite");
        }
        const Vector2 out{
            m00*v.x + m01*v.y,
            m10*v.x + m11*v.y};
        if (!finite(out)) {
            throw std::overflow_error("affine vector result nonfinite");
        }
        return out;
    }

    // Composite transform: left applies *after* right, i.e. F(G(point)).
    [[nodiscard]] friend Affine2 operator*(const Affine2& left,
                                            const Affine2& right) {
        left.validate();
        right.validate();
        Affine2 composed{
            left.m00*right.m00 + left.m01*right.m10,
            left.m00*right.m01 + left.m01*right.m11,
            left.m10*right.m00 + left.m11*right.m10,
            left.m10*right.m01 + left.m11*right.m11,
            left.m00*right.tx + left.m01*right.ty + left.tx,
            left.m10*right.tx + left.m11*right.ty + left.ty};
        if (!std::isfinite(composed.m00) || !std::isfinite(composed.m01) ||
            !std::isfinite(composed.m10) || !std::isfinite(composed.m11) ||
            !std::isfinite(composed.tx) || !std::isfinite(composed.ty)) {
            throw std::overflow_error("affine composition overflow");
        }
        return composed;
    }

    // A rotation (or any nonsingular affine map) about a chosen pivot.
    // The pivot remains fixed in this *same* local/world coordinate frame.
    [[nodiscard]] static Affine2 around_pivot(const Affine2& linear,
                                               Point2 pivot) {
        linear.validate();
        if (!finite(pivot) || linear.tx != 0.0 || linear.ty != 0.0) {
            throw std::invalid_argument(
                "pivot requires finite point and translation-free linear map");
        }
        return translation(pivot.x,pivot.y) *
               linear *
               translation(-pivot.x,-pivot.y);
    }

    // Classroom conditioning policy:
    //   max_entry = max(abs(m00), abs(m01), abs(m10), abs(m11))
    //   accept inverse only when abs(det)/max_entry^2 > 1e-10.
    // This criterion is scale-independent for uniform rescaling of A;
    // it rejects *some* severely ill-conditioned maps, not all. It does
    // not prove accuracy for arbitrary inputs. Zero/underflowing scale
    // and nonfinite intermediate results are rejected explicitly.
    [[nodiscard]] Affine2 inverse() const {
        validate();
        const double norm = std::max({
            std::abs(m00),std::abs(m01),std::abs(m10),std::abs(m11)});
        const double d = determinant();
        const double square = norm*norm;
        if (!(square > 0.0) || !std::isfinite(square) ||
            std::abs(d)/square <= 1e-10) {
            throw std::invalid_argument(
                "singular or ill-conditioned affine inverse");
        }

        const double reciprocal = 1.0 / d;
        Affine2 out{
             m11 * reciprocal, -m01 * reciprocal,
            -m10 * reciprocal,  m00 * reciprocal,
            (m01*ty - m11*tx) * reciprocal,
            (m10*tx - m00*ty) * reciprocal};

        if (!std::isfinite(out.m00) || !std::isfinite(out.m01) ||
            !std::isfinite(out.m10) || !std::isfinite(out.m11) ||
            !std::isfinite(out.tx) || !std::isfinite(out.ty)) {
            throw std::overflow_error("affine inverse overflow");
        }
        return out;
    }

    // The unnormalized normal transforms by A^(-T), not by A. A user
    // can normalize afterward; a zero-length input is rejected because
    // an absent direction cannot define perpendicularity.
    [[nodiscard]] Vector2 transform_normal(Vector2 n) const {
        if (!finite(n) || (n.x==0.0 && n.y==0.0)) {
            throw std::invalid_argument("normal must be finite and nonzero");
        }
        const Affine2 inverted = inverse();
        const Vector2 out{
            inverted.m00*n.x + inverted.m10*n.y,
            inverted.m01*n.x + inverted.m11*n.y};
        if (!finite(out)) {
            throw std::overflow_error("normal transform overflow");
        }
        return out;
    }
};

} // namespace book03
