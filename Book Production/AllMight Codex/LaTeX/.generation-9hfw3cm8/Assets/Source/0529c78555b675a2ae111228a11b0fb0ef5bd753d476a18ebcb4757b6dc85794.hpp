#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

namespace allmight::m018 {

// A handle is a logical identity, not a pointer to reallocatable storage.
struct MeshTag {};
struct TextureTag {};
template<class Kind> struct Handle {
    std::uint64_t catalog_id{};
    std::uint32_t slot{};
    std::uint64_t generation{};
    friend constexpr bool operator==(Handle, Handle) = default;
};
using MeshHandle = Handle<MeshTag>;
using TextureHandle = Handle<TextureTag>;
static_assert(!std::is_same_v<MeshHandle, TextureHandle>);

struct MeshInfo {
    std::string name;
    std::uint32_t vertex_count{};
};

class AssetCatalog {
public:
    AssetCatalog();
    ~AssetCatalog(); // Out-of-line, where Impl is complete.
    AssetCatalog(const AssetCatalog&) = delete;
    AssetCatalog& operator=(const AssetCatalog&) = delete;
    AssetCatalog(AssetCatalog&&) noexcept;
    AssetCatalog& operator=(AssetCatalog&&) noexcept;

    [[nodiscard]] MeshHandle insert(MeshInfo mesh);
    [[nodiscard]] bool erase(MeshHandle handle);
    // Owned snapshot: no pointer/reference to an internal vector escapes.
    [[nodiscard]] std::optional<MeshInfo> lookup(MeshHandle handle) const;
    [[nodiscard]] std::size_t live_count() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace allmight::m018
