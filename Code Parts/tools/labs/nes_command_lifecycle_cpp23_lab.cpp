// AllMight Book 01 / Pass 12 (NES-12Q): actual NES source & frame ring,
// plus a CPU-only explicit command submission/failure/completion journal.
//
// These tests do not launch a GPU, call Metal/Vulkan or simulate device timing.
// All "accepted", "rejected", "completion" and "device lost" observations
// are manually supplied deterministic TEST FIXTURE facts, never hardware.
#include "nes_course/command_submission_lifecycle.hpp"
#include "nes_course/generational_asset_registry.hpp"
#include "nes_course/cpu_debug_session.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
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
namespace cm=allmight::nes::commands;
namespace dg=allmight::nes::work;
namespace fr=allmight::nes::frames;
using Ring=fr::FrameUploadRing<3>;
using Queue=cm::CommandSubmissionQueue<3>;

static_assert(!std::is_copy_constructible_v<cm::CommandStream>);
static_assert(std::is_nothrow_move_constructible_v<cm::CommandStream>);
static_assert(!std::is_copy_constructible_v<Ring::WriteLease>);
static_assert(!std::is_copy_constructible_v<Ring::ReadLease>);
static_assert(std::is_trivially_copyable_v<cm::CompletionEvent>);

void check(bool value,std::string_view why) {
    if(!value) throw std::runtime_error(std::string(why));
}
dg::Task make_task(std::uint32_t n,std::string_view name,
                   dg::Domain domain,
                   std::vector<dg::Access> accesses,
                   std::vector<dg::TaskId> deps={}) {
    return dg::Task{dg::TaskId{n},std::string(name),domain,1,
                    std::move(deps),std::move(accesses)};
}
dg::ResourceKey logical(dg::ResourceKind kind,std::uint64_t owner,
                        std::uint64_t slot,std::uint64_t generation) {
    return {kind,owner,slot,generation};
}
dg::Access r(dg::ResourceKey key){return {key,true,false};}
dg::Access w(dg::ResourceKey key){return {key,false,true};}

