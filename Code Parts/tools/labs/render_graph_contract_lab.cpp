// An inspectable single-queue render-graph dependency compiler, not a GPU barrier API.
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

struct Resource {
    std::string name;
    bool imported = false; // Already initialized outside this graph, e.g. last frame's history.
};

struct Pass {
    std::string name;
    std::vector<std::string> reads;
    std::vector<std::string> writes;
};

struct Dependency {
    std::size_t before{};
    std::size_t after{};
    std::string resource;
    std::string reason; // RAW, WAR, WAW, or an explicit CONTROL ordering.
};

struct CompiledGraph {
    std::vector<std::size_t> order;
    std::vector<Dependency> dependencies;
};

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

struct ResourceState {
    bool imported = false;
    std::optional<std::size_t> last_writer;
    std::set<std::size_t> readers_since_write;
};

CompiledGraph compile_graph(
    const std::vector<Resource>& resources,
    const std::vector<Pass>& passes,
    const std::vector<std::pair<std::size_t, std::size_t>>& explicit_edges = {}) {

    // This laboratory uses resource names as whole-resource IDs. For actual
    // images, a production engine must track mip/layer/aspect subresources.
    std::map<std::string, ResourceState> states;
    for (const auto& resource : resources) {
        require(!resource.name.empty(), "a resource needs a name");
        require(states.emplace(resource.name, ResourceState{resource.imported, {}, {}}).second,
                "duplicate resource: " + resource.name);
    }

    // Each pass name is an identity for diagnostics, not a GPU object handle.
    std::set<std::string> pass_names;
    std::set<std::tuple<std::size_t, std::size_t, std::string, std::string>> edges;
    for (std::size_t index = 0; index < passes.size(); ++index) {
        const Pass& pass = passes[index];
        require(!pass.name.empty() && pass_names.insert(pass.name).second,
                "pass names must be nonempty and unique");
        std::set<std::string> seen;
        for (const auto& name : pass.reads) {
            require(seen.insert(name).second, "duplicate or feedback access in " + pass.name);
            auto state_it = states.find(name);
            require(state_it != states.end(), "undeclared read resource: " + name);
            ResourceState& state = state_it->second;
            if (state.last_writer)
                edges.emplace(*state.last_writer, index, name, "RAW");
            else
                require(state.imported, "read before first write: " + name);
            state.readers_since_write.insert(index);
        }
        for (const auto& name : pass.writes) {
            require(seen.insert(name).second,
                    "read/write feedback or duplicate write in " + pass.name);
            auto state_it = states.find(name);
            require(state_it != states.end(), "undeclared write resource: " + name);
            ResourceState& state = state_it->second;
            if (state.last_writer)
                edges.emplace(*state.last_writer, index, name, "WAW");
            for (const std::size_t reader : state.readers_since_write)
                edges.emplace(reader, index, name, "WAR");
            state.readers_since_write.clear();
            state.last_writer = index;
        }
    }

    // Explicit ordering is for external side effects or constraints that
    // cannot be deduced from declared resource accesses alone.
    for (const auto& [before, after] : explicit_edges) {
        require(before < passes.size() && after < passes.size() && before != after,
                "invalid explicit dependency");
        edges.emplace(before, after, "<external>", "CONTROL");
    }

    std::vector<std::set<std::size_t>> outgoing(passes.size());
    std::vector<std::size_t> indegree(passes.size(), 0);
    CompiledGraph compiled;
    for (const auto& [before, after, resource, reason] : edges) {
        compiled.dependencies.push_back({before, after, resource, reason});
        if (outgoing[before].insert(after).second) ++indegree[after];
    }

    // The priority queue makes topological scheduling reproducible when
    // multiple independent passes are ready simultaneously.
    std::priority_queue<std::size_t, std::vector<std::size_t>, std::greater<>> ready;
    for (std::size_t i = 0; i < indegree.size(); ++i)
        if (indegree[i] == 0) ready.push(i);
    while (!ready.empty()) {
        const std::size_t chosen = ready.top();
        ready.pop();
        compiled.order.push_back(chosen);
        for (std::size_t dependent : outgoing[chosen])
            if (--indegree[dependent] == 0) ready.push(dependent);
    }
    require(compiled.order.size() == passes.size(), "cycle in pass dependencies");
    return compiled;
}

