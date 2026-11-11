import allmight.asset_query;
// Another importing translation unit uses an exported inline constexpr function.
bool secondary_importer_ok() {
    return allmight::asset_query::kApiEpoch == 1 &&
           allmight::asset_query::has_generation({42, 3});
}
