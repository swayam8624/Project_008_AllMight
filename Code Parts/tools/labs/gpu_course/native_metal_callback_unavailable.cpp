// Pass16 explicit non-Apple/no-SDK native callback unavailable route.
// Code 77 means NOT EXECUTED on GPU, not a completed correctness run.
#include <iostream>
namespace allmight::gpu_course {
int run_native_gpu_profile_callback_probe() {
    std::cout<<"UNAVAILABLE: native Metal/Foundation/Objective-C++ backend was "
                "not built; CPU profile tests do not certify GPU execution.\n";
    return 77;
}
}
