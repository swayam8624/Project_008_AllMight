// Project AllMight Book 1 / Pass 08 — C++23 dependency and render-plan lab.
// Uses actual NES-01/02 NROM bus, Pass 05 typed asset import, Pass 06
// generational handles, and Pass 07 separately compiled CPU debugger.
//
// There is NO Vulkan/Metal API here. "GPU" tasks only emit a CPU-side plan;
// no physical GPU execution, memory barrier or completion fence is simulated.
// All ROMs, textures and task graphs are synthetic fixtures.
#include "nes_course/work_dependency_graph.hpp"
#include "nes_course/generational_asset_registry.hpp"
#include "nes_course/cpu_debug_session.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using namespace allmight::nes;
using namespace allmight::nes::assets;
using namespace allmight::nes::cpu;
namespace g = allmight::nes::work;

void check(bool valid, std::string_view reason) {
    if (!valid) throw std::runtime_error(std::string(reason));
}
constexpr g::ResourceKey cpu_state{
    g::ResourceKind::cpu_state,0,0,1};
constexpr g::ResourceKey cpu_program{
    g::ResourceKind::cpu_program,0,0,1};
constexpr g::ResourceKey cpu_ram{
    g::ResourceKind::cpu_ram,0,0,1};
constexpr g::ResourceKey instruction_trace{
    g::ResourceKind::instruction_trace,0,0,1};
constexpr g::ResourceKey debug_overlay{
    g::ResourceKind::debug_overlay,0,0,1};
constexpr g::ResourceKey gpu_texture{
    g::ResourceKind::gpu_texture,0,0,1};
constexpr g::ResourceKey gpu_output{
    g::ResourceKind::gpu_output,0,0,1};
constexpr g::Access read(g::ResourceKey r) noexcept {return {r,true,false};}
constexpr g::Access write(g::ResourceKey r) noexcept {return {r,false,true};}
constexpr g::Access update(g::ResourceKey r) noexcept {return {r,true,true};}
constexpr g::TaskId task(std::uint32_t n) noexcept {return {n};}

template<class Tag>
g::ResourceKey key(AssetHandle<Tag> h,g::ResourceKind kind) {
    return {kind,h.owner,static_cast<std::uint64_t>(h.slot),h.generation};
}
std::vector<Byte> rom_fixture() {
    std::vector<Byte> bytes(header_bytes + prg_bank_bytes,0);
    bytes[0]='N';bytes[1]='E';bytes[2]='S';bytes[3]=0x1A;
    bytes[4]=1;
    bytes[header_bytes + 0] = 0xA9; // LDA #$7F
    bytes[header_bytes + 1] = 0x7F;
    bytes[header_bytes + 2] = 0x85; // STA $01
    bytes[header_bytes + 3] = 0x01;
    bytes[header_bytes + 4] = 0xEA; // NOP, optional future step
    bytes[header_bytes + 0x3FFCU] = 0x00; // reset vector $8000
    bytes[header_bytes + 0x3FFDU] = 0x80;
    return bytes;
}
std::vector<Byte> texture_fixture() {
    std::vector<Byte> bytes(16,0); // TXR4 header 12 + 1 RGBA8 pixel
    bytes[0]='T';bytes[1]='X';bytes[2]='R';bytes[3]='4';
    bytes[4]=1;bytes[6]=1;
    bytes[12]=0x12;bytes[13]=0x34;bytes[14]=0x56;bytes[15]=0xFF;
    return bytes;
}
void must_add(g::DependencyGraph& graph,g::Task item) {
    const auto result=graph.add(std::move(item));
    check(result.has_value(),"valid task submission rejected");
}
const g::Edge* edge(const g::Plan& plan,std::uint32_t from,
                    std::uint32_t to) {
    const auto it=std::find_if(plan.edges.begin(),plan.edges.end(),
        [from,to](const g::Edge& e){
            return e.from.value==from && e.to.value==to;
        });
    return it==plan.edges.end()?nullptr:&*it;
}
void verify_order(const g::Plan& plan) {
    // Independent topological order oracle: every edge points forward in
    // emitted sequence. Check each emitted task is unique as well.
    for (std::size_t i=0;i<plan.ordered.size();++i) {
        for (std::size_t j=0;j<i;++j)
            check(plan.ordered[i].id!=plan.ordered[j].id,
                  "duplicate task in topological plan");
    }
    for (const auto& dependency:plan.edges) {
        auto a=std::find_if(plan.ordered.begin(),plan.ordered.end(),
             [&](const g::PlannedTask& x){return x.id==dependency.from;});
        auto b=std::find_if(plan.ordered.begin(),plan.ordered.end(),
             [&](const g::PlannedTask& x){return x.id==dependency.to;});
        check(a!=plan.ordered.end() && b!=plan.ordered.end() && a<b,
              "dependency not satisfied in returned topological order");
        check(a->layer<b->layer &&
              a->earliest_finish<=b->earliest_start,
              "layer/longest-path invariant violated");
    }
}

