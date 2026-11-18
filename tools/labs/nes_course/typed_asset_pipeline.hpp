#pragma once
// AllMight / Book 1 / Pass 05 — C++23 typed asset import and publication.
//
// Staging decodes untrusted bytes into a fully owned domain asset.
// Publication appends a completed movable value into a mixed-type store.
// The NES cartridge type is the ACTUAL shared NES-01/02 type, not a copy.
//
// This is a CPU-only, single-threaded, append-only teaching store. Typed IDs
// are NOT generational and are only meaningful for the store that issued them.
// References returned by find() must not survive a later insertion that may
// reallocate the underlying std::vector. Chapter 06 will solve that problem.

#include "nes_bus_nrom.hpp"
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <new>
#include <span>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace allmight::nes::assets {

using ByteView = std::span<const Byte>;

struct CartridgeTag {};
struct TextureTag {};
struct MeshTag {};

// Distinct runtime objects preserve different meanings even though all three
// arrive through the same external "bytes" argument.
struct TextureRgba8 {
    std::uint16_t width{};
    std::uint16_t height{};
    std::vector<Byte> pixels;
};

struct QuantizedVertex {
    std::int16_t x{};
    std::int16_t y{};
    std::int16_t z{};
};
struct TriangleMesh {
    std::vector<QuantizedVertex> vertices;
    std::vector<std::uint16_t> indices;
};

// A source-contract error is not the same as an allocation or store failure.
enum class DecodeError {
    short_header, wrong_magic, unsupported_version_or_flags,
    zero_dimension, too_large, bad_length, invalid_triangle_count,
    invalid_index, unsupported_mapper, unsupported_nrom_geometry,
    invalid_cartridge
};
enum class PublishError {
    rejected_source, capacity_exhausted, injected_failure, allocation_failure
};
struct Failure {
    PublishError kind{};
    DecodeError source{};  // relevant when kind == rejected_source
};
enum class TestFault { none, before_stage, after_stage, throw_bad_alloc_after_stage };

template<class Tag> struct Spec; // no fallback: an unknown asset has no import protocol

template<> struct Spec<CartridgeTag> {
    using asset_type = NromCartridge;
    static std::expected<asset_type, DecodeError> decode(ByteView bytes) {
        auto loaded = NromCartridge::from_image(bytes);
        if (loaded) return std::move(*loaded);
        switch (loaded.error()) {
            case CartridgeError::invalid_image:
                return std::unexpected(DecodeError::invalid_cartridge);
            case CartridgeError::unsupported_mapper:
                return std::unexpected(DecodeError::unsupported_mapper);
            case CartridgeError::unsupported_prg_banks:
            case CartridgeError::unsupported_chr_banks:
                return std::unexpected(DecodeError::unsupported_nrom_geometry);
        }
        return std::unexpected(DecodeError::invalid_cartridge);
    }
};

// Nonmember helpers are small, pure and shared by the texture/mesh codecs.
constexpr std::uint16_t u16le(ByteView bytes, std::size_t offset) noexcept {
    // PRECONDITION: two bytes at offset have already been validated.
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[offset]) |
        (static_cast<std::uint16_t>(bytes[offset + 1]) << 8U));
}
constexpr bool multiply(std::size_t a, std::size_t b, std::size_t& result) noexcept {
    if (a != 0 && b > std::numeric_limits<std::size_t>::max() / a) return false;
    result = a * b;
    return true;
}
constexpr bool add(std::size_t a, std::size_t b, std::size_t& result) noexcept {
    if (b > std::numeric_limits<std::size_t>::max() - a) return false;
    result = a + b;
    return true;
}
inline bool signature(ByteView bytes, char a, char b, char c, char d) noexcept {
    return bytes.size() >= 4
        && bytes[0] == static_cast<Byte>(a)
        && bytes[1] == static_cast<Byte>(b)
        && bytes[2] == static_cast<Byte>(c)
        && bytes[3] == static_cast<Byte>(d);
}

// Educational "TXR4" format: magic[4], width_u16LE, height_u16LE,
// four reserved zero bytes, then width*height*4 tightly packed RGBA8.
// It is NOT PNG, KTX, Vulkan/Metal texture storage, or a device upload.
template<> struct Spec<TextureTag> {
    using asset_type = TextureRgba8;
    static std::expected<asset_type, DecodeError> decode(ByteView bytes) {
        constexpr std::size_t header = 12;
        constexpr std::size_t max_payload = 1024 * 1024;
        if (bytes.size() < header) return std::unexpected(DecodeError::short_header);
        if (!signature(bytes, 'T','X','R','4'))
            return std::unexpected(DecodeError::wrong_magic);
        for (std::size_t i = 8; i < header; ++i)
            if (bytes[i] != 0)
                return std::unexpected(DecodeError::unsupported_version_or_flags);
        const auto width = u16le(bytes, 4);
        const auto height = u16le(bytes, 6);
        if (width == 0 || height == 0)
            return std::unexpected(DecodeError::zero_dimension);
        std::size_t pixels{};
        std::size_t byte_count{};
        if (!multiply(width, height, pixels) ||
            !multiply(pixels, std::size_t{4}, byte_count) ||
            byte_count > max_payload)
            return std::unexpected(DecodeError::too_large);
        if (bytes.size() - header != byte_count)
            return std::unexpected(DecodeError::bad_length);
        return asset_type{width, height,
                          std::vector<Byte>(bytes.begin() + header, bytes.end())};
    }
};

