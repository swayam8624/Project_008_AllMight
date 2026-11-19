// GPU-16P optional REAL Metal callback and ownership probe.
// Objective-C++ ARC, only in Apple Metal/Foundation toolchains.
// The completion block owns a shared internal receipt and callback, but
// holds only a weak reference to the destroyed renderer/UI observer.
// It never touches a raw Renderer* or calls Swift/SwiftUI off its actor.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "gpu_profile_ownership.hpp"
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>

namespace allmight::gpu_course {
namespace {
namespace p=allmight::gpu_course::profile;
struct Receipt {
    std::mutex mutex;
    std::condition_variable cv;
    bool called{};
    bool accepted{};
    bool ui_expired{};
    bool native_completed{};
    double start{},end{};
};
struct UiObserver {int marker{17};};
} // namespace

int run_native_gpu_profile_callback_probe() {
    @autoreleasepool {
        id<MTLDevice> device=MTLCreateSystemDefaultDevice();
        if(!device) {
            std::cout<<"UNAVAILABLE: no native Metal GPU device.\n";
            return 77;
        }
        id<MTLCommandQueue> queue=[device newCommandQueue];
        if(!queue) return 1;
        NSString* msl=
            @"#include <metal_stdlib>\n"
            @"using namespace metal;\n"
            @"kernel void compute_words(device uint* dst [[buffer(0)]],"
            @" uint id [[thread_position_in_grid]]) {"
            @" if(id<64u) dst[id]=(id*17u)^0x9E3779B9u; }\n";
        NSError* error=nil;
        id<MTLLibrary> lib=[device newLibraryWithSource:msl
                                                options:nil error:&error];
        if(!lib) {
            std::cerr<<"Metal MSL compile ERROR: "
                     <<(error?[[error localizedDescription] UTF8String]:"unknown")
                     <<'\n';
            return 1;
        }
        id<MTLFunction> kernel=[lib newFunctionWithName:@"compute_words"];
        if(!kernel) return 1;
        id<MTLComputePipelineState> pso=[
            device newComputePipelineStateWithFunction:kernel error:&error];
        if(!pso || pso.maxTotalThreadsPerThreadgroup==0) return 1;
        id<MTLBuffer> dst=[
            device newBufferWithLength:64*sizeof(std::uint32_t)
                               options:MTLResourceStorageModeShared];
        if(!dst || !dst.contents) {
            std::cout<<"UNAVAILABLE: native shared Metal buffer.\n";
            return 77;
        }
        auto* words=static_cast<std::uint32_t*>(dst.contents);
        for(unsigned i=0;i<64;++i) words[i]=0xDEADBEEFU;
        id<MTLCommandBuffer> command=[queue commandBuffer];
        if(!command) return 1;
        id<MTLComputeCommandEncoder> enc=[command computeCommandEncoder];
        if(!enc) return 1;
        [enc setComputePipelineState:pso];
        [enc setBuffer:dst offset:0 atIndex:0];
        [enc dispatchThreads:MTLSizeMake(64,1,1)
          threadsPerThreadgroup:MTLSizeMake(
              std::min<NSUInteger>(64,pso.maxTotalThreadsPerThreadgroup),1,1)];
        [enc endEncoding];

        auto ui=std::make_shared<UiObserver>();
        std::weak_ptr<UiObserver> ui_weak=ui;
        auto mailbox=std::make_unique<p::CompletionMailbox>(1);
        // Objective-C 'id' is a type keyword in the following block.
        // Naming an ordinary C++ local variable 'id' hides that typedef and
        // breaks ^(id<MTLCommandBuffer> done) parsing under AppleClang.
        auto frame_id=mailbox->reserve(p::Project::maveb);
        if(!frame_id || !mailbox->publish(*frame_id)) return 1;
        auto safe_callback=mailbox->callback_for(*frame_id);
        auto receipt=std::make_shared<Receipt>();
        [command addCompletedHandler:
            ^(id<MTLCommandBuffer> done) {
                p::NativeObservation note;
                note.terminal=done.status==MTLCommandBufferStatusCompleted?
                    p::Terminal::completed:p::Terminal::failed;
                note.gpu_start_seconds=done.GPUStartTime;
                note.gpu_end_seconds=done.GPUEndTime;
                note.origin=p::Origin::native_metal_observation;
                // Pixel/word correctness is checked only AFTER native
                // completion, not fabricated inside the callback.
                bool published=false;
                try {
                    published=safe_callback(note).has_value();
                } catch(...) {
                    // C++ exceptions never cross the Objective-C callback.
                }
                {
                    std::lock_guard lock(receipt->mutex);
                    receipt->called=true;
                    receipt->accepted=published;
                    receipt->ui_expired=ui_weak.expired();
                    receipt->native_completed=
                        done.status==MTLCommandBufferStatusCompleted;
                    receipt->start=done.GPUStartTime;
                    receipt->end=done.GPUEndTime;
                }
                receipt->cv.notify_all();
            }];
        // Destroy both original owning wrappers before GPU submission:
        // callback must retain *only* its internal state.
        ui.reset();
        mailbox.reset();
        [command commit];
        [command waitUntilCompleted];
        {
            std::unique_lock lock(receipt->mutex);
            if(!receipt->cv.wait_for(lock,std::chrono::seconds(5),
                                    [&]{return receipt->called;}))
                return 1;
            if(!receipt->accepted || !receipt->ui_expired ||
               !receipt->native_completed)
                return 1;
        }
        if(command.status!=MTLCommandBufferStatusCompleted) return 1;
        for(std::uint32_t i=0;i<64;++i) {
            if(words[i]!=((i*17U)^0x9E3779B9U)) {
                std::cerr<<"Metal GPU/CPU mismatch at word "<<i<<'\n';
                return 1;
            }
        }
        p::NativeObservation t;
        t.terminal=p::Terminal::completed;
        t.gpu_start_seconds=receipt->start;
        t.gpu_end_seconds=receipt->end;
        const auto elapsed=p::gpu_interval_ms(t);
        std::cout<<"NATIVE Metal completion and 64-word GPU/CPU oracle PASS; "
                    "renderer/UI destroyed before callback, no UAF.\n";
        if(elapsed)
            std::cout<<"Command GPU interval diagnostic: "<<*elapsed
                     <<" ms (single sample, NOT benchmark).\n";
        else
            std::cout<<"GPU interval UNAVAILABLE; no fabricated 0 ms or "
                        "unsupported hardware counters.\n";
        return 0;
    }
}
} // namespace allmight::gpu_course
