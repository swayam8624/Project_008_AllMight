// AllMight Book 01 — Pass 11, NES-11B.
// CPU-only synthetic integration with real NROM, Pass05/06 asset registry
// and Pass07 separately compiled CPU debugger. No native Vulkan/Metal work.
#include "nes_course/scoped_frame_upload_ring.hpp"
#include "nes_course/generational_asset_registry.hpp"
#include "nes_course/cpu_debug_session.hpp"
#include <array>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using namespace allmight::nes;
using namespace allmight::nes::assets;
using namespace allmight::nes::cpu;
namespace fr=allmight::nes::frames;
using Ring=fr::FrameUploadRing<3>;

static_assert(!std::is_convertible_v<int**,const int**>);
static_assert(std::is_convertible_v<int**,const int* const*>);
// const applies to the span wrapper and its ELEMENTS independently.
static_assert(std::is_assignable_v<
    decltype(std::declval<const std::span<Byte>&>()[0]),Byte>);
static_assert(!std::is_assignable_v<
    decltype(std::declval<std::span<const Byte>&>()[0]),Byte>);
static_assert(!std::is_assignable_v<
    decltype(std::declval<const Ring::ReadLease&>().view()[0]),Byte>);
static_assert(std::is_nothrow_move_constructible_v<Ring::WriteLease>);
static_assert(std::is_nothrow_move_constructible_v<Ring::ReadLease>);
static_assert(!std::is_copy_constructible_v<Ring::ReadLease>);
static_assert(!std::is_copy_constructible_v<Ring::WriteLease>);

void check(bool yes,std::string_view message) {
    if(!yes) throw std::runtime_error(std::string(message));
}
std::vector<Byte> texture() {
    std::vector<Byte> data(20,0);
    data[0]='T';data[1]='X';data[2]='R';data[3]='4';
    data[4]=2;data[6]=1;
    constexpr std::array<Byte,8> pixels{
        0x12,0x34,0x56,0xFF,0xAB,0xCD,0xEF,0x44};
    for(std::size_t i=0;i<pixels.size();++i) data[12+i]=pixels[i];
    return data;
}
std::vector<Byte> cartridge() {
    std::vector<Byte> rom(header_bytes+prg_bank_bytes,0);
    rom[0]='N';rom[1]='E';rom[2]='S';rom[3]=0x1A;rom[4]=1;
    rom[header_bytes]=0xA9;rom[header_bytes+1]=0x7F; // LDA #$7F
    rom[header_bytes+2]=0x85;rom[header_bytes+3]=0x02; // STA $02
    rom[header_bytes+0x3FFCU]=0;rom[header_bytes+0x3FFDU]=0x80;
    return rom;
}
std::vector<Byte> overlay(const CpuStateSnapshot& state,
                          std::span<const Byte> pixels,Byte frame) {
    check(state.ready,"only initialized CPU states can be serialized");
    std::vector<Byte> bytes;
    bytes.reserve(16+pixels.size());
    bytes.push_back('N');bytes.push_back('E');
    bytes.push_back('S');bytes.push_back('F');
    bytes.push_back(frame);bytes.push_back(state.a);
    bytes.push_back(static_cast<Byte>(state.pc&0xFFU));
    bytes.push_back(static_cast<Byte>((state.pc>>8U)&0xFFU));
    // Define the wire format byte by byte, never memcpy a padded struct.
    for(unsigned i=0;i<8;++i)
        bytes.push_back(static_cast<Byte>(
            (state.total_cycles>>(i*8U))&0xFFU));
    bytes.insert(bytes.end(),pixels.begin(),pixels.end());
    return bytes;
}
struct Sources {
    GenerationalAssetRegistry registry{2};
    AssetHandle<TextureTag> texture_handle{};
    std::vector<Byte> pixels;
    CpuBus bus;
    std::optional<CpuDebugSession> cpu;
    CpuStateSnapshot snapshot{};

    Sources() {
        const auto raw=texture();
        const auto imported=registry.import<TextureTag>(raw);
        check(imported.has_value(),"real TXR4 decoder");
        texture_handle=*imported;
        const auto resolved=registry.lookup(texture_handle);
        check(resolved.has_value() && resolved->get().pixels.size()==8,
              "actual generation-aware typed asset lookup");
        pixels=resolved->get().pixels; // an owned copy, no escaped borrow
        auto rom=cartridge();
        auto loaded=NromCartridge::from_image(rom);
        check(loaded.has_value(),"NROM fixture valid");
        check(bus.insert(std::move(*loaded)).has_value(),"NROM bus mounted");
        cpu.emplace(bus);
        check(cpu->reset().has_value(),"NROM reset vector");
        check(cpu->step().has_value(),"LDA #$7F");
        check(cpu->step().has_value(),"STA $02");
        const auto current=cpu->snapshot();
        check(current.has_value() && current->ready &&
              current->a==0x7F && current->pc==0x8004 &&
              current->total_cycles==5 && cpu->pending_traces()==2,
              "real CPU debugger's actual resulting state");
        snapshot=*current;
    }
};