cm::CommandStream make_valid_stream(
    dg::ResourceKey texture=logical(dg::ResourceKind::gpu_texture,1,0,1),
    dg::ResourceKey output=logical(dg::ResourceKind::gpu_output,1,0,1),
    dg::ResourceKey overlay=logical(dg::ResourceKind::debug_overlay,1,0,1)) {
    cm::CommandStream stream;
    check(stream.begin().has_value(),"begin GPU command encoding");
    check(stream.record(make_task(10,"copy owned texture",
        dg::Domain::gpu_transfer,{w(texture)})).has_value(),
        "transfer intent can be recorded");
    check(stream.record(make_task(20,"compute debugger texture",
        dg::Domain::gpu_compute,{r(texture),w(output)})).has_value(),
        "compute intent can be recorded");
    check(stream.record(make_task(30,"draw debugger overlay",
        dg::Domain::gpu_graphics,{r(output),r(overlay)})).has_value(),
        "graphics intent can be recorded");
    check(stream.seal().has_value(),"complete GPU DAG can be sealed");
    check(stream.state()==cm::State::executable &&
          stream.command_count()==3 && stream.plan()!=nullptr &&
          stream.plan()->ordered.size()==3 &&
          stream.plan()->edges.size()>=2,
          "plan owns transfer->compute->graphics dependencies");
    return stream;
}
std::array<Byte,4> bytes{1,2,3,4};
void stage(Ring::WriteLease& writer) {
    check(writer.replace(bytes).has_value(),
          "writer stages an independently owned payload");
}
void test_encoder_errors_and_dag_rejection() {
    cm::CommandStream stream;
    const auto early=stream.seal();
    check(!early && early.error()==cm::CommandError::wrong_state,
          "cannot seal before recording");
    check(stream.begin().has_value(),"begin");
    const auto twice=stream.begin();
    check(!twice && twice.error()==cm::CommandError::wrong_state,
          "recording already begun");
    const auto empty=stream.seal();
    check(!empty && empty.error()==cm::CommandError::empty_recording,
          "empty command batch is not executable");
    auto cpu=make_task(1,"host should not become a GPU command",
                       dg::Domain::host,{});
    const auto reject_cpu=stream.record(std::move(cpu));
    check(!reject_cpu &&
          reject_cpu.error()==cm::CommandError::host_work_not_gpu_command,
          "CPU host callbacks must not be disguised as GPU commands");

    const auto a=logical(dg::ResourceKind::gpu_texture,3,5,1);
    check(stream.record(make_task(1,"first",dg::Domain::gpu_transfer,
                                  {w(a)})).has_value(),"valid first task");
    const auto repeated=stream.record(
        make_task(1,"duplicate",dg::Domain::gpu_transfer,{r(a)}));
    check(!repeated &&
          repeated.error()==cm::CommandError::graph_rejected &&
          stream.last_graph_failure() &&
          stream.last_graph_failure()->kind==dg::GraphError::duplicate_task &&
          stream.command_count()==1,
          "duplicate graph command refused with an exact diagnostic");
    const auto missing=stream.record(
        make_task(2,"unresolved",dg::Domain::gpu_compute,{r(a)},
                  {dg::TaskId{99}}));
    check(missing.has_value(),"forward dependency allowed while recording");
    const auto bad_seal=stream.seal();
    check(!bad_seal && bad_seal.error()==cm::CommandError::graph_rejected &&
          stream.last_graph_failure() &&
          stream.last_graph_failure()->kind==dg::GraphError::missing_dependency &&
          stream.state()==cm::State::recording &&
          stream.plan()==nullptr,
          "unresolved dependency fails seal atomically with exact cause");
    check(stream.abandon().has_value(),"unsubmitted batch may be abandoned");
    check(stream.state()==cm::State::initial &&
          stream.command_count()==0,"abandon clears recorded work");

    cm::CommandStream cycle;
    check(cycle.begin().has_value(),"cycle fixture recording");
    check(cycle.record(make_task(10,"A",dg::Domain::gpu_transfer,{},
                                 {dg::TaskId{20}})).has_value(),"A");
    check(cycle.record(make_task(20,"B",dg::Domain::gpu_compute,{},
                                 {dg::TaskId{10}})).has_value(),"B");
    check(!cycle.seal() && cycle.state()==cm::State::recording &&
          cycle.last_graph_failure() &&
          cycle.last_graph_failure()->kind==dg::GraphError::directed_cycle,
          "two-command directed cycle refused with exact graph cause");
    check(cycle.abandon().has_value(),"cycle can be safely abandoned");

    auto original=make_valid_stream();
    auto moved=std::move(original);
    const auto old=original.begin();
    check(!old && old.error()==cm::CommandError::moved_from,
          "moved-from command wrapper cannot begin");
    check(moved.state()==cm::State::executable &&
          moved.command_count()==3,"state moves without losing command body");
    const auto after=moved.record(
        make_task(40,"late",dg::Domain::gpu_graphics,{}));
    check(!after && after.error()==cm::CommandError::wrong_state,
          "sealed commands cannot be mutated");
}

