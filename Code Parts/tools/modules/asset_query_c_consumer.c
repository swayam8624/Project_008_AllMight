#include "asset_query_c_api.h"
#include <stdio.h>
/* A separately compiled C11 translation unit cannot import the C++ module.
 * It calls a versioned C-linkage wrapper instead. */
int main(void) {
    if (allmight_asset_approved_v1(64) != 1 ||
        allmight_asset_approved_v1(2048) != 0) return 1;
    puts("asset_query_c_consumer: all checks passed");
    return 0;
}
