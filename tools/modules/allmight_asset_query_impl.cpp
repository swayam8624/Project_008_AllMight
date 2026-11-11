module;
#include <cstdint>
module allmight.asset_query; // module implementation unit, not exported

namespace allmight::asset_query {
PreviewPlan make_preview(std::uint64_t byte_count) {
    return PreviewPlan{byte_count, within_budget(byte_count)};
}
}