void test_rejection_retry_and_exactly_once_completion() {
    Ring ring(64);
    Queue queue(ring);
    auto stream=make_valid_stream();
    auto writer=ring.begin();
    check(writer.has_value(),"frame writer");
    stage(*writer);
    const auto invalid=queue.submit(
        stream,*writer,{1,0,3},cm::SimulatedSubmit::accepted);
    check(!invalid &&
          invalid.error()==cm::CommandError::invalid_queue_stamp &&
          stream.state()==cm::State::executable &&
          writer->staged_size()==4 && ring.available()==2,
          "missing compute ticket refuses publication without draft loss");
    const auto rejected=queue.submit(
        stream,*writer,{1,2,3},
        cm::SimulatedSubmit::rejected_before_submit);
    check(!rejected &&
          rejected.error()==cm::CommandError::driver_rejected_before_submit &&
          stream.state()==cm::State::executable &&
          writer->staged_size()==4 && ring.available()==2 &&
          queue.outstanding()==0,
          "definite pre-submit rejection leaves command and draft retryable");
    const auto good=queue.submit(
        stream,*writer,{1,2,3},cm::SimulatedSubmit::accepted);
    check(good && good->ticket==1 &&
          good->state==cm::State::pending &&
          stream.state()==cm::State::pending && ring.available()==2,
          "acceptance publishes frame and locks command until retirement");
    const auto abort=stream.abandon();
    check(!abort && abort.error()==cm::CommandError::wrong_state,
          "recorded command cannot be abandoned after queue acceptance");
    const auto reset=stream.reset();
    check(!reset && reset.error()==cm::CommandError::wrong_state,
          "pending command cannot be reset");
    const auto early=queue.retire(good->ticket);
    check(!early && early.error()==cm::CommandError::not_completed,
          "accepted is not completed");
    check(queue.poll().has_value() && *queue.poll()==0,
          "no fabricated completion when no observed tickets exist");

    check(queue.observe_completion({1,1,3}).has_value(),
          "partial simulated progress is accepted");
    const auto no_event=queue.poll();
    check(no_event && *no_event==0 && queue.drain_events().empty(),
          "missing compute completion blocks event");
    check(queue.observe_completion({1,2,3}).has_value(),
          "full simulated completion observations supplied");
    const auto fired=queue.poll();
    check(fired && *fired==1 &&
          stream.state()==cm::State::completed &&
          queue.status(good->ticket)==cm::State::completed,
          "completed event recorded only after all required queues");
    const auto again=queue.poll();
    check(again && *again==0,
          "repeated poll must never duplicate completion event");
    auto events=queue.drain_events();
    check(events.size()==1 && events[0].ticket==good->ticket &&
          events[0].frame==good->frame &&
          events[0].observed==fr::CompletionStamp{1,2,3},
          "owned event identifies completed submission and exact frame");
    check(queue.drain_events().empty(),"drain consumes events once");

    {
        auto reader=ring.borrow(good->frame);
        check(reader && reader->view().size()==4 &&
              reader->view()[0]==1,
              "CPU frame reader pins same actual submitted payload");
        const auto pinned=queue.retire(good->ticket);
        check(!pinned && pinned.error()==cm::CommandError::pending_reader,
              "completion is insufficient while any CPU reader pins bytes");
    }
    check(queue.retire(good->ticket).has_value() &&
          stream.state()==cm::State::retired &&
          ring.available()==3,
          "release last reader then retire and recycle exactly one slot");
    check(!queue.retire(good->ticket),
          "double retirement rejected");
    check(stream.reset().has_value() && stream.state()==cm::State::initial,
          "only retired command stream can be reused for recording");
    check(queue.prune_retired()==1 && queue.outstanding()==0,
          "journal capacity can be recovered only for safely retired entries");
}

void test_unknown_submission_quarantine_and_confirmation() {
    Ring ring(64);
    Queue queue(ring);
    auto stream=make_valid_stream();
    auto writer=ring.begin();
    check(writer.has_value(),"ambiguous writer");
    stage(*writer);
    const auto uncertain=queue.submit(
        stream,*writer,{2,4,6},cm::SimulatedSubmit::indeterminate);
    check(uncertain && uncertain->state==cm::State::unknown &&
          stream.state()==cm::State::unknown,
          "an ambiguous outcome publishes quarantine, never rollback");
    check(!stream.abandon() && !stream.reset(),
          "unknown submission cannot be cancelled or reset");
    check(queue.observe_completion({2,4,6}).has_value(),
          "test can advance independent completion observations");
    const auto not_proven=queue.poll();
    check(not_proven && *not_proven==0 &&
          queue.drain_events().empty(),
          "unknown acceptance suppresses fabricated completion notices");
    const auto not_retired=queue.retire(uncertain->ticket);
    check(!not_retired &&
          not_retired.error()==cm::CommandError::submission_indeterminate,
          "unknown outcomes cannot silently release resources");
    check(ring.available()==2,
          "ambiguous submission continues to occupy one frame slot");
    check(queue.confirm_accepted(uncertain->ticket).has_value(),
          "only explicit independent acceptance evidence unblocks state");
    const auto fired=queue.poll();
    check(fired && *fired==1 &&
          queue.drain_events().size()==1 &&
          queue.retire(uncertain->ticket).has_value(),
          "confirmed accepted submission can complete then retire");

    Ring second(64);
    Queue lost(second);
    auto next=make_valid_stream();
    auto draft=second.begin();
    check(draft.has_value(),"device-loss draft");
    stage(*draft);
    const auto unknown=lost.submit(
        next,*draft,{1,1,1},cm::SimulatedSubmit::indeterminate);
    check(unknown.has_value(),"second ambiguous submission");
    lost.declare_device_lost();
    check(lost.lost() && lost.outstanding()==1,
          "device lost keeps one resource quarantined");
    check(!lost.confirm_accepted(unknown->ticket) &&
          !lost.observe_completion({1,1,1}) &&
          !lost.poll() && !lost.retire(unknown->ticket),
          "device loss refuses unverified acceptance, events and retirement");
    auto later=make_valid_stream();
    auto another=second.begin();
    check(another.has_value(),"pool still has a separate available slot");
    stage(*another);
    const auto refuse=lost.submit(
        later,*another,{2,2,2},cm::SimulatedSubmit::accepted);
    check(!refuse && refuse.error()==cm::CommandError::device_lost &&
          later.state()==cm::State::executable,
          "device-lost queue refuses further submission before publication");
    // Real backend must use a separate verified device-idle/reset protocol.
    // This test deliberately does NOT fake freeing in-flight GPU storage.
}