// Educational "MSH1" format: magic[4], vertex_count_u16LE,
// index_count_u16LE, four reserved zero bytes, then vertices as
// 3x signed-16-bit LE fixed-point positions (1/256 world unit),
// then triangle indices as unsigned-16-bit LE.
// It is NOT OBJ, glTF, an editable mesh, or a GPU vertex layout.
template<> struct Spec<MeshTag> {
    using asset_type = TriangleMesh;
    static std::expected<asset_type, DecodeError> decode(ByteView bytes) {
        constexpr std::size_t header = 12;
        constexpr std::size_t max_vertices = 8192;
        constexpr std::size_t max_indices = 24576;
        if (bytes.size() < header) return std::unexpected(DecodeError::short_header);
        if (!signature(bytes, 'M','S','H','1'))
            return std::unexpected(DecodeError::wrong_magic);
        for (std::size_t i = 8; i < header; ++i)
            if (bytes[i] != 0)
                return std::unexpected(DecodeError::unsupported_version_or_flags);
        const std::size_t vertices = u16le(bytes, 4);
        const std::size_t indices = u16le(bytes, 6);
        if (vertices < 3 || indices == 0 || indices % 3 != 0)
            return std::unexpected(DecodeError::invalid_triangle_count);
        if (vertices > max_vertices || indices > max_indices)
            return std::unexpected(DecodeError::too_large);

        std::size_t vertex_bytes{};
        std::size_t index_bytes{};
        std::size_t payload_bytes{};
        if (!multiply(vertices, 6, vertex_bytes) ||
            !multiply(indices, 2, index_bytes) ||
            !add(vertex_bytes, index_bytes, payload_bytes))
            return std::unexpected(DecodeError::too_large);
        if (bytes.size() - header != payload_bytes)
            return std::unexpected(DecodeError::bad_length);

        asset_type output;
        output.vertices.reserve(vertices);
        output.indices.reserve(indices);
        for (std::size_t i = 0; i < vertices; ++i) {
            const std::size_t at = header + i * 6;
            // bit_cast preserves all 16 representation bits without
            // converting an out-of-range unsigned value into signed.
            output.vertices.push_back({
                std::bit_cast<std::int16_t>(u16le(bytes, at)),
                std::bit_cast<std::int16_t>(u16le(bytes, at + 2)),
                std::bit_cast<std::int16_t>(u16le(bytes, at + 4))
            });
        }
        const std::size_t index_start = header + vertex_bytes;
        for (std::size_t i = 0; i < indices; ++i) {
            const auto index = u16le(bytes, index_start + i * 2);
            if (index >= vertices) return std::unexpected(DecodeError::invalid_index);
            output.indices.push_back(index);
        }
        return std::move(output);
    }
};

template<class Tag>
concept Supported = requires(ByteView bytes) {
    typename Spec<Tag>::asset_type;
    { Spec<Tag>::decode(bytes) } ->
        std::same_as<std::expected<typename Spec<Tag>::asset_type, DecodeError>>;
};

template<Supported Tag>
using Asset = typename Spec<Tag>::asset_type;

template<class Tag>
struct AssetId {
    std::size_t slot{};
};

// All three concrete moves are noexcept for the default std::vector
// allocator. That matters for vector reallocation's strong guarantee.
using AssetRecord = std::variant<NromCartridge, TextureRgba8, TriangleMesh>;
static_assert(std::is_nothrow_move_constructible_v<AssetRecord>);

class AssetStore {
public:
    explicit AssetStore(std::size_t capacity) noexcept : capacity_(capacity) {}
    AssetStore(const AssetStore&) = delete;
    AssetStore& operator=(const AssetStore&) = delete;

    [[nodiscard]] std::size_t count() const noexcept { return records_.size(); }

    template<Supported Tag>
    std::expected<AssetId<Tag>, Failure> import(
        ByteView raw, TestFault fault = TestFault::none)
    {
        if (records_.size() >= capacity_)
            return std::unexpected(Failure{PublishError::capacity_exhausted, {}});
        if (fault == TestFault::before_stage)
            return std::unexpected(Failure{PublishError::injected_failure, {}});
        try {
            auto staged = Spec<Tag>::decode(raw);
            if (!staged)
                return std::unexpected(Failure{PublishError::rejected_source, staged.error()});
            if (fault == TestFault::after_stage)
                return std::unexpected(Failure{PublishError::injected_failure, {}});
            if (fault == TestFault::throw_bad_alloc_after_stage)
                throw std::bad_alloc{};

            const auto next = records_.size();
            // No new element is visible until this finishes; if allocation
            // fails, noexcept relocation preserves old records.
            records_.emplace_back(std::in_place_type<Asset<Tag>>, std::move(*staged));
            return AssetId<Tag>{next};
        } catch (const std::bad_alloc&) {
            return std::unexpected(Failure{PublishError::allocation_failure, {}});
        }
    }

    template<Supported Tag>
    [[nodiscard]] const Asset<Tag>* find(AssetId<Tag> id) const noexcept {
        // Borrowed pointer. Any later import may invalidate this address.
        if (id.slot >= records_.size()) return nullptr;
        return std::get_if<Asset<Tag>>(&records_[id.slot]);
    }

private:
    std::size_t capacity_{};
    std::vector<AssetRecord> records_;
};

static_assert(Supported<CartridgeTag>);
static_assert(Supported<TextureTag>);
static_assert(Supported<MeshTag>);
struct UnsupportedAsset {};
static_assert(!Supported<UnsupportedAsset>);

} // namespace allmight::nes::assets
