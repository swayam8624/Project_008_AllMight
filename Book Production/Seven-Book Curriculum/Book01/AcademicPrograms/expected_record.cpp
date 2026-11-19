// std::expected is C++23; std::span first appeared in C++20.
// The zero result is valid, distinct from a rejected version or length.
#include <cstdint>
#include <expected>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>

enum class DecodeError { too_short, unsupported_version };

std::expected<std::uint8_t, DecodeError>
decode(std::span<const std::uint8_t> bytes) {
    if (bytes.size() < 2)
        return std::unexpected(DecodeError::too_short);
    if (bytes[0] != 1)
        return std::unexpected(DecodeError::unsupported_version);
    return bytes[1];
}

int main() {
    const std::vector<std::uint8_t> valid{1, 0};
    const std::vector<std::uint8_t> bad_version{2, 5};
    const std::vector<std::uint8_t> short_record{1};
    const auto first = decode(valid);
    const auto second = decode(bad_version);
    const auto third = decode(short_record);
    if (!first || *first != 0 || second || third)
        throw std::runtime_error("expected-value contract broken");
    if (second.error() != DecodeError::unsupported_version ||
        third.error() != DecodeError::too_short)
        throw std::runtime_error("incorrect error classification");
    std::cout << "Expected: valid zero and two error classes distinguished\n";
}