void hazard_reason_and_independent_read_test() {
    g::DependencyGraph graph;
    const auto resource=g::ResourceKey{g::ResourceKind::texture,11,4,3};
    must_add(graph,{task(10),"write initial",g::Domain::host,2,
                    {},{write(resource)}});
    must_add(graph,{task(20),"reader A",g::Domain::host,3,
                    {},{read(resource)}});
    must_add(graph,{task(30),"reader B",g::Domain::host,4,
                    {},{read(resource)}});
    must_add(graph,{task(40),"overwrite",g::Domain::host,5,
                    {task(20)},{write(resource)}});
    const auto plan=graph.plan();
    check(plan.has_value() && plan->ordered.size()==4,
          "four-task hazard graph should be acyclic");
    verify_order(*plan);
    check(plan->critical_path_cost==11,
          "critical path should be write(2)->reader B(4)->overwrite(5)");
    check(edge(*plan,10,20) &&
          g::has_reason(*edge(*plan,10,20),g::read_after_write),
          "RAW edge missing");
    check(edge(*plan,10,30) &&
          g::has_reason(*edge(*plan,10,30),g::read_after_write),
          "second RAW edge missing");
    check(edge(*plan,10,40) &&
          g::has_reason(*edge(*plan,10,40),g::write_after_write),
          "WAW edge missing");
    check(edge(*plan,20,40) &&
          g::has_reason(*edge(*plan,20,40),g::write_after_read) &&
          g::has_reason(*edge(*plan,20,40),g::explicit_dependency),
          "explicit and WAR reasons should MERGE on one edge");
    check(edge(*plan,30,40) &&
          g::has_reason(*edge(*plan,30,40),g::write_after_read),
          "second WAR edge missing");
    check(!edge(*plan,20,30) && !edge(*plan,30,20),
          "two reads must not create artificial ordering");
    check(plan->gpu_edges_requiring_backend_review.empty(),
          "pure host graph must have no GPU synchronization annotations");
}

void failures_and_determinism() {
    {
        g::DependencyGraph g1;
        must_add(g1,{task(99),"late id first",g::Domain::host,1,{},{}});
        must_add(g1,{task(3),"small id second",g::Domain::host,1,{},{}});
        const auto p=g1.plan();
        check(p && p->ordered[0].id==task(3) &&
              p->ordered[1].id==task(99),
              "equal-readiness tasks must be ordered by numerical task ID");
        const auto p2=g1.plan();
        check(p2 && p2->ordered==p->ordered,
              "identical graph must yield identical plan");
        const auto duplicate=g1.add({task(3),"duplicate",g::Domain::host,1,{},{}});
        check(!duplicate && duplicate.error().kind==g::GraphError::duplicate_task &&
              g1.size()==2, "duplicate submission must preserve graph");
        const auto empty=g1.add({task(20),"",g::Domain::host,1,{},{}});
        check(!empty && empty.error().kind==g::GraphError::invalid_task,
              "empty task name invalid");
        const auto zero=g1.add({task(20),"zero",g::Domain::host,0,{},{}});
        check(!zero && zero.error().kind==g::GraphError::invalid_task,
              "zero estimate invalid");
        const auto no_access=g1.add({task(20),"broken",g::Domain::host,1,{},
            {{cpu_ram,false,false}}});
        check(!no_access && no_access.error().kind==g::GraphError::invalid_access,
              "access must specify read and/or write");
        const auto duplicate_resource=g1.add({task(20),"twice",g::Domain::host,1,{},
             {read(cpu_ram),write(cpu_ram)}});
        check(!duplicate_resource &&
              duplicate_resource.error().kind==g::GraphError::duplicate_resource,
              "per-task resource declarations may not duplicate a key");
        check(g1.size()==2, "semantic submission failure must not mutate graph");
    }
    {
        g::DependencyGraph missing;
        must_add(missing,{task(7),"missing predecessor",g::Domain::host,1,
                          {task(42)},{}});
        const auto p=missing.plan();
        check(!p && p.error().kind==g::GraphError::missing_dependency &&
              p.error().related==task(42), "forward reference must be resolved");
        check(missing.size()==1, "failed plan must not mutate source tasks");
    }
    {
        g::DependencyGraph cycle;
        must_add(cycle,{task(10),"A",g::Domain::host,1,{task(20)},{}});
        must_add(cycle,{task(20),"B",g::Domain::host,1,{task(10)},{}});
        const auto p=cycle.plan();
        check(!p && p.error().kind==g::GraphError::directed_cycle,
              "mutual dependence is a directed cycle");
        const auto p2=cycle.plan();
        check(!p2 && p2.error().kind==g::GraphError::directed_cycle &&
              cycle.size()==2, "cycle rejection cannot publish partial plan");
    }
    {
        g::DependencyGraph overflow;
        must_add(overflow,{task(1),"near max",g::Domain::host,
                           std::numeric_limits<std::uint64_t>::max(),{},{}});
        must_add(overflow,{task(2),"beyond max",g::Domain::host,1,
                           {task(1)},{}});
        const auto p=overflow.plan();
        check(!p && p.error().kind==g::GraphError::critical_path_overflow,
              "estimated critical-path time must not wrap");
    }
}

