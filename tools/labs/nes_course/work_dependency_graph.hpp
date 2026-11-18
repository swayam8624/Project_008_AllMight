#pragma once
// AllMight Book 1 / Pass 08 — CPU-side dependency *planner* for NES/debugger
// work and conceptual GPU copy/compute/graphics stages.
//
// This is NOT an actual Vulkan/Metal command scheduler. A DAG order is not a
// memory barrier, a queue semaphore, a timeline value, or proof of GPU
// completion. The executor must validate owner/generation handles at use time.
//
// Tasks declare every resource touched and whether it is read or written.
// Read-after-read is free; RAW/WAR/WAW relationships preserve submission order
// for shared resource keys. Explicit dependency edges preserve causal order
// even where there is no resource overlap. The graph is single-threaded.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <initializer_list>
#include <limits>
#include <optional>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace allmight::nes::work {

struct TaskId {
    std::uint32_t value{};
    friend constexpr bool operator==(TaskId, TaskId) = default;
};
enum class Domain : std::uint8_t {
    host,
    gpu_transfer,
    gpu_compute,
    gpu_graphics
};
enum class ResourceKind : std::uint8_t {
    cpu_state,
    cpu_ram,
    cpu_program,
    instruction_trace,
    debug_overlay,
    cartridge,
    texture,
    mesh,
    gpu_texture,
    gpu_output
};
struct ResourceKey {
    ResourceKind kind{};
    std::uint64_t owner{};
    std::uint64_t slot{};
    std::uint64_t generation{};
    friend constexpr bool operator==(ResourceKey, ResourceKey) = default;
};
struct Access {
    ResourceKey resource{};
    bool read{};
    bool write{};
};
struct Task {
    TaskId id{};
    std::string name;
    Domain domain{Domain::host};
    std::uint64_t estimated_cost{1}; // scheduling *estimate*, not 6502 cycles
    std::vector<TaskId> explicit_after;
    std::vector<Access> accesses;
};

enum class GraphError {
    duplicate_task,
    invalid_task,
    invalid_access,
    duplicate_resource,
    missing_dependency,
    directed_cycle,
    critical_path_overflow,
    too_many_tasks
};
struct Failure {
    GraphError kind{};
    TaskId at{};
    TaskId related{};
};

enum EdgeReason : std::uint8_t {
    explicit_dependency = 1,
    read_after_write = 2,
    write_after_read = 4,
    write_after_write = 8
};
struct Edge {
    TaskId from{}, to{};
    std::uint8_t reasons{};
};
struct PlannedTask {
    TaskId id{};
    std::uint32_t layer{};
    std::uint64_t earliest_start{};
    std::uint64_t earliest_finish{};
    Domain domain{Domain::host};
    friend bool operator==(const PlannedTask&, const PlannedTask&) = default;
};
struct Plan {
    std::vector<PlannedTask> ordered;
    std::vector<Edge> edges; // canonical sorted, merged direct edges
    std::uint64_t critical_path_cost{};
    // GPU queue/pipeline changes are descriptive obligations only.
    std::vector<Edge> gpu_edges_requiring_backend_review;
};

[[nodiscard]] constexpr bool is_gpu(Domain d) noexcept {
    return d != Domain::host;
}
[[nodiscard]] constexpr bool is_dependency(const Edge& e,
                                            TaskId from, TaskId to) noexcept {
    return e.from == from && e.to == to;
}
[[nodiscard]] constexpr bool has_reason(const Edge& e, EdgeReason reason) noexcept {
    return (e.reasons & static_cast<std::uint8_t>(reason)) != 0;
}
struct ResourceKeyHash {
    std::size_t operator()(const ResourceKey& r) const noexcept {
        // Identity comparison remains exact: hash collisions are expected
        // and harmless because unordered_map applies operator== afterwards.
        std::uint64_t h = static_cast<std::uint8_t>(r.kind) + 0x9e3779b97f4a7c15ULL;
        for (const auto x : {r.owner, r.slot, r.generation}) {
            h ^= x + 0x9e3779b97f4a7c15ULL + (h << 6U) + (h >> 2U);
        }
        return static_cast<std::size_t>(h);
    }
};