void test_foreign_frame_writer_refused_before_publication() {
    Ring correct(64),foreign(64);
    Queue queue(correct);
    auto stream=make_valid_stream();
    auto writer=foreign.begin();
    check(writer.has_value(),"foreign ring provides its own writer");
    stage(*writer);
    const auto bad=queue.submit(
        stream,*writer,{1,2,3},cm::SimulatedSubmit::accepted);
    check(!bad &&
          bad.error()==cm::CommandError::foreign_frame_writer &&
          stream.state()==cm::State::executable &&
          writer->staged_size()==4 && foreign.available()==2 &&
          correct.available()==3 && queue.outstanding()==0,
          "queue cannot publish a frame it does not own or retire");
    // Drop foreign writer: RAII will cancel without consuming queue slots.
}

void test_bounded_journal_and_command_wrapper_lifetime() {
    Ring ring(64);
    Queue queue(ring,1);
    std::uint64_t receipt_id{};
    fr::FrameHandle frame{};
    {
        auto stream=std::make_unique<cm::CommandStream>(make_valid_stream());
        auto writer=ring.begin();
        check(writer.has_value(),"owning frame writer");
        stage(*writer);
        const auto receipt=queue.submit(
            *stream,*writer,{1,2,3},cm::SimulatedSubmit::accepted);
        check(receipt.has_value(),"accepted journal entry");
        receipt_id=receipt->ticket;
        frame=receipt->frame;
    } // Wrapper destroyed, queue still owns command Control.
    check(queue.status(receipt_id)==cm::State::pending &&
          ring.borrow(frame).has_value(),
          "command journal and frame survive destruction of user wrapper");
    auto later=make_valid_stream();
    auto writer=ring.begin();
    check(writer.has_value(),"second frame free");
    stage(*writer);
    const auto full=queue.submit(
        later,*writer,{2,3,4},cm::SimulatedSubmit::accepted);
    check(!full && full.error()==cm::CommandError::capacity_exhausted &&
          later.state()==cm::State::executable,
          "bounded journal refuses extra accepted work before publication");
    check(queue.observe_completion({1,2,3}).has_value() &&
          queue.poll().has_value() &&
          queue.retire(receipt_id).has_value(),
          "journal retires first work even without original wrapper");
    check(queue.prune_retired()==1,"only retired journal record can be pruned");
    const auto again=queue.submit(
        later,*writer,{2,3,4},cm::SimulatedSubmit::accepted);
    check(again && again->ticket==2 && again->frame!=frame,
          "next ticket monotonic; no accidental reuse of old submission ID");
    check(queue.observe_completion({2,3,4}).has_value() &&
          queue.poll().has_value() &&
          queue.retire(again->ticket).has_value(),
          "second command also completes and retires correctly");
}

