// Project AllMight — Book 01, Pass 10: a reproducibility *protocol*.
//
// Ordinary CTest mode is a deterministic correctness/invariant check; it
// performs NO performance threshold assertion. --benchmark separately runs
// timed workloads, with warmups, independent result digests and recorded
// distribution statistics. Any user comparing machines must capture toolchain
// and source provenance (tools/nes_course_evidence.py).
//
// We deliberately reuse REAL preceding modules: NES-01/02 NROM parsing,
// Pass07 typed 6502 subset, Pass08 work DAG and checked prefix scan. No GPU.
#include "nes_course/measurement_protocol.hpp"
#include "nes_course/cpu6502_subset.hpp"
#include "nes_course/work_dependency_graph.hpp"
#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using namespace allmight::nes;
using namespace allmight::nes::cpu;
namespace g=allmight::nes::work;
namespace m=allmight::nes::measure;

void check(bool condition,std::string_view why) {
    if (!condition) throw std::runtime_error(std::string(why));
}

std::vector<Byte> make_cartridge() {
    // iNES header (16 bytes), single 16-KiB PRG-ROM bank and CHR-RAM.
    // FFFC/FFFD maps into the mirrored last four bytes of that PRG bank.
    std::vector<Byte> bytes(header_bytes+prg_bank_bytes,0);
    bytes[0]='N';bytes[1]='E';bytes[2]='S';bytes[3]=0x1A;
    bytes[4]=1; bytes[5]=0;
    bytes[header_bytes+0]=0xA9; // LDA #$7F
    bytes[header_bytes+1]=0x7F;
    bytes[header_bytes+2]=0x85; // STA $02
    bytes[header_bytes+3]=0x02;
    bytes[header_bytes+4]=0xEA; // NOP
    bytes[header_bytes+0x3FFCU]=0;
    bytes[header_bytes+0x3FFDU]=0x80;
    return bytes;
}

g::DependencyGraph make_workload_graph() {
    g::DependencyGraph graph;
    // Fixed 32-task graph, four logical resources and deterministic
    // submission order. The planner must derive WAR/WAW/RAW from accesses.
    for (std::uint32_t i=0;i<32;++i) {
        const g::ResourceKey resource{
            g::ResourceKind::cpu_ram,0,i%4,1
        };
        const g::Access access{
            resource, (i%3)!=0, (i%3)==0
        };
        auto added=graph.add({
            g::TaskId{i+1},
            "profile-graph",
            g::Domain::host,
            static_cast<std::uint64_t>((i%5)+1),
            {},
            {access}
        });
        check(added.has_value(),"32-task profiler graph fixture");
    }
    return graph;
}

std::vector<std::size_t> make_counts() {
    std::vector<std::size_t> result;
    result.reserve(256);
    std::uint32_t state=0xC0FFEEU;
    for (std::size_t i=0;i<256;++i) {
        state=state*1664525U+1013904223U;
        result.push_back(static_cast<std::size_t>(state%7U));
    }
    return result;
}

std::uint64_t witness_rom(std::span<const Byte> bytes) {
    const auto loaded=NromCartridge::from_image(bytes);
    check(loaded.has_value() && loaded->ready(),"valid NROM import");
    const auto op=loaded->read_prg(0x8000);
    const auto vector_lo=loaded->read_prg(0xFFFC);
    const auto vector_hi=loaded->read_prg(0xFFFD);
    check(op && vector_lo && vector_hi,
          "PRG and reset vector must be available");
    return (static_cast<std::uint64_t>(*op)<<16U) |
           (static_cast<std::uint64_t>(*vector_hi)<<8U) |
           static_cast<std::uint64_t>(*vector_lo);
}