class DependencyGraph {
public:
    // A submission is atomic for semantic errors. Allocation failures may
    // propagate std::bad_alloc; no live work runs during graph construction.
    [[nodiscard]] std::expected<void, Failure> add(Task task) {
        if (tasks_.size() >= 4096)
            return std::unexpected(Failure{GraphError::too_many_tasks, task.id, {}});
        if (task.name.empty() || task.estimated_cost == 0)
            return std::unexpected(Failure{GraphError::invalid_task, task.id, {}});
        for (const auto& old : tasks_)
            if (old.id == task.id)
                return std::unexpected(Failure{
                    GraphError::duplicate_task, task.id, old.id});
        for (std::size_t i=0; i<task.accesses.size(); ++i) {
            const auto& a = task.accesses[i];
            if (!a.read && !a.write)
                return std::unexpected(Failure{GraphError::invalid_access,task.id,{}});
            for (std::size_t j=0; j<i; ++j)
                if (a.resource == task.accesses[j].resource)
                    return std::unexpected(Failure{
                        GraphError::duplicate_resource,task.id,{}});
        }
        tasks_.push_back(std::move(task));
        return {};
    }

    [[nodiscard]] std::size_t size() const noexcept { return tasks_.size(); }

    // Pure with respect to the graph: even a cycle/missing prerequisite does
    // not mutate submitted tasks or publish a partial Plan.
    [[nodiscard]] std::expected<Plan, Failure> plan() const {
        const std::size_t n = tasks_.size();
        std::unordered_map<std::uint32_t,std::size_t> by_id;
        by_id.reserve(n);
        for (std::size_t i=0; i<n; ++i)
            by_id.emplace(tasks_[i].id.value,i);

        struct IndexedEdge {
            std::size_t from{}, to{};
            std::uint8_t reasons{};
        };
        std::vector<IndexedEdge> edges;
        std::unordered_map<std::uint64_t,std::size_t> edge_index;
        const auto add_edge = [&edges,&edge_index](std::size_t a,std::size_t b,
                                                   std::uint8_t reason) {
            // 4096-task bound permits a collision-free 32+32 bit index pair.
            // Different reasons for one pair are flags on ONE graph edge,
            // otherwise Kahn indegrees and debug output would be misleading.
            const auto key = (static_cast<std::uint64_t>(a) << 32U) |
                             static_cast<std::uint64_t>(b);
            const auto it = edge_index.find(key);
            if (it != edge_index.end()) {
                auto& flags = edges[it->second].reasons;
                flags = static_cast<std::uint8_t>(flags | reason);
                return;
            }
            // Reserve both mutations so a failure cannot leave an invalid
            // edge-index map entry. Exceptions propagate; plan is pure.
            edges.push_back(IndexedEdge{a,b,reason});
            try {
                edge_index.emplace(key,edges.size()-1);
            } catch (...) {
                edges.pop_back();
                throw;
            }
        };

        // Forward references are legal while building. They must resolve
        // before any plan may be returned.
        for (std::size_t to=0; to<n; ++to) {
            for (const auto parent : tasks_[to].explicit_after) {
                const auto it = by_id.find(parent.value);
                if (it == by_id.end())
                    return std::unexpected(Failure{
                        GraphError::missing_dependency,tasks_[to].id,parent});
                add_edge(it->second,to,explicit_dependency);
            }
        }

        struct AccessHistory {
            std::optional<std::size_t> writer;
            std::vector<std::size_t> readers;
        };
        std::unordered_map<ResourceKey,AccessHistory,ResourceKeyHash> history;
        // Inferred hazards follow submission order. Two reads are independent.
        // For each writer, depend on the last writer and all intervening
        // readers. Earlier writers are already ordered transitively.
        for (std::size_t to=0; to<n; ++to) {
            for (const auto& a : tasks_[to].accesses) {
                auto& state = history[a.resource];
                if (state.writer) {
                    std::uint8_t reason = 0;
                    if (a.read) reason = static_cast<std::uint8_t>(
                        reason | read_after_write);
                    if (a.write) reason = static_cast<std::uint8_t>(
                        reason | write_after_write);
                    add_edge(*state.writer,to,reason);
                }
                if (a.write) {
                    for (const auto reader : state.readers)
                        add_edge(reader,to,write_after_read);
                    state.readers.clear();
                    state.writer = to;
                } else {
                    state.readers.push_back(to);
                }
            }
        }

        std::vector<std::size_t> indegree(n,0);
        std::vector<std::vector<std::size_t>> outgoing(n);
        for (std::size_t i=0;i<edges.size();++i) {
            ++indegree[edges[i].to];
            outgoing[edges[i].from].push_back(i);
        }
        const auto lower_id = [this](std::size_t a,std::size_t b) {
            return tasks_[a].id.value > tasks_[b].id.value;
        };
        std::priority_queue<std::size_t,std::vector<std::size_t>,
                            decltype(lower_id)> ready(lower_id);
        for (std::size_t i=0;i<n;++i)
            if (indegree[i]==0) ready.push(i);

        std::vector<std::uint64_t> earliest(n,0);
        std::vector<std::uint32_t> layer(n,0);
        Plan result;
        result.ordered.reserve(n);
        result.edges.reserve(edges.size());
        while (!ready.empty()) {
            const auto i=ready.top(); ready.pop();
            const auto start=earliest[i];
            const auto cost=tasks_[i].estimated_cost;
            if (cost>std::numeric_limits<std::uint64_t>::max()-start)
                return std::unexpected(Failure{
                    GraphError::critical_path_overflow,tasks_[i].id,{}});
            const auto finish=start+cost;
            result.critical_path_cost=std::max(result.critical_path_cost,finish);
            result.ordered.push_back(PlannedTask{
                tasks_[i].id,layer[i],start,finish,tasks_[i].domain});
            for (const auto e_index:outgoing[i]) {
                const auto& e=edges[e_index];
                earliest[e.to]=std::max(earliest[e.to],finish);
                layer[e.to]=std::max(layer[e.to],layer[i]+1U);
                if (--indegree[e.to]==0) ready.push(e.to);
            }
        }
        if (result.ordered.size()!=n) {
            // First unresolved vertex is a BLOCKED task, not necessarily
            // a member of the cycle. A true cycle witness would require a
            // separate DFS of the unresolved subgraph.
            for (std::size_t i=0;i<n;++i)
                if (indegree[i]!=0) return std::unexpected(Failure{
                    GraphError::directed_cycle,tasks_[i].id,{}});
        }

        for (const auto& e:edges) {
            Edge item{tasks_[e.from].id,tasks_[e.to].id,e.reasons};
            result.edges.push_back(item);
            // Every edge touching GPU work is an obligation to review
            // stages/access scopes and (where applicable) queue ownership.
            // This list is NOT a list of actual VkDependencyInfo barriers.
            if (is_gpu(tasks_[e.from].domain) ||
                is_gpu(tasks_[e.to].domain))
                result.gpu_edges_requiring_backend_review.push_back(item);
        }
        const auto order=[](const Edge& a,const Edge& b) {
            return a.from.value!=b.from.value
                ? a.from.value<b.from.value : a.to.value<b.to.value;
        };
        std::sort(result.edges.begin(),result.edges.end(),order);
        std::sort(result.gpu_edges_requiring_backend_review.begin(),
                  result.gpu_edges_requiring_backend_review.end(),order);
        return result;
    }

private:
    std::vector<Task> tasks_; // submission order defines hazard direction
};

static_assert(std::is_trivially_copyable_v<TaskId>);
static_assert(std::is_trivially_copyable_v<ResourceKey>);

} // namespace allmight::nes::work
