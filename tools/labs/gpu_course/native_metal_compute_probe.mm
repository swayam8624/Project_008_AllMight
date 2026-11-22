// Project AllMight / GPU-15M — actual native macOS Metal 3-era execution.
//
// Objective-C++ bridge intentionally compiled only with Apple Metal and
// Foundation frameworks, using ARC. No metal-cpp headers, private SDKs,
// window, CAMetalLayer, HDR surface or external Maveb build are required.
//
// REAL native proof on --probe-native:
//   1. Compile MSL compute broad-phase AND vertex/fragment shaders.
//   2. Allocate host-visible shared AABBs and pair mask, offscreen PRIVATE
//      BGRA8Unorm render target and 256-byte-pitch shared capture buffer.
//   3. Encode a compute dispatch and a real red full-screen triangle draw.
//   4. Blit 3x2 render target into padded readback rows.
//   5. Commit, wait for ACTUAL terminal MTLCommandBufferStatusCompleted.
//   6. Compare 32x32 pair mask to independent CPU half-open AABB overlap,
//      check guard words, then verify every BGRA pixel exactly red.
//   7. Report GPUStartTime/GPUEndTime ONLY if finite, monotonic and nonzero.
//
// --benchmark-native: five warmups followed by 30 independently verified
// frames. Reports median/p95 host encode/commit versus GPU command interval.
// This is NOT a whole Maveb renderer FPS, XR latency or statistical CI.
// No measurement is printed unless a native device actually completed work.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace allmight::gpu_course {
namespace {
constexpr std::size_t kBoxCount=32;
constexpr std::size_t kPairCount=kBoxCount*kBoxCount;
constexpr std::size_t kGuard=16;
constexpr std::size_t kCapturePitch=256;
constexpr std::size_t kCaptureHeight=2;
constexpr std::uint32_t kGuardWord=0xA51DEAD5U;

// Match the explicitly declared Metal-Shading-Language struct field order.
// Never cast arbitrary scene C++ object memory into a device buffer.
struct alignas(4) Aabb {
    std::uint32_t min_x{}, min_y{}, max_x{}, max_y{};
};
static_assert(sizeof(Aabb)==16);
static_assert(alignof(Aabb)==4);

std::vector<Aabb> make_boxes() {
    std::vector<Aabb> boxes(kBoxCount);
    for(std::size_t i=0;i<kBoxCount;++i) {
        const auto x=static_cast<std::uint32_t>((i*7U)%17U);
        const auto y=static_cast<std::uint32_t>((i*11U)%13U);
        // The loop bound is 32; the modulo terms are bounded to 0..1 and
        // 0..2. Convert those explicit tiny indices before combining with
        // the uint32 coordinates, rather than narrowing size_t in a braced
        // initializer (which AppleClang correctly rejects).
        const auto dx=3U+static_cast<std::uint32_t>(i%2U);
        const auto dy=2U+static_cast<std::uint32_t>(i%3U);
        boxes[i]={x,y,x+dx,y+dy};
    }
    return boxes;
}
// Half-open AABB intersection: touching boundary is NOT overlap. The native
// shader independently implements this same declared geometry predicate.
std::uint32_t cpu_overlap(const Aabb& a,const Aabb& b) noexcept {
    return a.max_x>b.min_x && b.max_x>a.min_x &&
           a.max_y>b.min_y && b.max_y>a.min_y ? 1U : 0U;
}
std::string ns_description(NSError* error) {
    if(!error) return "error object unavailable";
    const char* utf8=[[error localizedDescription] UTF8String];
    return utf8?std::string(utf8):"error description not UTF8";
}
struct Sample {
    double encode_submit_ms{};
    double gpu_ms{};
    bool gpu_time_valid{};
};
[[nodiscard]] double nearest_rank_percentile(
    std::vector<double> values,double fraction) {
    std::sort(values.begin(),values.end());
    const auto rank=static_cast<std::size_t>(
        std::ceil(fraction*static_cast<double>(values.size())));
    return values[std::max<std::size_t>(1,rank)-1];
}
} // namespace