void generation_key_independence() {
    g::DependencyGraph graph;
    const g::ResourceKey old{g::ResourceKind::texture,5,1,7};
    const g::ResourceKey newer{g::ResourceKind::texture,5,1,8};
    must_add(graph,{task(1),"old occupant write",g::Domain::host,1,{}, {write(old)}});
    must_add(graph,{task(2),"new occupant write",g::Domain::host,1,{}, {write(newer)}});
    const auto p=graph.plan();
    check(p && p->edges.empty(),
          "distinct generations must not be silently treated as same asset");
    // Important boundary: an old handle may be stale even when a plan uses
    // it! Execution must validate the actual registry generation separately.
    // Physical allocation aliasing needs a distinct device allocation key.
}

struct Demo {
    GenerationalAssetRegistry registry{8};
    std::vector<Byte> rom=rom_fixture();
    std::vector<Byte> tex=texture_fixture();
    AssetHandle<CartridgeTag> cartridge{};
    AssetHandle<TextureTag> texture{};
    CpuBus bus;
    std::optional<CpuDebugSession> cpu;

    Demo() {
        const auto a=registry.import<CartridgeTag>(rom);
        const auto t=registry.import<TextureTag>(tex);
        check(a && t, "shared Pass05/06 decoders must accept synthetic assets");
        cartridge=*a; texture=*t;
    }
};
[[nodiscard]] bool preflight_asset_handles(const Demo& demo) noexcept {
    // A topologically valid plan does not pin assets. Revalidate the actual
    // owner/generation/kind at the execution boundary, before doing any work.
    return demo.registry.lookup(demo.cartridge).has_value() &&
           demo.registry.lookup(demo.texture).has_value();
}
g::DependencyGraph make_demo_graph(const Demo& demo) {
    g::DependencyGraph graph;
    const auto rom=key(demo.cartridge,g::ResourceKind::cartridge);
    const auto tex=key(demo.texture,g::ResourceKind::texture);
    must_add(graph,{task(10),"validate owned cartridge",g::Domain::host,2,{}, {read(rom)}});
    must_add(graph,{task(20),"mount fresh NROM bus copy",g::Domain::host,4,
                    {task(10)},{read(rom),write(cpu_program)}});
    must_add(graph,{task(30),"CPU reset",g::Domain::host,1,
                    {},{read(cpu_program),write(cpu_state)}});
    must_add(graph,{task(40),"LDA immediate",g::Domain::host,2,
                    {},{read(cpu_program),update(cpu_state),write(instruction_trace)}});
    must_add(graph,{task(50),"STA zero page",g::Domain::host,3,
                    {},{read(cpu_program),update(cpu_state),write(cpu_ram),
                        write(instruction_trace)}});
    must_add(graph,{task(60),"inspect CPU RAM",g::Domain::host,1,
                    {},{read(cpu_ram)}});
    must_add(graph,{task(65),"prepare CPU debug overlay",g::Domain::host,2,
                    {task(60)},{read(cpu_state),read(instruction_trace),
                                write(debug_overlay)}});
    must_add(graph,{task(70),"validate RGBA8 asset",g::Domain::host,1,
                    {},{read(tex)}});
    must_add(graph,{task(80),"GPU texture upload plan",g::Domain::gpu_transfer,5,
                    {task(70)},{read(tex),write(gpu_texture)}});
    must_add(graph,{task(90),"GPU compute plan",g::Domain::gpu_compute,8,
                    {},{read(gpu_texture),write(gpu_output)}});
    must_add(graph,{task(100),"GPU graphics plan",g::Domain::gpu_graphics,3,
                    {},{read(gpu_output),read(debug_overlay)}});
    return graph;
}
void execute_cpu_only(Demo& demo,const g::Plan& plan) {
    check(preflight_asset_handles(demo),
          "stale generation must block dispatch before the first task");
    std::vector<std::uint32_t> executed;
    for (const auto& work:plan.ordered) {
        if (work.domain!=g::Domain::host) continue;
        switch(work.id.value) {
            case 10: {
                const auto valid=demo.registry.lookup(demo.cartridge);
                check(valid && valid->get().read_prg(0x8000)==0xA9,
                      "validate real cartridge before mounting");
                break;
            }
            case 20: {
                // The catalog still owns one cartridge. The bus needs its
                // OWN independent NROM value, so re-decode the stored source
                // instead of borrowing a moved-from or dead object.
                // std::expected holds a move-only NromCartridge. Keeping
                // the expected non-const permits a genuine move out of it;
                // moving from const would try the deleted copy constructor.
                auto parsed=NromCartridge::from_image(demo.rom);
                check(parsed.has_value(),"second owned cartridge decode");
                auto copied=std::move(*parsed);
                check(demo.bus.insert(std::move(copied)).has_value(),
                      "real NROM bus mount");
                break;
            }
            case 30:
                demo.cpu.emplace(demo.bus);
                check(demo.cpu->reset().has_value(),"real CPU reset");
                break;
            case 40: {
                check(demo.cpu.has_value(),"CPU must be constructed");
                const auto result=demo.cpu->step();
                check(result && result->opcode==0xA9 && result->total_cycles==2,
                      "LDA #$7F");
                break;
            }
            case 50: {
                const auto result=demo.cpu->step();
                check(result && result->opcode==0x85 && result->total_cycles==5,
                      "STA $01");
                break;
            }
            case 60:
                check(demo.bus.read(0x0001)==0x7F &&
                      demo.bus.read(0x1801)==0x7F,
                      "CPU bus RAM mirror after scheduled STA");
                break;
            case 65:
                check(demo.cpu && demo.cpu->snapshot() &&
                      demo.cpu->snapshot()->pc==0x8004 &&
                      demo.cpu->pending_traces()==2,
                      "overlay staging consumes the completed CPU snapshot");
                break;
            case 70: {
                const auto valid=demo.registry.lookup(demo.texture);
                check(valid && valid->get().pixels.size()==4 &&
                      valid->get().pixels[0]==0x12,
                      "texture only checked as CPU-owned pixels");
                break;
            }
            default:
                check(false,"unknown host task in execution plan");
        }
        executed.push_back(work.id.value);
    }
    check(executed.size()==8,"all eight host tasks must execute once");
    check(demo.cpu && demo.cpu->snapshot() &&
          demo.cpu->snapshot()->a==0x7F &&
          demo.cpu->snapshot()->pc==0x8004 &&
          demo.cpu->snapshot()->total_cycles==5 &&
          demo.cpu->pending_traces()==2,
          "same real Pass07 CPU debugger must retain ordered trace and state");
}
void cpu_gpu_dependency_demo() {
    Demo demo;
    auto graph=make_demo_graph(demo);
    auto p=graph.plan();
    check(p && p->ordered.size()==11,
          "NES+GPU planning fixture must contain 11 tasks");
    verify_order(*p);
    check(edge(*p,20,30) &&
          g::has_reason(*edge(*p,20,30),g::read_after_write),
          "real CPU reset must depend on mounted PRG bytes");
    check(edge(*p,30,40) &&
          g::has_reason(*edge(*p,30,40),g::read_after_write),
          "first CPU instruction must depend on initialized CPU state");
    check(edge(*p,40,50) &&
          g::has_reason(*edge(*p,40,50),g::write_after_write),
          "CPU steps must not be re-ordered");
    check(edge(*p,50,60) &&
          g::has_reason(*edge(*p,50,60),g::read_after_write),
          "RAM observer must wait for STA write");
    check(edge(*p,70,80) &&
          g::has_reason(*edge(*p,70,80),g::explicit_dependency),
          "transfer request cannot preempt CPU-side texture validation");
    check(edge(*p,80,90) && edge(*p,90,100),
          "GPU stage resource writes must create planning dependencies");
    check(edge(*p,65,100) &&
          g::has_reason(*edge(*p,65,100),g::read_after_write),
          "graphics must wait for the CPU-prepared debugger overlay");
    check(p->gpu_edges_requiring_backend_review.size()>=3,
          "GPU stages must carry backend review obligations");

    // CPU work actually RUNS on the previously built NROM debugger. GPU
    // planning nodes are NOT dispatched to a device.
    execute_cpu_only(demo,*p);

    // A plan cannot own/revalidate the external registry. After erasure the
    // same old ordered tasks are syntactically valid but not executable.
    const auto old=demo.cartridge;
    check(demo.registry.erase(old).has_value(),"erase live cartridge");
    check(!demo.registry.lookup(old),"planned handle must now be stale");
    check(!preflight_asset_handles(demo),
          "stale cartridge must refuse execution in preflight");
    // No stale work is dispatched after the failed preflight.
}

