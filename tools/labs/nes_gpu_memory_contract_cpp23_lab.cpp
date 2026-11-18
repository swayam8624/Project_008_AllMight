// Project AllMight / Book 1 / Pass 09.
//
// A CPU reference oracle for selected *logical* GPU memory contracts.
// It imports the actual NES-05A RGBA8 texture via the NES-06G generational
// registry and checks a Pass-08 conceptual upload/compute/graphics DAG.
// It does not submit GPU commands or prove that a Vk/Metal barrier is correct.
//
// The host atomic handshake is a separate C++ memory-model experiment;
// do NOT infer that C++ memory_order maps directly to a Metal/Vulkan shader.
#include "nes_course/gpu_memory_contract_oracle.hpp"
#include "nes_course/generational_asset_registry.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace {
using namespace allmight::nes;
using namespace allmight::nes::assets;
namespace g = allmight::nes::work;
namespace m = allmight::nes::gpu_model;

void check(bool yes,std::string_view why) {
    if (!yes) throw std::runtime_error(std::string(why));
}
g::ResourceKey key(g::ResourceKind kind,
                   std::uint64_t owner,std::uint64_t slot,
                   std::uint64_t generation) {
    return {kind,owner,slot,generation};
}
std::vector<Byte> texture_image() {
    // TXR4: 2x1 RGBA8, 12-byte header plus 8-byte payload.
    std::vector<Byte> raw(20,0);
    raw[0]='T';raw[1]='X';raw[2]='R';raw[3]='4';
    raw[4]=2;raw[6]=1;
    const std::array<Byte,8> pixels{0x12,0x34,0x56,0xFF,
                                    0xAB,0xCD,0xEF,0x44};
    for (std::size_t i=0;i<pixels.size();++i) raw[12+i]=pixels[i];
    return raw;
}
void check_allocation_and_validation() {
    m::MemoryContractOracle oracle(96);
    const auto keyA=key(g::ResourceKind::gpu_texture,1,9,2);
    const auto keyB=key(g::ResourceKind::gpu_texture,1,10,2);
    const auto keyC=key(g::ResourceKind::gpu_texture,1,11,2);
    const auto before=oracle.committed_extent();
    check(!oracle.allocate(keyA,0,16) &&
          oracle.committed_extent()==before,
          "zero-byte allocation must fail atomically");
    const auto bad=oracle.allocate(keyA,4,3);
    check(!bad && bad.error()==m::MemoryError::invalid_alignment &&
          oracle.committed_extent()==before,
          "non-power-of-two alignment must be rejected");
    const auto first=oracle.allocate(keyA,8,16);
    check(first && first->offset==0 && first->size==8,
          "first virtual allocation at zero");
    const auto second=oracle.allocate(keyB,8,32);
    check(second && second->offset==32 && second->alignment==32,
          "second allocation offset aligned up to 32");
    check(!oracle.allocate(keyA,8,16) &&
          oracle.committed_extent()==40,
          "duplicate key must refuse without changing virtual offset");
    const auto too_big=oracle.allocate(keyC,80,16);
    check(!too_big &&
          too_big.error()==m::MemoryError::capacity_exhausted &&
          oracle.committed_extent()==40,
          "arena budget must refuse aligned overflow");
    const auto missing=oracle.describe(keyC);
    check(!missing && missing.error()==m::MemoryError::missing_identity,
          "missing allocation must not be fabricated");
    const auto desc=oracle.describe(keyB);
    check(desc && desc->live && desc->allocation.size==8 &&
          desc->layout==m::Layout::undefined,
          "descriptor reports the actual virtual state");

    const auto zero=oracle.read(keyA,g::Domain::host,0,8);
    check(!zero && zero.error()==m::MemoryError::uninitialized,
          "allocation alone must not imply initialized contents");
    const std::array<Byte,2> partial{0x41,0x42};
    check(oracle.write(keyA,g::Domain::host,0,partial).has_value(),
          "two-byte host initialization");
    const auto known=oracle.read(keyA,g::Domain::host,0,2);
    check(known && (*known)==std::vector<Byte>({0x41,0x42}),
          "initialized range is readable");
    const auto larger=oracle.read(keyA,g::Domain::host,0,3);
    check(!larger && larger.error()==m::MemoryError::uninitialized,
          "partially initialized allocation must reject larger read");
    const auto beyond=oracle.write(keyA,g::Domain::host,7,partial);
    check(!beyond && beyond.error()==m::MemoryError::range_out_of_bounds,
          "offset+length must be bounds checked before mutation");
    const auto still=oracle.read(keyA,g::Domain::host,0,2);
    check(still && *still==*known,
          "failed out-of-bounds write did not alter previous bytes");

    const auto invalidPartial=oracle.barrier({keyA,g::Domain::host,
        g::Domain::gpu_transfer,m::Layout::undefined,
        m::Layout::transfer_source,0,2});
    check(!invalidPartial &&
          invalidPartial.error()==m::MemoryError::invalid_barrier,
          "this oracle permits role/layout transfers only for whole resources");
    const auto state=oracle.describe(keyA);
    check(state && state->owner==g::Domain::host &&
          state->layout==m::Layout::undefined,
          "rejected barrier leaves resource role/layout unchanged");
}
void real_texture_through_upload_plan() {
    GenerationalAssetRegistry registry(2);
    auto image=texture_image();
    const auto loaded=registry.import<TextureTag>(image);
    check(loaded.has_value(),"Pass05 RGBA8 decoder must accept our fixture");
    const auto owned=registry.lookup(*loaded);
    check(owned && owned->get().pixels.size()==8,
          "Pass06 typed handle retrieves owned texture pixels");
    image.clear();
    image.shrink_to_fit();
    const auto& pixels=owned->get().pixels;
    const auto stage=key(g::ResourceKind::texture,loaded->owner,
                         static_cast<std::uint64_t>(loaded->slot),
                         loaded->generation);
    const auto device=key(g::ResourceKind::gpu_texture,loaded->owner,
                          static_cast<std::uint64_t>(loaded->slot),
                          loaded->generation);
    const auto computed=key(g::ResourceKind::gpu_output,loaded->owner,
                            static_cast<std::uint64_t>(loaded->slot),
                            loaded->generation);

    // The real Pass08 planner tells us REQUIRED ordering; it does not issue
    // any native transfer or memory-visibility operation.
    g::DependencyGraph graph;
    check(graph.add({g::TaskId{10},"host pixels",g::Domain::host,1,{},{
        {stage,true,false}}}).has_value(),"host source task");
    check(graph.add({g::TaskId{20},"transfer pixels",g::Domain::gpu_transfer,2,
        {g::TaskId{10}},{{stage,true,false},{device,false,true}}}).has_value(),
        "copy task");
    check(graph.add({g::TaskId{30},"compute from texture",g::Domain::gpu_compute,3,
        {},{{device,true,false},{computed,false,true}}}).has_value(),
        "compute task");
    check(graph.add({g::TaskId{40},"sample computed pixels",g::Domain::gpu_graphics,4,
        {},{{computed,true,false}}}).has_value(),"graphics task");
    const auto planned=graph.plan();
    check(planned && planned->ordered.size()==4 &&
          planned->gpu_edges_requiring_backend_review.size()>=2,
          "Pass08 graph must include GPU synchronization review obligations");

    m::MemoryContractOracle oracle(256);
    check(oracle.allocate(stage,8,16).has_value(),"CPU staging allocation");
    check(oracle.allocate(device,8,64).has_value(),"virtual GPU destination");
    check(oracle.allocate(computed,8,64).has_value(),"virtual compute output");
    check(oracle.committed_extent()==136,
          "monotonic virtual offsets must be 0,64,128 and end 136");
    check(oracle.write(stage,g::Domain::host,0,pixels).has_value(),
          "CPU-only staging upload of eight owned source bytes");

    const auto unseen=oracle.read(stage,g::Domain::gpu_transfer,0,8);
    check(!unseen && unseen.error()==m::MemoryError::wrong_role,
          "graph edge alone cannot grant transfer-stage resource access");
    check(oracle.barrier({stage,g::Domain::host,g::Domain::gpu_transfer,
        m::Layout::undefined,m::Layout::transfer_source,0,8}),
        "conceptual host-to-transfer ownership and visibility");
    check(oracle.barrier({device,g::Domain::host,g::Domain::gpu_transfer,
        m::Layout::undefined,m::Layout::transfer_destination,0,8}),
        "conceptual destination transition before copy");
    check(oracle.copy_transfer(stage,device,8).has_value(),
          "CPU memory oracle copies eight exact byte values");

    const auto wronglyRead=oracle.read(device,g::Domain::gpu_compute,0,8);
    check(!wronglyRead && wronglyRead.error()==m::MemoryError::wrong_role,
          "compute cannot read a transfer-owned resource yet");
    const auto badBarrier=oracle.barrier({device,g::Domain::gpu_compute,
        g::Domain::gpu_graphics,m::Layout::transfer_destination,
        m::Layout::shader_read_only,0,8});
    check(!badBarrier && badBarrier.error()==m::MemoryError::wrong_role,
          "barrier from wrong source role must fail");
    check(oracle.barrier({device,g::Domain::gpu_transfer,g::Domain::gpu_compute,
        m::Layout::transfer_destination,m::Layout::shader_read_only,0,8}),
        "transfer data made visible to the conceptual compute stage");
    const auto seen=oracle.read(device,g::Domain::gpu_compute,0,8);
    check(seen && *seen==pixels,
          "consumer bytes must equal the owned original RGBA8 payload");

    check(oracle.barrier({computed,g::Domain::host,g::Domain::gpu_compute,
        m::Layout::undefined,m::Layout::general,0,8}),
        "compute output prepared for conceptual general writes");
    std::array<Byte,8> transformed{};
    for (std::size_t i=0;i<8;++i)
        transformed[i]=static_cast<Byte>((*seen)[i]^0xFFU);
    check(oracle.write(computed,g::Domain::gpu_compute,0,transformed),
          "CPU oracle models a compute-generated byte array");
    const auto tooEarly=oracle.read(computed,g::Domain::gpu_graphics,0,8);
    check(!tooEarly && tooEarly.error()==m::MemoryError::wrong_role,
          "graphics may not consume unpublished compute output");
    check(oracle.barrier({computed,g::Domain::gpu_compute,
        g::Domain::gpu_graphics,m::Layout::general,
        m::Layout::shader_read_only,0,8}),
        "conceptual compute-to-graphics visibility+layout transition");
    const auto output=oracle.read(computed,g::Domain::gpu_graphics,0,8);
    check(output && output->size()==8 &&
          (*output)[0]==static_cast<Byte>(0x12U^0xFFU) &&
          (*output)[7]==static_cast<Byte>(0x44U^0xFFU),
          "graphics consumer reads exact transformed CPU model result");

    // An old generational handle becomes invalid after logical registry
    // erasure, regardless of the separately staged virtual device data.
    check(registry.erase(*loaded).has_value(),"remove owned source asset");
    check(!registry.lookup(*loaded),"logical source handle is stale");
    // Here the simulated allocation is STILL live: CPU registry erasure
    // does not imply completion of pending GPU commands or free backing.
    check(oracle.mark_in_flight(device,7).has_value(),"simulate queued read");
    const auto pending=oracle.retire(device);
    check(!pending && pending.error()==m::MemoryError::completion_pending,
          "pending conceptual GPU read prevents immediate retirement");
    check(oracle.observe_completion(6).has_value(),"partial ticket completion");
    check(!oracle.retire(device),"ticket six must not retire ticket-seven user");
    check(oracle.observe_completion(7).has_value(),"completed ticket seven");
    check(oracle.retire(device).has_value(),"retire after stated completion");
    check(!oracle.read(device,g::Domain::gpu_compute,0,8) &&
          oracle.read(device,g::Domain::gpu_compute,0,8).error()
              ==m::MemoryError::already_retired,
          "retired allocation must reject all old reads");
    check(!oracle.allocate(device,8,16) &&
          oracle.allocate(device,8,16).error()
              ==m::MemoryError::duplicate_identity,
          "tombstone rejects old resource identity resurrection");
    // A future generation can be allocated, but it receives a different
    // allocation offset: this model does not assert safe physical aliasing.
    const auto next=key(g::ResourceKind::gpu_texture,
                        loaded->owner,static_cast<std::uint64_t>(loaded->slot),
                        loaded->generation+1U);
    const auto fresh=oracle.allocate(next,8,16);
    check(fresh && fresh->offset>=oracle.describe(device)->allocation.offset+8,
          "new generation must occupy a distinct virtual address region");
}
void memory_pattern_and_barrier_participation() {
    constexpr std::array<std::uint64_t,8> linear{
        0,4,8,12,16,20,24,28};
    constexpr std::array<std::uint64_t,8> strided{
        0,32,64,96,128,160,192,224};
    const auto a=m::segment_count(linear,32);
    const auto b=m::segment_count(strided,32);
    check(a && *a==1 && b && *b==8,
          "idealized 32-byte segments distinguish contiguous and strided accesses");
    check(!m::segment_count(linear,0) &&
          m::segment_count(linear,0).error()==m::PatternError::invalid_segment,
          "segment width cannot be zero");
    check(!m::segment_count(linear,24),
          "non-power-of-two hypothetical segment rejected by this model");

    constexpr std::array<Byte,8> unanimous{1,1,1,1,1,1,1,1};
    constexpr std::array<Byte,8> partial{1,1,0,0,0,0,0,0};
    constexpr std::array<Byte,8> absent{0,0,0,0,0,0,0,0};
    check(m::barrier_participation(unanimous)==m::Participation::everybody,
          "every intended lane reaches the required workgroup rendezvous");
    check(m::barrier_participation(partial)==m::Participation::divergent,
          "conditional barrier for a subset fails the participation contract");
    check(m::barrier_participation(absent)==m::Participation::nobody,
          "no barrier execution cannot synchronize two phases");
}
void host_cpp_release_acquire_handshake() {
    // Standard C++ host threads only! This does not model GPU shader atomics.
    //
    // Producer writes non-atomic payload, then publishes a release flag.
    // Consumer acquires the flag before reading payload. Its release ack
    // orders the producer's next write after the previous consumption.
    // No data race remains on the plain payload due to the two handshakes.
    std::atomic<int> flag{0};
    std::atomic<bool> valid{true};
    int payload=0;
    constexpr int count=512;
    std::thread producer([&]{
        for (int i=1;i<=count;++i) {
            while(flag.load(std::memory_order_acquire)!=0)
                std::this_thread::yield();
            payload=i;
            flag.store(1,std::memory_order_release);
        }
    });
    std::thread consumer([&]{
        for (int i=1;i<=count;++i) {
            while(flag.load(std::memory_order_acquire)!=1)
                std::this_thread::yield();
            if (payload!=i) valid.store(false,std::memory_order_relaxed);
            flag.store(0,std::memory_order_release);
        }
    });
    producer.join();
    consumer.join();
    check(valid.load(std::memory_order_relaxed) && payload==count,
          "host release/acquire message passing preserves all 512 payloads");
}
} // namespace

int main() {
    try {
        check_allocation_and_validation();
        real_texture_through_upload_plan();
        memory_pattern_and_barrier_participation();
        host_cpp_release_acquire_handshake();
        std::cout <<
            "NES-09M: virtual allocation, range initialization, "
            "explicit visibility, device retirement, lane patterns, "
            "barrier participation and host release/acquire passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "NES-09M FAILED: " << e.what() << '\n';
        return 1;
    }
}