std::vector<Byte> texture_image() {
    std::vector<Byte> data(20,0);
    data[0]='T';data[1]='X';data[2]='R';data[3]='4';
    data[4]=2;data[6]=1;
    constexpr std::array<Byte,8> pixels{
        0x12,0x34,0x56,0xFF,0xAB,0xCD,0xEF,0x44
    };
    for(std::size_t i=0;i<pixels.size();++i)
        data[12+i]=pixels[i];
    return data;
}
std::vector<Byte> rom_image() {
    std::vector<Byte> rom(header_bytes+prg_bank_bytes,0);
    rom[0]='N';rom[1]='E';rom[2]='S';rom[3]=0x1A;rom[4]=1;
    rom[header_bytes]=0xA9;rom[header_bytes+1]=0x7F;
    rom[header_bytes+2]=0x85;rom[header_bytes+3]=0x02;
    rom[header_bytes+0x3FFCU]=0;rom[header_bytes+0x3FFDU]=0x80;
    return rom;
}
void test_actual_nes_texture_cpu_and_submission_contract() {
    GenerationalAssetRegistry registry(2);
    auto tex=texture_image();
    auto imported=registry.import<TextureTag>(tex);
    check(imported.has_value(),"actual Pass05 TXR4 decoder");
    const auto lookup=registry.lookup(*imported);
    check(lookup && lookup->get().pixels.size()==8,
          "actual Pass06 texture handle and owned pixels");
    const std::vector<Byte> pixels=lookup->get().pixels;
    auto rom=rom_image();
    auto decoded=NromCartridge::from_image(rom);
    check(decoded.has_value(),"actual NROM bytes and reset vector");
    CpuBus bus;
    check(bus.insert(std::move(*decoded)).has_value(),
          "same prior NROM CPU memory bus");
    CpuDebugSession cpu(bus);
    check(cpu.reset().has_value() && cpu.step().has_value() &&
          cpu.step().has_value(),"execute LDA #$7F and STA $02 on real CPU");
    const auto state=cpu.snapshot();
    check(state && state->ready && state->a==0x7F &&
          state->pc==0x8004 && state->total_cycles==5 &&
          cpu.pending_traces()==2,
          "Pass07 real CPU snapshot and trace are retained");

    // An explicit, portable 24-byte serialized payload: NESF, frame,
    // A, PC little-endian, cycle-counter LE64 and eight RGBA8 pixels.
    std::vector<Byte> overlay;
    overlay.reserve(24);
    overlay.insert(overlay.end(),{'N','E','S','F',0,state->a,
        static_cast<Byte>(state->pc&0xFFU),
        static_cast<Byte>((state->pc>>8U)&0xFFU)});
    for(unsigned i=0;i<8;++i)
        overlay.push_back(static_cast<Byte>(
            (state->total_cycles>>(i*8U))&0xFFU));
    overlay.insert(overlay.end(),pixels.begin(),pixels.end());
    check(overlay.size()==24 && overlay[5]==0x7F &&
          overlay[16]==0x12 && overlay[23]==0x44,
          "actual owned NES/texture data serialized without struct padding");

    const auto device=logical(dg::ResourceKind::gpu_texture,
        imported->owner,static_cast<std::uint64_t>(imported->slot),
        imported->generation);
    const auto result=logical(dg::ResourceKind::gpu_output,
        imported->owner,static_cast<std::uint64_t>(imported->slot),
        imported->generation);
    const auto debugger=logical(dg::ResourceKind::debug_overlay,
        imported->owner,static_cast<std::uint64_t>(imported->slot),
        imported->generation);
    auto stream=make_valid_stream(device,result,debugger);
    Ring ring(128);
    Queue queue(ring);
    auto writer=ring.begin();
    check(writer && writer->replace(overlay).has_value(),
          "exact NES overlay staged in guarded upload ring");
    const auto receipt=queue.submit(
        stream,*writer,{7,8,9},cm::SimulatedSubmit::accepted);
    check(receipt.has_value(),"conceptual transfer/compute/graphics publication");
    auto read=ring.borrow(receipt->frame);
    check(read && read->view().size()==24 &&
          read->view()[6]==0x04 && read->view()[7]==0x80 &&
          read->view()[16]==0x12 && read->view()[23]==0x44,
          "submitted payload matches original CPU and image source");
    overlay.assign(24,0); tex.clear(); rom.clear();
    check(registry.erase(*imported).has_value(),
          "separately owned asset may be invalidated after staging");
    check(!registry.lookup(*imported) && read->view()[16]==0x12,
          "asset registry erasure does not mutate already staged owned bytes");

    check(queue.observe_completion({7,8,9}).has_value() &&
          queue.poll() && queue.drain_events().size()==1,
          "manual test fixture completion produces one owned event");
    const auto still=queue.retire(receipt->ticket);
    check(!still && still.error()==cm::CommandError::pending_reader,
          "device completion does not override held CPU diagnostic borrow");
    read=Ring::ReadLease{};
    check(queue.retire(receipt->ticket).has_value() &&
          ring.available()==3,
          "all queue stamps and CPU pins satisfied before recycling frame");
}
} // namespace

int main() {
    try {
        test_encoder_errors_and_dag_rejection();
        test_rejection_retry_and_exactly_once_completion();
        test_unknown_submission_quarantine_and_confirmation();
        test_foreign_frame_writer_refused_before_publication();
        test_bounded_journal_and_command_wrapper_lifetime();
        test_actual_nes_texture_cpu_and_submission_contract();
        std::cout<<"NES-12Q: command encode/seal, DAG hazards, definite "
                   "rejection/retry, uncertain-submission quarantine, "
                   "completion notifications, retained command ownership, "
                   "device loss, bounded journal and real NES overlay passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr<<"NES-12Q FAILED: "<<error.what()<<'\n';
        return 1;
    }
}
