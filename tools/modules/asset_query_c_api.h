#ifndef ALLMIGHT_ASSET_QUERY_C_API_H
#define ALLMIGHT_ASSET_QUERY_C_API_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Versioned opaque/simple C boundary. C linkage alone does not freeze all ABIs.
int allmight_asset_approved_v1(uint64_t byte_count);
#ifdef __cplusplus
}
#endif
#endif