std::uint64_t witness_cpu(Cpu6502Subset& cpu,CpuBus& bus) {
    check(cpu.reset().has_value(),"CPU reset vector must work");
    auto lda=cpu.step();
    auto sta=cpu.step();
    auto nop=cpu.step();
    check(lda && sta && nop,"three-opcode CPU workload");
    check(lda->opcode==0xA9 && sta->opcode==0x85 && nop->opcode==0xEA,
          "expected exact synthetic program opcodes");
    const auto readback=bus.read(0x1802);
    check(readback && *readback==0x7F,
          "mirrored CPU RAM must contain the stored accumulator");
    const auto& regs=cpu.registers();
    check(regs.a==0x7F && regs.pc.value==0x8005 &&
          cpu.total_cycles()==7,"CPU trace and total cost oracle");
    return (static_cast<std::uint64_t>(regs.a)<<32U) |
           (static_cast<std::uint64_t>(regs.pc.value)<<16U) |
           cpu.total_cycles();
}

std::uint64_t witness_graph(const g::DependencyGraph& graph) {
    const auto planned=graph.plan();
    check(planned && planned->ordered.size()==32,
          "work graph must contain all declared vertices");
    std::uint64_t digest=0x5A17;
    for (const auto& entry:planned->ordered)
        digest=m::mix(digest,
            static_cast<std::uint64_t>(entry.id.value) ^
            (entry.earliest_finish<<16U));
    // Independent property oracle: each direct dependency must be satisfied
    // by the output order and its earliest-start/finish estimates.
    for (const auto& edge:planned->edges) {
        const auto from=std::find_if(planned->ordered.begin(),planned->ordered.end(),
            [&](const g::PlannedTask& t){return t.id==edge.from;});
        const auto to=std::find_if(planned->ordered.begin(),planned->ordered.end(),
            [&](const g::PlannedTask& t){return t.id==edge.to;});
        check(from!=planned->ordered.end() &&
              to!=planned->ordered.end() &&
              from<to && from->earliest_finish<=to->earliest_start,
              "profile graph order/critical-path constraint");
    }
    return m::mix(digest,planned->critical_path_cost);
}

std::uint64_t witness_scan(std::span<const std::size_t> counts) {
    const auto result=g::checked_exclusive_scan(counts);
    check(result && result->offsets.size()==counts.size(),
          "exclusive scan correct size");
    std::uint64_t digest=0x9382;
    std::size_t independent_total=0;
    for (std::size_t i=0;i<counts.size();++i) {
        check(result->offsets[i]==independent_total,
              "reference scan equals independently accumulated prefix");
        digest=m::mix(digest,static_cast<std::uint64_t>(independent_total));
        independent_total+=counts[i]; // Fixture is bounded by 256*6.
    }
    check(result->total==independent_total,
          "scan total must equal sum of reference counts");
    return m::mix(digest,static_cast<std::uint64_t>(result->total));
}

void test_summary_and_rejection() {
    constexpr std::array<std::uint64_t,9> samples{9,1,7,3,5,2,8,4,6};
    const m::Settings settings{2,9,1};
    const auto report=m::summarize(samples,0xABCD,settings);
    check(report && report->min_ns==1 && report->median_ns==5 &&
          report->p90_ns==9 && report->max_ns==9 &&
          report->stable_digest==0xABCD,
          "nearest-rank report must use actual sorted sample values");
    check(m::rank_nearest(9,50)==4 && m::rank_nearest(9,90)==8,
          "median and p90 index convention is documented");
    check(!m::summarize(samples,0,{0,0,1}) &&
          !m::summarize(samples,0,{0,9,0}),
          "zero trials and zero iterations must be rejected");
    check(!m::run([](std::size_t){return std::uint64_t{1};},
                  {0,1,0}),"timed zero-iteration request refused");
    check(!m::run([](std::size_t){return std::uint64_t{1};},
                  {0,0,1}),"timed zero-trial request refused");
    check(!m::run([](std::size_t){return std::uint64_t{1};},
                  {0,1,1000001}),"unbounded iteration request refused");
    // This deliberate alternating witness checks the profiler detects
    // inconsistent results rather than reporting an apparent speedup.
    std::uint64_t alternating{};
    const auto unreliable=m::run(
        [&](std::size_t){return ++alternating;},{0,2,1});
    check(!unreliable &&
          unreliable.error()==m::MeasureError::inconsistent_work_result,
          "changing workload result must invalidate the measurement");
}