bool has_dependency(const CompiledGraph& graph, std::size_t before,
                    std::size_t after, const std::string& resource,
                    const std::string& reason) {
    return std::any_of(graph.dependencies.begin(), graph.dependencies.end(),
        [&](const Dependency& d) {
            return d.before == before && d.after == after &&
                   d.resource == resource && d.reason == reason;
        });
}

template<class Function>
void must_reject(Function&& function, const std::string& expected_message) {
    try {
        function();
    } catch (const std::runtime_error& error) {
        require(std::string(error.what()).find(expected_message) != std::string::npos,
                "rejected for the wrong reason: " + std::string(error.what()));
        return;
    }
    throw std::runtime_error("invalid graph was accepted: " + expected_message);
}

int main() {
    // The renderer declares logical resources; no Vulkan image, Metal texture,
    // memory allocation, or transition has been created by this compilation.
    const std::vector<Resource> resources{{"albedo"}, {"depth"}, {"shadow"},
        {"hdr"}, {"history", true}, {"history_next"}, {"ldr"}};
    const std::vector<Pass> passes{
        {"gbuffer", {}, {"albedo", "depth"}},
        {"shadow_map", {}, {"shadow"}},
        {"lighting", {"albedo", "depth", "shadow"}, {"hdr"}},
        {"temporal", {"hdr", "history"}, {"history_next"}},
        {"tonemap", {"history_next"}, {"ldr"}},
        {"present", {"ldr"}, {}},
    };
    const CompiledGraph compiled = compile_graph(resources, passes);
    require(compiled.order == std::vector<std::size_t>({0, 1, 2, 3, 4, 5}),
            "stable topological order changed");
    require(has_dependency(compiled, 0, 2, "albedo", "RAW"), "missing albedo RAW");
    require(has_dependency(compiled, 0, 2, "depth", "RAW"), "missing depth RAW");
    require(has_dependency(compiled, 1, 2, "shadow", "RAW"), "missing shadow RAW");
    require(has_dependency(compiled, 2, 3, "hdr", "RAW"), "missing HDR RAW");
    require(has_dependency(compiled, 3, 4, "history_next", "RAW"), "missing temporal RAW");
    require(has_dependency(compiled, 4, 5, "ldr", "RAW"), "missing present RAW");

    // First a reader uses imported history, then a later pass overwrites it.
    // Even with no earlier writer in this frame, the later writer must wait.
    const auto war = compile_graph({{"history", true}},
        {{"sample_old_history", {"history"}, {}},
         {"overwrite_history", {}, {"history"}}});
    require(has_dependency(war, 0, 1, "history", "WAR"), "missing WAR");

    const auto waw = compile_graph({{"target"}},
        {{"clear", {}, {"target"}}, {"draw", {}, {"target"}}});
    require(has_dependency(waw, 0, 1, "target", "WAW"), "missing WAW");

    must_reject([] {
        (void)compile_graph({{"transient"}}, {{"bad_read", {"transient"}, {}}});
    }, "read before first write");
    must_reject([] {
        (void)compile_graph({{"texture", true}},
                            {{"feedback", {"texture"}, {"texture"}}});
    }, "feedback");
    must_reject([] {
        (void)compile_graph({}, {{"a", {}, {}}, {"b", {}, {}}}, {{0, 1}, {1, 0}});
    }, "cycle");
    must_reject([] {
        (void)compile_graph({{"x"}, {"x"}}, {});
    }, "duplicate resource");

    std::cout << "PASS: graph hazards and cycle/initialization/feedback rejection\n";
    for (std::size_t index : compiled.order)
        std::cout << "pass[" << index << "]: " << passes[index].name << '\n';
    for (const auto& dependency : compiled.dependencies)
        std::cout << passes[dependency.before].name << " -> "
                  << passes[dependency.after].name << " : "
                  << dependency.reason << " " << dependency.resource << '\n';
}
