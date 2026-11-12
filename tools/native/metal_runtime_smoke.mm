// Native Metal runtime witness for PROJECTALLMIGHT.
// Executes a compiled compute kernel when the CI/macOS host exposes a Metal device.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include <algorithm>
#include <array>
#include <cmath>
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
        NSURL* libraryURL = [NSURL fileURLWithPath:path];
        id<MTLLibrary> library = [device newLibraryWithURL:libraryURL error:&error];
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


        // A second native pipeline proves the Phase-29 compressed-kernel boundary:
        // signed int8 storage is dequantized by a scale inside the Metal kernel.
        id<MTLFunction> q8Function = [library newFunctionWithName:@"allmight_q8_dot"];
        if (q8Function == nil) {
            std::cerr << "metal_runtime_smoke: q8 kernel not found\n";
            return 1;
        }
        id<MTLComputePipelineState> q8Pipeline =
            [device newComputePipelineStateWithFunction:q8Function error:&error];
        if (q8Pipeline == nil) {
            std::cerr << "metal_runtime_smoke: q8 pipeline creation failed: "
                      << (error ? [[error localizedDescription] UTF8String] : "unknown") << "\n";
            return 1;
        }

        constexpr std::array<std::int8_t, 8> qweights{
            127, -64, 32, 0, -7, 19, -101, 55
        };
        constexpr std::array<float, 8> input{
            0.5F, -1.0F, 2.0F, 4.0F, -3.0F, 0.25F, 1.5F, -0.75F
        };
        constexpr std::uint32_t qcount = static_cast<std::uint32_t>(qweights.size());
        constexpr float qscale = 0.0125F;
        float qexpected = 0.0F;
        for (std::size_t i = 0; i < qweights.size(); ++i) {
            qexpected += static_cast<float>(qweights[i]) * qscale * input[i];
        }
        float qinitial = -999.0F;

        id<MTLBuffer> qweightsBuffer =
            [device newBufferWithBytes:qweights.data()
                                length:sizeof(qweights)
                               options:MTLResourceStorageModeShared];
        id<MTLBuffer> inputBuffer =
            [device newBufferWithBytes:input.data()
                                length:sizeof(input)
                               options:MTLResourceStorageModeShared];
        id<MTLBuffer> countBuffer =
            [device newBufferWithBytes:&qcount
                                length:sizeof(qcount)
                               options:MTLResourceStorageModeShared];
        id<MTLBuffer> scaleBuffer =
            [device newBufferWithBytes:&qscale
                                length:sizeof(qscale)
                               options:MTLResourceStorageModeShared];
        id<MTLBuffer> outputBuffer =
            [device newBufferWithBytes:&qinitial
                                length:sizeof(qinitial)
                               options:MTLResourceStorageModeShared];
        if (qweightsBuffer == nil || inputBuffer == nil || countBuffer == nil ||
            scaleBuffer == nil || outputBuffer == nil) {
            std::cerr << "metal_runtime_smoke: q8 buffer creation failed\n";
            return 1;
        }

        id<MTLCommandBuffer> q8Command = [queue commandBuffer];
        id<MTLComputeCommandEncoder> q8Encoder = [q8Command computeCommandEncoder];
        if (q8Command == nil || q8Encoder == nil) {
            std::cerr << "metal_runtime_smoke: q8 command object creation failed\n";
            return 1;
        }
        [q8Encoder setComputePipelineState:q8Pipeline];
        [q8Encoder setBuffer:qweightsBuffer offset:0 atIndex:0];
        [q8Encoder setBuffer:inputBuffer offset:0 atIndex:1];
        [q8Encoder setBuffer:countBuffer offset:0 atIndex:2];
        [q8Encoder setBuffer:scaleBuffer offset:0 atIndex:3];
        [q8Encoder setBuffer:outputBuffer offset:0 atIndex:4];
        [q8Encoder dispatchThreads:MTLSizeMake(1, 1, 1)
           threadsPerThreadgroup:MTLSizeMake(1, 1, 1)];
        [q8Encoder endEncoding];
        [q8Command commit];
        [q8Command waitUntilCompleted];
        if (q8Command.status != MTLCommandBufferStatusCompleted) {
            std::cerr << "metal_runtime_smoke: q8 command failed";
            if (q8Command.error != nil) {
                std::cerr << ": " << [[q8Command.error localizedDescription] UTF8String];
            }
            std::cerr << "\n";
            return 1;
        }
        const float qactual = *static_cast<const float*>(outputBuffer.contents);
        if (std::fabs(qactual - qexpected) > 1.0e-5F * std::max(1.0F, std::fabs(qexpected))) {
            std::cerr << "metal_runtime_smoke: q8 result mismatch got " << qactual
                      << " expected " << qexpected << "\n";
            return 1;
        }

        std::cout
            << "metal_runtime_smoke: PASS\n"
            << "proved: MTLDevice/queue/library/pipeline/buffer creation, compute dispatch, "
               "command completion, shared-buffer result visibility, native int8 dequantized-dot execution\n"
            << "not proved: optimized/vectorized quantized kernels, private heaps, explicit hazard tracking, aliasing, render passes, "
               "multi-queue synchronization, performance counters, or production renderer parity\n";
        return 0;
    }
}
