// First-order contact penetration refresh; planar correction and ordering witness.
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct Vec2 {
    double x;
    double y;
};

Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }

double dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

// Signed delta is positive counter-clockwise. This is the first-order point
// displacement theta x r in 2D; large rotations require refreshed geometry.
Vec2 small_rotation_displacement(double delta_angle, Vec2 r) {
    return {-delta_angle * r.y, delta_angle * r.x};
}

struct BodyCorrection {
    Vec2 translation;
    double delta_angle;
};

struct Contact {
    std::string name;
    const BodyCorrection* body_a;
    const BodyCorrection* body_b;
    Vec2 normal_a_to_b;
    Vec2 r_a;
    Vec2 r_b;
    double penetration;
};

Vec2 point_delta(const BodyCorrection& body, Vec2 r) {
    return body.translation + small_rotation_displacement(body.delta_angle, r);
}

// Positive penetration means overlap. With n from A to B, d = -n·(pB-pA),
// so the first-order depth change is n·(delta_pA-delta_pB).
void refresh_penetration(Contact& contact) {
    const Vec2 delta_a = point_delta(*contact.body_a, contact.r_a);
    const Vec2 delta_b = point_delta(*contact.body_b, contact.r_b);
    contact.penetration += dot(contact.normal_a_to_b, delta_a - delta_b);
}

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(double actual, double expected, double tolerance = 1.0e-12) {
    return std::fabs(actual - expected) <= tolerance;
}

std::size_t deepest_index(const std::array<Contact, 3>& contacts) {
    std::size_t result = 0;
    for (std::size_t i = 1; i < contacts.size(); ++i) {
        if (contacts[i].penetration > contacts[result].penetration) {
            result = i;
        }
    }
    return result;
}

void three_contact_trace() {
    constexpr Vec2 up{0.0, 1.0};
    const BodyCorrection fixed_floor{{0.0, 0.0}, 0.0};
    const BodyCorrection fixed_wall{{0.0, 0.0}, 0.0};

    // A is the floor, B is the crate. Contact points are +/-1 m from the
    // crate's correction origin. The unrelated pair does not include the crate.
    BodyCorrection crate_correction{{0.0, 0.0025}, -0.0075};
    std::array<Contact, 3> contacts{{
        {"left", &fixed_floor, &crate_correction, up, {0.0, 0.0}, {-1.0, 0.0}, 0.012},
        {"unrelated", &fixed_floor, &fixed_wall, up, {0.0, 0.0}, {0.0, 0.0}, 0.010},
        {"right", &fixed_floor, &crate_correction, up, {0.0, 0.0}, {1.0, 0.0}, 0.008},
    }};

    require(deepest_index(contacts) == 0, "initial deepest contact should be left");

    // Refresh every existing contact after applying the crate's one correction.
    // The selected feature can change depth too; the unrelated pair has no moved
    // body and therefore remains unchanged.
    for (Contact& contact : contacts) {
        refresh_penetration(contact);
    }

    require(near(contacts[0].penetration, 0.002), "left contact should fall to 2 mm");
    require(near(contacts[1].penetration, 0.010), "unrelated contact should stay at 10 mm");
    require(near(contacts[2].penetration, 0.013), "right contact should rise to 13 mm");
    require(deepest_index(contacts) == 2, "right contact should become the next worst");

    std::cout << "three-contact refresh: ";
    for (const Contact& contact : contacts) {
        std::cout << contact.name << '=' << contact.penetration * 1000.0 << " mm ";
    }
    std::cout << "next=" << contacts[deepest_index(contacts)].name << '\n';
}

void both_body_sign_case() {
    constexpr Vec2 right{1.0, 0.0};
    const BodyCorrection a{{0.002, 0.0}, 0.0};
    const BodyCorrection b{{-0.003, 0.0}, 0.0};
    Contact contact{"both-bodies", &a, &b, right, {0.0, 0.0}, {0.0, 0.0}, 0.004};

    refresh_penetration(contact);
    // A moves 2 mm toward B; B moves 3 mm toward A. The gap closes by 5 mm.
    require(near(contact.penetration, 0.009), "both-body sign should add 5 mm penetration");
    std::cout << "both-body sign: " << contact.penetration * 1000.0 << " mm\n";
}

void unrelated_contact_and_tangent_case() {
    constexpr Vec2 up{0.0, 1.0};
    const BodyCorrection still_a{{0.0, 0.0}, 0.0};
    const BodyCorrection still_b{{0.0, 0.0}, 0.0};
    Contact unrelated{"unrelated", &still_a, &still_b, up, {0.0, 0.0}, {0.0, 0.0}, 0.010};
    refresh_penetration(unrelated);
    require(near(unrelated.penetration, 0.010), "unrelated contact must not change");

    // A 3 mm horizontal shift has zero projection on the vertical normal.
    const BodyCorrection tangent_body{{0.003, 0.0}, 0.0};
    Contact tangent{"tangent-only", &still_a, &tangent_body, up,
                    {0.0, 0.0}, {0.0, 0.0}, 0.006};
    refresh_penetration(tangent);
    require(near(tangent.penetration, 0.006), "tangent shift must not change depth");
    std::cout << "unrelated/tangent: 10 mm and 6 mm unchanged\n";
}

} // namespace

int main() {
    try {
        three_contact_trace();
        both_body_sign_case();
        unrelated_contact_and_tangent_case();
        std::cout << "all contact-refresh checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << "contact-refresh witness failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
