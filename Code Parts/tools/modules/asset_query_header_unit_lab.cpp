#include <iostream>
import "asset_query_interop.hpp";
import allmight.asset_query;

int main() {
    if (kLegacyAssetFormatVersion != 7 ||
        !allmight::asset_query::make_preview(64).accepted)
        return 1;
    std::cout << "asset_query_header_unit_lab: all checks passed\n";
}
