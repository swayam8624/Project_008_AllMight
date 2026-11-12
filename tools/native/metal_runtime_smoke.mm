// Native Metal runtime witness for PROJECTALLMIGHT.
// Executes a compiled compute kernel when the CI/macOS host exposes a Metal device.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include <cstdint>
#include <iostream>

int main(int argc, char** argv) {
    @autoreleasepool {
        if (argc != 2) {
            std::cerr << "usage: metal_runtime_smoke <metallib>\n";
            return 2;
        }

        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (device == nil) {
            std::cout << "metal_runtime_smoke: SKIP (host exposes no Metal device)\n";
            return 77;
        }

        NSError* error = nil;
        NSString* path = [NSString stringWithUTF8String:argv[1]];
        id<MTLLibrary> library = [device newLibraryWithFile:path error:&error];
        if (library == nil) {
            std::cerr << "metal_runtime_smoke: failed to load metallib: "
                      << (error ? [[error localizedDescription] UTF8String] : "unknown") << "\n";
            return 1;
        }

        id<MTLFunction> function = [library newFunctionWithName:@"allmight_prefix_offset"];
        if (function == nil) {
            std::cerr << "metal_runtime_smoke: kernel not found\n";
            return 1;
        }

        id<MTLComputePipelineState> pipeline =
            [device newComputePipelineStateWithFunction:function error:&error];
        if (pipeline == nil) {
            std::cerr << "metal_runtime_smoke: pipeline creation failed: "
                      << (error ? [[error localizedDescription] UTF8String] : "unknown") << "\n";
            return 1;
        }

        constexpr std::uint32_t initial[] = {1, 7, 11, 42};
        constexpr std::uint32_t offset = 5;

        id<MTLBuffer> values =
            [device newBufferWithBytes:initial
                                length:sizeof(initial)
                               options:MTLResourceStorageModeShared];
        id<MTLBuffer> addend =
            [device newBufferWithBytes:&offset
                                length:sizeof(offset)
                               options:MTLResourceStorageModeShared];
        if (values == nil || addend == nil) {
            std::cerr << "metal_runtime_smoke: buffer creation failed\n";
            return 1;
        }

        id<MTLCommandQueue> queue = [device newCommandQueue];
        id<MTLCommandBuffer> command = [queue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [command computeCommandEncoder];
        if (queue == nil || command == nil || encoder == nil) {
            std::cerr << "metal_runtime_smoke: command object creation failed\n";
            return 1;
        }

        [encoder setComputePipelineState:pipeline];
        [encoder setBuffer:values offset:0 atIndex:0];
        [encoder setBuffer:addend offset:0 atIndex:1];

        const NSUInteger width = 4;
        const NSUInteger tg = std::min<NSUInteger>(
            pipeline.maxTotalThreadsPerThreadgroup,
            std::max<NSUInteger>(1, pipeline.threadExecutionWidth));
        [encoder dispatchThreads:MTLSizeMake(width, 1, 1)
          threadsPerThreadgroup:MTLSizeMake(std::min(width, tg), 1, 1)];
        [encoder endEncoding];

        [command commit];
        [command waitUntilCompleted];

        if (command.status != MTLCommandBufferStatusCompleted) {
            std::cerr << "metal_runtime_smoke: command failed";
            if (command.error != nil) {
                std::cerr << ": " << [[command.error localizedDescription] UTF8String];
            }
            std::cerr << "\n";
            return 1;
        }

        const auto* result =
            static_cast<const std::uint32_t*>(values.contents);
        for (std::size_t i = 0; i < 4; ++i) {
            if (result[i] != initial[i] + offset) {
                std::cerr << "metal_runtime_smoke: result mismatch at " << i
                          << " got " << result[i]
                          << " expected " << (initial[i] + offset) << "\n";
                return 1;
            }
        }

        std::cout
            << "metal_runtime_smoke: PASS\n"
            << "proved: MTLDevice/queue/library/pipeline/buffer creation, compute dispatch, "
               "command completion, shared-buffer result visibility\n"
            << "not proved: private heaps, explicit hazard tracking, aliasing, render passes, "
               "multi-queue synchronization, performance counters, or production renderer parity\n";
        return 0;
    }
}