void correctness_suite() {
    test_summary_and_rejection();
    const auto image=make_cartridge();
    const auto counts=make_counts();
    const auto graph=make_workload_graph();
    CpuBus bus;
    auto loaded=NromCartridge::from_image(image);
    check(loaded.has_value(),"fixture cartridge");
    check(bus.insert(std::move(*loaded)).has_value(),"mount NROM fixture");
    Cpu6502Subset cpu(bus);

    const auto rom=witness_rom(image);
    const auto cpu_value=witness_cpu(cpu,bus);
    const auto graph_value=witness_graph(graph);
    const auto scan_value=witness_scan(counts);
    for (int i=0;i<16;++i) {
        check(witness_rom(image)==rom,"NROM decoded result is stable");
        check(witness_cpu(cpu,bus)==cpu_value,
              "CPU reset+instruction witness is stable");
        check(witness_graph(graph)==graph_value,
              "graph order and costs stable");
        check(witness_scan(counts)==scan_value,
              "prefix scan output stable");
    }
}

std::optional<std::size_t> parse_positive(std::string_view arg) {
    std::size_t value{};
    const auto begin=arg.data();
    const auto end=begin+arg.size();
    const auto result=std::from_chars(begin,end,value);
    if(result.ec!=std::errc{} || result.ptr!=end || value==0)
        return std::nullopt;
    return value;
}

void print_report(std::string_view name,const m::SampleReport& report) {
    std::cout<<name<<','<<report.warmups<<','<<report.trials<<','
             <<report.iterations_per_trial<<','
             <<report.min_ns<<','<<report.median_ns<<','
             <<report.p90_ns<<','<<report.max_ns<<','
             <<report.stable_digest<<'\n';
}

void timed_protocol(m::Settings settings) {
    const auto image=make_cartridge();
    const auto counts=make_counts();
    const auto graph=make_workload_graph();
    CpuBus bus;
    auto loaded=NromCartridge::from_image(image);
    check(loaded.has_value(),"timed NROM fixture");
    check(bus.insert(std::move(*loaded)).has_value(),"timed bus mount");
    Cpu6502Subset cpu(bus);

    const auto time_one=[&](std::string_view name,auto&& workload) {
        const auto measured=m::run(std::forward<decltype(workload)>(workload),
                                   settings);
        check(measured.has_value(),"benchmark timed result invalid");
        print_report(name,*measured);
    };
    std::cout<<"workload,warmups,trials,iterations_per_trial,"
                "min_ns,median_ns,p90_ns,max_ns,checksum\n";
    time_one("nrom_owned_decode",[&](std::size_t) {
        return witness_rom(image);
    });
    time_one("cpu_3_instructions",[&](std::size_t) {
        return witness_cpu(cpu,bus);
    });
    time_one("dependency_graph_32",[&](std::size_t) {
        return witness_graph(graph);
    });
    time_one("exclusive_scan_256",[&](std::size_t) {
        return witness_scan(counts);
    });
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc==1) {
            correctness_suite();
            std::cout<<"NES-10R: deterministic independent correctness"
                        " oracles and measurement contract checks passed\n";
            return 0;
        }
        if (std::string_view(argv[1])!="--benchmark" || argc>5) {
            std::cerr<<"usage: nes_reproducible_benchmark_cpp23_lab "
                        "[--benchmark [trials [iterations [warmups]]]]\n";
            return 2;
        }
        // Real timing is an opt-in experiment, never run as a CTest speed
        // threshold. A benchmark invocation uses one homogeneous process.
        m::Settings settings{};
        if(argc>=3) {
            const auto parsed=parse_positive(argv[2]);
            check(parsed.has_value(),"invalid positive number of trials");
            settings.trials=*parsed;
        }
        if(argc>=4) {
            const auto parsed=parse_positive(argv[3]);
            check(parsed.has_value(),"invalid positive iteration count");
            settings.iterations=*parsed;
        }
        if(argc>=5) {
            const auto parsed=parse_positive(argv[4]);
            check(parsed.has_value(),"invalid positive warmup count");
            settings.warmups=*parsed;
        }
        correctness_suite();
        timed_protocol(settings);
        return 0;
    } catch (const std::exception& error) {
        std::cerr<<"NES-10R FAILED: "<<error.what()<<'\n';
        return 1;
    }
}
