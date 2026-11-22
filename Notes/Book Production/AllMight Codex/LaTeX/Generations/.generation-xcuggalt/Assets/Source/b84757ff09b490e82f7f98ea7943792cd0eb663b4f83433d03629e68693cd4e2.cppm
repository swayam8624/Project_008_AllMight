module;
#include <cstddef>
export module allmight.asset_query:rules;

export namespace allmight::asset_query {
inline constexpr std::size_t kMaxPreviewBytes = 1024;
inline constexpr bool within_budget(std::size_t bytes) noexcept {
    return bytes > 0 && bytes <= kMaxPreviewBytes;
}
}
