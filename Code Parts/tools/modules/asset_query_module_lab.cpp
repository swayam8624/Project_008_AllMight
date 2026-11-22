#include "asset_query_c_api.h"
#include <iostream>
import allmight.asset_query;

bool secondary_importer_ok();

int main() {
    using namespace allmight::asset_query;
    const auto valid = make_preview(64);
    const auto too_big = make_preview(2048);
    if (!valid.accepted || too_big.accepted || !has_generation({1, 2}) ||
        has_generation({1, 0}) || !within_budget(1024) ||
        allmight_asset_approved_v1(64) != 1 ||
        allmight_asset_approved_v1(0) != 0 || kApiEpoch != 1 ||
        !secondary_importer_ok())
        return 1;
    std::cout << "asset_query_module_lab: all checks passed\n";
}
