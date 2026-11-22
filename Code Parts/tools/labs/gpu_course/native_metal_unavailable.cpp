// Pass15 portable fallback: default CPU CTest remains usable on Linux,
// CI containers and Macs whose build toolchain has no Objective-C++/Metal.
// This is NEVER a successful native GPU run.
#include <iostream>
namespace allmight::gpu_course {
int run_native_metal_probe(bool) {
    std::cout<<"UNAVAILABLE: native macOS Objective-C++ Metal frameworks "
                "were not selected at CMake configure time. No Metal "
                "GPU or benchmark claim is made.\n";
    return 77;
}
} // namespace allmight::gpu_course