void three_slots_require_all_queues_and_all_readers() {
    Sources sources;
    Ring ring(64);
    std::array<fr::FrameHandle,3> handles{};
    const std::array<fr::CompletionStamp,3> required{{
        {1,2,4},{2,0,5},{3,6,0}
    }};
    for(std::size_t i=0;i<3;++i) {
        auto writer=ring.begin();
        check(writer.has_value(),"frame writer acquired");
        auto bytes=overlay(sources.snapshot,sources.pixels,
                           static_cast<Byte>(i));
        check(bytes.size()==24,"16-byte header and 8 owned texture bytes");
        check(writer->replace(bytes).has_value(),"staged owned CPU bytes");
        const auto submitted=writer->submit(required[i]);
        check(submitted.has_value(),"multi-queue completion stamp attached");
        handles[i]=*submitted;
        bytes.assign(bytes.size(),0); // source changes do not affect slot
    }
    check(ring.available()==0 && !ring.begin(),
          "three live frames prevent premature modulo reuse");
    check(handles[0].slot==0 && handles[1].slot==1 &&
          handles[2].slot==2,"deterministic slots for first 3 frames");

    auto read=ring.borrow(handles[0]);
    check(read.has_value(),"submitted frame permits pinned immutable borrow");
    const auto view=read->view();
    check(view.size()==24 && view[0]=='N' && view[3]=='F' &&
          view[5]==0x7F && view[6]==0x04 && view[7]==0x80 &&
          view[16]==0x12 && view[23]==0x44,
          "snapshot is explicitly serialized from actual CPU+texture");
    const auto independent=read->copy_snapshot();
    check(independent==std::vector<Byte>(view.begin(),view.end()),
          "owned copy equals pinned span without sharing its lifetime");

    check(ring.observe_completion({3,1,4}).has_value(),
          "queue counters may advance independently");
    const auto too_early=ring.retire(handles[0]);
    check(!too_early && too_early.error()==fr::FrameError::completion_pending,
          "graphics and transfer completion cannot hide missing compute");
    check(!ring.retire(handles[1]) && !ring.retire(handles[2]),
          "remaining slots also have uncompleted queue requirements");
    const auto regression=ring.observe_completion({2,1,4});
    check(!regression &&
          regression.error()==fr::FrameError::completion_regressed,
          "per-queue monotonicity is enforced");
    check(ring.completed()==fr::CompletionStamp{3,1,4},
          "rejected regression leaves completion record unchanged");

    check(ring.observe_completion({3,6,5}).has_value(),
          "all required independent queue values reached");
    const auto pinned=ring.retire(handles[0]);
    check(!pinned && pinned.error()==fr::FrameError::readers_pinned,
          "GPU completion does not cancel an outstanding CPU read pin");
    check(ring.available()==0,"read lease keeps frame storage pinned");
    // The view must not be accessed after the lease is released.
    read=Ring::ReadLease{};
    check(ring.retire(handles[0]).has_value(),
          "zero pins AND completed GPU stamps permit reuse");
    check(ring.available()==1,"retirement frees exactly one frame");

    auto next=ring.begin();
    check(next.has_value(),"frame four may reuse completed frame zero");
    check(next->replace(independent).has_value(),
          "owned snapshot survives the old borrow's release");
    const auto new_id=next->submit({4,7,6});
    check(new_id && new_id->slot==handles[0].slot &&
          new_id->generation==handles[0].generation+1U,
          "reuse increments generation to prevent ABA");
    const auto stale=ring.borrow(handles[0]);
    check(!stale && stale.error()==fr::FrameError::stale_handle,
          "old frame generation no longer resolves");
    check(ring.retire(handles[1]).has_value() &&
          ring.retire(handles[2]).has_value(),
          "independent other slots can retire after their own completion");
    check(!ring.retire(*new_id),"new frame not yet complete");
    check(ring.observe_completion({4,7,6}).has_value() &&
          ring.retire(*new_id).has_value(),"frame four also retires safely");
    check(independent[0]=='N' && independent[23]==0x44,
          "owned data independent of all slot epochs");

    check(sources.registry.erase(sources.texture_handle).has_value(),
          "texture source can later be erased independently");
    check(!sources.registry.lookup(sources.texture_handle),
          "old asset handle stale in different owner's namespace");
}
void failed_writer_is_rolled_back() {
    Ring ring(12);
    const std::array<Byte,4> bytes{4,5,6,7};
    const std::array<Byte,13> huge{};
    {
        auto writer=ring.begin();
        check(writer.has_value() && writer->replace(bytes).has_value(),
              "initial candidate valid");
        const auto refused=writer->replace(huge);
        check(!refused && refused.error()==fr::FrameError::oversized_payload,
              "large candidate rejected");
        check(writer->staged_size()==4,
              "failed replacement preserves earlier candidate");
        auto moved=std::move(*writer);
        const auto used=writer->replace(bytes);
        check(!used && used.error()==fr::FrameError::no_writer,
              "moved-from writer cannot mutate slot");
        check(moved.staged_size()==4,"move retains staged bytes");
        // Abandon without submission. Only the active moved writer cancels.
    }
    check(ring.available()==3,
          "last writer destructor cancels and releases the slot");

    auto writer=ring.begin();
    check(writer.has_value(),"slot free after rollback");
    const auto invalid=writer->submit({1,0,0});
    check(!invalid && invalid.error()==fr::FrameError::empty_payload,
          "empty payload cannot be submitted");
    const std::array<Byte,2> two{1,2};
    check(writer->replace(two).has_value(),"small candidate accepted");
    const auto submitted=writer->submit({1,0,0});
    check(submitted.has_value(),"single-queue submit accepted");
    auto reader=ring.borrow(*submitted);
    check(reader && reader->view().size()==2 &&
          reader->view()[0]==1 && reader->view()[1]==2,
          "immutable read sees the accepted copy");
    reader=Ring::ReadLease{};
    check(ring.observe_completion({1,0,0}).has_value() &&
          ring.retire(*submitted).has_value(),
          "single queue completion and zero pins enable recycling");
}
void identity_and_lifetime_are_distinct() {
    fr::FrameUploadRing<1> left(16),right(16);
    auto writer=left.begin();
    const std::array<Byte,1> byte{42};
    check(writer && writer->replace(byte).has_value(),"left draft");
    const auto id=writer->submit({1,0,0});
    check(id.has_value(),"left submission");
    const auto foreign=right.borrow(*id);
    check(!foreign && foreign.error()==fr::FrameError::wrong_owner,
          "other pool cannot accept identical slot number");
    const auto forged=left.borrow({id->owner,999,id->generation});
    check(!forged && forged.error()==fr::FrameError::invalid_slot,
          "fabricated out-of-range frame rejected");
    const auto mismatched=left.borrow({id->owner,id->slot,id->generation+1U});
    check(!mismatched && mismatched.error()==fr::FrameError::stale_handle,
          "fabricated generation rejected");

    fr::FrameUploadRing<1>::ReadLease surviving;
    {
        fr::FrameUploadRing<1> local(16);
        auto own=local.begin();
        const std::array<Byte,3> value{9,8,7};
        check(own && own->replace(value).has_value(),"local frame");
        const auto local_id=own->submit({1,0,0});
        check(local_id.has_value(),"local submission");
        auto borrowed=local.borrow(*local_id);
        check(borrowed.has_value(),"local read pin");
        surviving=std::move(*borrowed);
    }
    // This control survives because the lease holds shared ownership.
    check(surviving.active() && surviving.view().size()==3 &&
          surviving.view()[0]==9 && surviving.view()[2]==7,
          "borrow lease outlives the pool's wrapper safely");
    surviving=fr::FrameUploadRing<1>::ReadLease{};

    fr::FrameUploadRing<1> no_wrap(
        16,std::numeric_limits<std::uint64_t>::max());
    auto last_writer=no_wrap.begin();
    check(last_writer && last_writer->replace(byte).has_value(),
          "last-generation writer");
    const auto last=last_writer->submit({1,0,0});
    check(last && last->generation==std::numeric_limits<std::uint64_t>::max(),
          "near-overflow fixture configured");
    check(no_wrap.observe_completion({1,0,0}).has_value() &&
          no_wrap.retire(*last).has_value(),"last generation retires");
    check(no_wrap.available()==0 && !no_wrap.begin(),
          "permanently retire slot before generation wraps");
}
void move_pool_and_keep_swapchain_index_distinct() {
    Ring before(16);
    const auto owner=before.owner_id();
    Ring after(std::move(before));
    check(before.owner_id()==0 && after.owner_id()==owner,
          "moving wrapper transfers stable control identity");
    const auto moved=before.begin();
    check(!moved && moved.error()==fr::FrameError::moved_from_pool,
          "moved-from wrapper rejects new work");
    auto writer=after.begin();
    const std::array<Byte,1> value{23};
    check(writer && writer->replace(value).has_value(),
          "moved pool still stages data");
    const auto id=writer->submit({1,0,0});
    check(id && id->owner==owner,"stable logical owner after move");
    check(after.observe_completion({1,0,0}).has_value() &&
          after.retire(*id).has_value(),"moved ring remains functional");

    // Frame slot 0/1/2 and independently acquired swapchain image 2/0/1
    // belong to different namespaces. No matching-index assumption.
    constexpr std::array<std::size_t,4> frame_slots{0,1,2,0};
    constexpr std::array<std::size_t,4> acquired{2,0,1,2};
    check(frame_slots[0]!=acquired[0] &&
          frame_slots[1]!=acquired[1] &&
          frame_slots[3]!=acquired[3],
          "presentation image identity differs from frame slot identity");
}
} // namespace

int main() {
    try {
        three_slots_require_all_queues_and_all_readers();
        failed_writer_is_rolled_back();
        identity_and_lifetime_are_distinct();
        move_pool_and_keep_swapchain_index_distinct();
        std::cout<<"NES-11B: NROM+asset+CPU owned overlay, const views, "
                   "writer rollback, pinned borrows, queue stamps, "
                   "stale generations and detached pool lifetimes passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr<<"NES-11B FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