int run_native_metal_probe(bool benchmark) {
    @autoreleasepool {
        id<MTLDevice> device=MTLCreateSystemDefaultDevice();
        if(!device) {
            std::cout<<"UNAVAILABLE: MTLCreateSystemDefaultDevice returned nil. "
                        "No Metal GPU result is claimed.\n";
            return 77;
        }
        id<MTLCommandQueue> queue=[device newCommandQueue];
        if(!queue) {
            std::cerr<<"ERROR: native Metal command queue creation failed.\n";
            return 1;
        }

        // This is deliberately inline, ORIGINAL teaching MSL; it does not
        // replace or silently pretend to compile Maveb's actual shaders.
        NSString* source =
            @"#include <metal_stdlib>\n"
            @"using namespace metal;\n"
            @"struct Aabb { uint min_x; uint min_y; uint max_x; uint max_y; };\n"
            @"kernel void exact_pair_mask(device const Aabb* boxes [[buffer(0)]],\n"
            @"  device uint* out [[buffer(1)]], constant uint& count [[buffer(2)]],\n"
            @"  uint gid [[thread_position_in_grid]]) {\n"
            @"  if (gid >= count * count) return;\n"
            @"  uint a = gid / count; uint b = gid % count;\n"
            @"  Aabb x = boxes[a]; Aabb y = boxes[b];\n"
            @"  bool overlap = x.max_x > y.min_x && y.max_x > x.min_x &&\n"
            @"                 x.max_y > y.min_y && y.max_y > x.min_y;\n"
            @"  out[gid] = (a < b && overlap) ? 1u : 0u;\n"
            @"}\n"
            @"struct VOut { float4 position [[position]]; };\n"
            @"vertex VOut fullscreen_triangle(uint vid [[vertex_id]]) {\n"
            @"  float2 p[3] = { float2(-1.0,-1.0), float2(3.0,-1.0),\n"
            @"                   float2(-1.0,3.0) };\n"
            @"  VOut o; o.position = float4(p[vid], 0.0, 1.0); return o;\n"
            @"}\n"
            @"fragment float4 solid_red(VOut in [[stage_in]]) {\n"
            @"  return float4(1.0, 0.0, 0.0, 1.0);\n"
            @"}\n";
        NSError* error=nil;
        id<MTLLibrary> library=[device newLibraryWithSource:source
                                                       options:nil
                                                         error:&error];
        if(!library) {
            std::cerr<<"ERROR: original Metal Shading Language compile failed: "
                     <<ns_description(error)<<'\n';
            return 1;
        }
        id<MTLFunction> computeFunction=[
            library newFunctionWithName:@"exact_pair_mask"];
        id<MTLFunction> vertexFunction=[
            library newFunctionWithName:@"fullscreen_triangle"];
        id<MTLFunction> fragmentFunction=[
            library newFunctionWithName:@"solid_red"];
        if(!computeFunction || !vertexFunction || !fragmentFunction) {
            std::cerr<<"ERROR: MSL entry points missing from compiled library.\n";
            return 1;
        }
        id<MTLComputePipelineState> computePSO=[
            device newComputePipelineStateWithFunction:computeFunction
                                                 error:&error];
        if(!computePSO) {
            std::cerr<<"ERROR: Metal compute pipeline: "
                     <<ns_description(error)<<'\n';
            return 1;
        }
        MTLRenderPipelineDescriptor* pipelineDesc=[
            [MTLRenderPipelineDescriptor alloc] init];
        pipelineDesc.vertexFunction=vertexFunction;
        pipelineDesc.fragmentFunction=fragmentFunction;
        pipelineDesc.colorAttachments[0].pixelFormat=MTLPixelFormatBGRA8Unorm;
        id<MTLRenderPipelineState> renderPSO=[
            device newRenderPipelineStateWithDescriptor:pipelineDesc
                                                  error:&error];
        if(!renderPSO) {
            std::cerr<<"ERROR: Metal render pipeline: "
                     <<ns_description(error)<<'\n';
            return 1;
        }

        const auto boxes=make_boxes();
        id<MTLBuffer> boxBuffer=[
            device newBufferWithLength:boxes.size()*sizeof(Aabb)
                               options:MTLResourceStorageModeShared];
        id<MTLBuffer> maskBuffer=[
            device newBufferWithLength:(kPairCount+kGuard)*sizeof(std::uint32_t)
                               options:MTLResourceStorageModeShared];
        id<MTLBuffer> readback=[
            device newBufferWithLength:kCapturePitch*kCaptureHeight
                               options:MTLResourceStorageModeShared];
        MTLTextureDescriptor* textureDesc=[
            MTLTextureDescriptor texture2DDescriptorWithPixelFormat:
                MTLPixelFormatBGRA8Unorm width:3 height:2 mipmapped:NO];
        textureDesc.storageMode=MTLStorageModePrivate;
        textureDesc.usage=MTLTextureUsageRenderTarget;
        id<MTLTexture> offscreen=[device newTextureWithDescriptor:textureDesc];
        if(!boxBuffer || !maskBuffer || !readback || !offscreen ||
           !boxBuffer.contents || !maskBuffer.contents ||
           !readback.contents) {
            std::cout<<"UNAVAILABLE: Metal device cannot create the "
                        "required private render target and shared buffers.\n";
            return 77;
        }
        std::memcpy(boxBuffer.contents,boxes.data(),
                    boxes.size()*sizeof(Aabb));

        const auto invocations=benchmark?35U:1U;
        const auto warmups=benchmark?5U:0U;
        std::vector<double> cpuTimes,gpuTimes;
        cpuTimes.reserve(invocations-warmups);
        gpuTimes.reserve(invocations-warmups);
        constexpr std::uint32_t count=static_cast<std::uint32_t>(kBoxCount);
        const NSUInteger maxGroup=computePSO.maxTotalThreadsPerThreadgroup;
        if(maxGroup==0) {
            std::cerr<<"ERROR: Metal compute pipeline reports no legal threads.\n";
            return 1;
        }
        const NSUInteger perGroup=std::min<NSUInteger>(64U,maxGroup);

        for(unsigned trial=0;trial<invocations;++trial) {
            // All buffers are reused only after the PRIOR command buffer's
            // verified terminal completion. No implicit multi-frame overlap.
            auto* out=static_cast<std::uint32_t*>(maskBuffer.contents);
            for(std::size_t i=0;i<kPairCount+kGuard;++i)
                out[i]=kGuardWord;
            std::memset(readback.contents,0xAD,
                        kCapturePitch*kCaptureHeight);

            const auto before=std::chrono::steady_clock::now();
            @autoreleasepool {
                id<MTLCommandBuffer> cmd=[queue commandBuffer];
                if(!cmd) {
                    std::cerr<<"ERROR: no Metal command buffer.\n";
                    return 1;
                }
                id<MTLComputeCommandEncoder> compute=[
                    cmd computeCommandEncoder];
                if(!compute) {
                    std::cerr<<"ERROR: no Metal compute encoder.\n";
                    return 1;
                }
                [compute setComputePipelineState:computePSO];
                [compute setBuffer:boxBuffer offset:0 atIndex:0];
                [compute setBuffer:maskBuffer offset:0 atIndex:1];
                [compute setBytes:&count length:sizeof(count) atIndex:2];
                [compute dispatchThreads:MTLSizeMake(kPairCount,1,1)
                    threadsPerThreadgroup:MTLSizeMake(perGroup,1,1)];
                [compute endEncoding];

                MTLRenderPassDescriptor* pass=[
                    MTLRenderPassDescriptor renderPassDescriptor];
                pass.colorAttachments[0].texture=offscreen;
                pass.colorAttachments[0].loadAction=MTLLoadActionClear;
                pass.colorAttachments[0].storeAction=MTLStoreActionStore;
                pass.colorAttachments[0].clearColor=MTLClearColorMake(0,0,0,1);
                id<MTLRenderCommandEncoder> render=[
                    cmd renderCommandEncoderWithDescriptor:pass];
                if(!render) {
                    std::cerr<<"ERROR: no Metal render encoder.\n";
                    return 1;
                }
                [render setRenderPipelineState:renderPSO];
                [render drawPrimitives:MTLPrimitiveTypeTriangle
                         vertexStart:0 vertexCount:3];
                [render endEncoding];

                id<MTLBlitCommandEncoder> blit=[cmd blitCommandEncoder];
                if(!blit) {
                    std::cerr<<"ERROR: no Metal blit encoder.\n";
                    return 1;
                }
                [blit copyFromTexture:offscreen
                         sourceSlice:0 sourceLevel:0
                        sourceOrigin:MTLOriginMake(0,0,0)
                          sourceSize:MTLSizeMake(3,2,1)
                            toBuffer:readback destinationOffset:0
             destinationBytesPerRow:kCapturePitch
           destinationBytesPerImage:kCapturePitch*kCaptureHeight];
                [blit endEncoding];
                [cmd commit];
                const auto after=std::chrono::steady_clock::now();
                [cmd waitUntilCompleted];
                if(cmd.status!=MTLCommandBufferStatusCompleted) {
                    std::cerr<<"ERROR: native Metal command terminal status "
                             <<static_cast<int>(cmd.status)
                             <<" "<<ns_description(cmd.error)<<'\n';
                    return 1;
                }

                // Independent, pair-by-pair CPU oracle. Not an expected
                // output copied from the shader: recalculate every result.
                for(std::size_t i=0;i<kBoxCount;++i)
                    for(std::size_t j=0;j<kBoxCount;++j) {
                        const auto expected=i<j?
                            cpu_overlap(boxes[i],boxes[j]):0U;
                        if(out[i*kBoxCount+j]!=expected) {
                            std::cerr<<"ERROR: pair-mask mismatch at ("<<i
                                     <<","<<j<<"), expected "<<expected
                                     <<", GPU "<<out[i*kBoxCount+j]<<'\n';
                            return 1;
                        }
                    }
                for(std::size_t i=kPairCount;i<kPairCount+kGuard;++i)
                    if(out[i]!=kGuardWord) {
                        std::cerr<<"ERROR: GPU pair shader modified guard "
                                    "element "<<i<<'\n';
                        return 1;
                    }

                // MTLPixelFormatBGRA8Unorm: solid opaque red should map to
                // BGRA bytes [0, 0, 255, 255]; discard 256-byte row padding.
                const auto* pixels=static_cast<const std::uint8_t*>(
                    readback.contents);
                for(std::size_t y=0;y<2;++y)
                    for(std::size_t x=0;x<3;++x) {
                        const auto* p=pixels+y*kCapturePitch+x*4;
                        if(p[0]!=0 || p[1]!=0 ||
                           p[2]!=255 || p[3]!=255) {
                            std::cerr<<"ERROR: offscreen Metal render pixel "
                                     <<x<<","<<y<<": "
                                     <<static_cast<int>(p[0])<<","
                                     <<static_cast<int>(p[1])<<","
                                     <<static_cast<int>(p[2])<<","
                                     <<static_cast<int>(p[3])<<'\n';
                            return 1;
                        }
                    }
                if(trial>=warmups) {
                    const auto cpuMs=
                        std::chrono::duration<double,std::milli>(
                            after-before).count();
                    cpuTimes.push_back(cpuMs);
                    // Apple's properties may be 0.0 or unavailable;
                    // this is a single command-buffer GPU interval,
                    // not a rendered-frame FPS or presentation latency.
                    const double start=cmd.GPUStartTime;
                    const double end=cmd.GPUEndTime;
                    if(std::isfinite(start) && std::isfinite(end) &&
                       start>0.0 && end>start)
                        gpuTimes.push_back((end-start)*1000.0);
                }
            }
        }
        const char* gpuName=[[device name] UTF8String];
        std::cout<<"NATIVE METAL PASS: "
                 <<(gpuName?gpuName:"unnamed MTLDevice")
                 <<", MSL compute "<<kPairCount
                 <<" pair candidates (exact CPU oracle + guard), "
                 <<"3x2 offscreen BGRA8 render + padded blit checked, "
                 <<invocations<<" command buffers completed.\n";
        if(!benchmark) {
            if(!gpuTimes.empty())
                std::cout<<"GPU command interval diagnostic sample: "
                         <<gpuTimes.front()<<" ms (not a benchmark).\n";
            else
                std::cout<<"GPU command interval: UNAVAILABLE; "
                            "no 0-ms result fabricated.\n";
        } else {
            std::cout<<"MEASURED (5 warmups + 30 samples; NOT production FPS): "
                     <<"CPU encode-to-commit median "
                     <<nearest_rank_percentile(cpuTimes,0.5)<<" ms, p95 "
                     <<nearest_rank_percentile(cpuTimes,0.95)<<" ms.\n";
            if(gpuTimes.size()==cpuTimes.size())
                std::cout<<"GPU command interval median "
                         <<nearest_rank_percentile(gpuTimes,0.5)
                         <<" ms, p95 "<<nearest_rank_percentile(gpuTimes,0.95)
                         <<" ms.\n";
            else
                std::cout<<"GPU interval unavailable for "
                         <<(cpuTimes.size()-gpuTimes.size())
                         <<" sample(s); no comparable GPU percentile.\n";
        }
        return 0;
    }
}
} // namespace allmight::gpu_course
