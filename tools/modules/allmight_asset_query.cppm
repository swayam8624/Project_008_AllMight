module;
#include <cstddef>
#include <cstdint>
export module allmight.asset_query;
export import :rules;

export namespace allmight::asset_query {
struct AssetHandle { std::uint32_t slot, generation; };
struct PreviewPlan { std::uint64_t byte_count; bool accepted; };
PreviewPlan make_preview(std::uint64_t byte_count); // defined in implementation unit
inline constexpr std::uint32_t kApiEpoch = 1;
inline constexpr bool has_generation(AssetHandle h) noexcept {
    return h.generation != 0;
}
}