void checked_scatter_offsets_and_overflow() {
    constexpr std::array<std::size_t,4> counts{2,0,3,1};
    const auto layout=g::checked_exclusive_scan(counts);
    check(layout && layout->total==6 &&
          layout->offsets==std::vector<std::size_t>({0,2,2,5}),
          "exclusive scan must produce 0,2,2,5 for 2,0,3,1");
    for (std::size_t i=0;i+1<counts.size();++i) {
        check(layout->offsets[i]+counts[i] == layout->offsets[i+1],
              "each half-open scatter interval ends at next offset");
    }
    check(layout->offsets.back()+counts.back()==layout->total,
          "last output interval must end at total");
    const std::array<std::size_t,0> empty{};
    const auto none=g::checked_exclusive_scan(empty);
    check(none && none->offsets.empty() && none->total==0,
          "empty compaction produces no writes");
    constexpr std::array<std::size_t,2> invalid{
        std::numeric_limits<std::size_t>::max(),1};
    const auto over=g::checked_exclusive_scan(invalid);
    check(!over && over.error()==g::ScanError::overflow,
          "oversized count sum must reject rather than wrap");
    // This CPU oracle tests index arithmetic only, not GPU memory visibility.
}

void randomized_acyclic_graph_oracle() {
    std::uint32_t state=0xBADC0DEU;
    const auto next=[&state]() {
        state=state*1664525U+1013904223U;
        return state;
    };
    for (int round=0;round<30;++round) {
        g::DependencyGraph graph;
        // Construct a graph whose explicit edges only point backward in
        // submission index: it is DAG by construction even with extra
        // resource hazards, which also point forward in submission order.
        for (int i=0;i<60;++i) {
            std::vector<g::TaskId> prior;
            if (i>0 && (next()&1U)) {
                prior.push_back(task(static_cast<std::uint32_t>(
                    1+(next()%static_cast<std::uint32_t>(i)))));
            }
            const auto resource=static_cast<std::uint64_t>(next()%11U);
            const bool writer=(next()%3U)==0U;
            must_add(graph,{task(static_cast<std::uint32_t>(i+1)),
                "generated",g::Domain::host,
                static_cast<std::uint64_t>((next()%7U)+1U),
                std::move(prior),
                {writer?write({g::ResourceKind::cpu_ram,0,resource,1}):
                        read({g::ResourceKind::cpu_ram,0,resource,1})}});
        }
        const auto plan=graph.plan();
        check(plan && plan->ordered.size()==60,
              "generated acyclic task graph must produce complete plan");
        verify_order(*plan);
    }
}
} // namespace

int main() {
    try {
        hazard_reason_and_independent_read_test();
        failures_and_determinism();
        generation_key_independence();
        cpu_gpu_dependency_demo();
        checked_scatter_offsets_and_overflow();
        randomized_acyclic_graph_oracle();
        std::cout << "NES-08G: RAW/WAR/WAW, stable topological planning, "
                     "real NROM CPU work, GPU obligations, cycle/overflow "
                     "rejection, 1800-task generated DAG oracle passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "NES-08G FAILED: " << error.what() << '\n';
        return 1;
    }
}
