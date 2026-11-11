// Phase 5 (M025-M030, original Parts 401-500): algorithmic witnesses.
// Correctness first. This lab does not claim every listed algorithm is
// implemented, nor that asymptotic analysis predicts cache behavior.
// Tests use throwing checks and remain enabled under -DNDEBUG.
#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <queue>
#include <random>
#include <span>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace allmight::phase5 {
void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

// Fixed-capacity FIFO: count separates full from empty. Another valid design
// reserves an unused slot. Both require explicit wrap and lifetime rules.
template<class T, std::size_t Capacity>
class Ring {
    static_assert(Capacity > 0);
    std::array<std::optional<T>, Capacity> slots_{};
    std::size_t head_{}, count_{};
public:
    bool push(T value) {
        if (count_ == Capacity) return false;
        const auto tail = (head_ + count_) % Capacity;
        slots_[tail].emplace(std::move(value));
        ++count_;
        return true;
    }
    std::optional<T> pop() {
        if (!count_) return std::nullopt;
        auto value = std::move(slots_[head_]);
        slots_[head_].reset();
        head_ = (head_ + 1) % Capacity;
        --count_;
        return value;
    }
    std::size_t size() const noexcept { return count_; }
};

// An explicit, pedagogical open-addressed hash table. Linear probing uses
// tombstones to preserve search chains after deletions. Growth rehashes.
// No concurrent access and no stable element pointer promise.
class LinearMap {
    struct Slot {
        int key{}, value{};
        bool occupied{}, deleted{};
    };
    std::vector<Slot> slots_{8};
    std::size_t size_{}, tombstones_{};

    std::size_t bucket(int key) const {
        return std::hash<int>{}(key) % slots_.size();
    }
    void insert_no_resize(int key, int value) {
        std::size_t position = bucket(key);
        std::size_t first_dead = slots_.size();
        for (std::size_t probe = 0; probe < slots_.size(); ++probe) {
            auto& slot = slots_[position];
            if (slot.occupied && slot.key == key) {
                slot.value = value;
                return;
            }
            if (!slot.occupied && slot.deleted && first_dead == slots_.size())
                first_dead = position;
            if (!slot.occupied && !slot.deleted) {
                const auto chosen = first_dead == slots_.size()
                                  ? position : first_dead;
                auto& target = slots_[chosen];
                if (target.deleted) --tombstones_;
                target = Slot{key, value, true, false};
                ++size_;
                return;
            }
            position = (position + 1) % slots_.size();
        }
        if (first_dead != slots_.size()) {
            slots_[first_dead] = Slot{key, value, true, false};
            --tombstones_;
            ++size_;
            return;
        }
        throw std::logic_error("probe table unexpectedly full");
    }
    void rehash(std::size_t capacity) {
        auto previous = std::move(slots_);
        slots_.assign(capacity, {});
        size_ = 0;
        tombstones_ = 0;
        for (const Slot& s : previous)
            if (s.occupied) insert_no_resize(s.key, s.value);
    }
public:
    void insert(int key, int value) {
        if ((size_ + tombstones_ + 1) * 10 >= slots_.size() * 7)
            rehash(slots_.size() * 2);
        insert_no_resize(key, value);
    }
    std::optional<int> get(int key) const {
        std::size_t position = bucket(key);
        for (std::size_t probe = 0; probe < slots_.size(); ++probe) {
            const auto& slot = slots_[position];
            if (slot.occupied && slot.key == key) return slot.value;
            if (!slot.occupied && !slot.deleted) return std::nullopt;
            position = (position + 1) % slots_.size();
        }
        return std::nullopt;
    }
    bool erase(int key) {
        std::size_t position = bucket(key);
        for (std::size_t probe = 0; probe < slots_.size(); ++probe) {
            auto& slot = slots_[position];
            if (slot.occupied && slot.key == key) {
                slot.occupied = false;
                slot.deleted = true;
                --size_;
                ++tombstones_;
                return true;
            }
            if (!slot.occupied && !slot.deleted) return false;
            position = (position + 1) % slots_.size();
        }
        return false;
    }
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return slots_.size(); }
};

// Bloom filters can reject a nonmember with certainty when its bits are
// absent. All-positive bits mean "possibly present", not a set membership
// proof. Deletion requires an additional algorithm such as counting filters.
class Bloom {
    static constexpr std::size_t Bits = 2048;
    std::bitset<Bits> bits_;
    static std::size_t h1(std::uint32_t v) {
        return (static_cast<std::uint64_t>(v) * 2654435761ULL) % Bits;
    }
    static std::size_t h2(std::uint32_t v) {
        return (static_cast<std::uint64_t>(v) * 2246822519ULL + 17ULL) % Bits;
    }
public:
    void add(std::uint32_t v) { bits_.set(h1(v)); bits_.set(h2(v)); }
    bool possibly_contains(std::uint32_t v) const {
        return bits_.test(h1(v)) && bits_.test(h2(v));
    }
};

class DisjointSet {
    std::vector<std::size_t> parent_, size_;
public:
    explicit DisjointSet(std::size_t n) : parent_(n), size_(n, 1) {
        for (std::size_t i=0;i<n;++i) parent_[i]=i;
    }
    std::size_t find(std::size_t node) {
        if (node >= parent_.size()) throw std::out_of_range("DSU index");
        if (parent_[node] != node) parent_[node] = find(parent_[node]);
        return parent_[node];
    }
    bool unite(std::size_t a, std::size_t b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (size_[a] < size_[b]) std::swap(a,b);
        parent_[b]=a;
        size_[a]+=size_[b];
        return true;
    }
};

struct Edge { std::size_t to; std::int64_t weight; };
using Graph = std::vector<std::vector<Edge>>;
constexpr auto INF = std::numeric_limits<std::int64_t>::max() / 4;

void add_directed(Graph& g, std::size_t from, std::size_t to,
                  std::int64_t weight) {
    if (from >= g.size() || to >= g.size() || weight <= -INF || weight >= INF)
        throw std::invalid_argument("graph edge out of contract");
    g[from].push_back({to,weight});
}

std::vector<int> bfs_hops(const Graph& g, std::size_t source) {
    if (source >= g.size()) throw std::out_of_range("BFS source");
    std::vector<int> hop(g.size(), -1);
    std::queue<std::size_t> pending;
    hop[source]=0;
    pending.push(source);
    while (!pending.empty()) {
        const auto u=pending.front();
        pending.pop();
        for (const auto& e:g[u]) if(hop[e.to] < 0) {
            hop[e.to]=hop[u]+1;
            pending.push(e.to);
        }
    }
    return hop;
}

// Invariant: the extracted min-distance vertex has an optimal distance
// ONLY under nonnegative edge weights. Stale heap entries are skipped.
std::vector<std::int64_t> dijkstra(const Graph& g, std::size_t source) {
    if (source >= g.size()) throw std::out_of_range("Dijkstra source");
    std::vector<std::int64_t> dist(g.size(), INF);
    using State=std::pair<std::int64_t,std::size_t>;
    std::priority_queue<State,std::vector<State>,std::greater<State>> pq;
    dist[source]=0; pq.push({0,source});
    while (!pq.empty()) {
        const auto [d,u]=pq.top();pq.pop();
        if (d != dist[u]) continue;
        for (const auto& e:g[u]) {
            if (e.weight < 0) throw std::invalid_argument("negative Dijkstra edge");
            if (e.weight > INF-d) continue;
            const auto candidate=d+e.weight;
            if (candidate<dist[e.to]) {
                dist[e.to]=candidate;
                pq.push({candidate,e.to});
            }
        }
    }
    return dist;
}

// Kahn's algorithm: a DAG has at least one zero-indegree vertex. Fewer than
// N extracted vertices implies a directed cycle.
std::optional<std::vector<std::size_t>> topological_sort(const Graph& g) {
    std::vector<std::size_t> degree(g.size(),0), order;
    for (const auto& edges:g)
        for(const auto& e:edges) ++degree[e.to];
    std::queue<std::size_t> ready;
    for (std::size_t i=0;i<g.size();++i)
        if(degree[i]==0)ready.push(i);
    while (!ready.empty()) {
        const auto u=ready.front();ready.pop();
        order.push_back(u);
        for(const auto& e:g[u])
            if(--degree[e.to]==0)ready.push(e.to);
    }
    if(order.size()!=g.size())return std::nullopt;
    return order;
}

// A* returns an optimal result when the supplied heuristic is admissible,
// nonnegative on this graph and the algorithm's termination rules apply.
// It permits reopening vertices rather than assuming consistency.
std::optional<std::int64_t> a_star(
    const Graph& g, std::size_t source, std::size_t goal,
    std::span<const std::int64_t> heuristic) {
    if(source>=g.size()||goal>=g.size()||heuristic.size()!=g.size())
        throw std::invalid_argument("A* invalid dimensions");
    for(const auto h:heuristic)
        if(h<0||h>=INF)throw std::invalid_argument("A* bad heuristic value");
    using Item=std::tuple<std::int64_t,std::int64_t,std::size_t>;
    std::priority_queue<Item,std::vector<Item>,std::greater<Item>> open;
    std::vector<std::int64_t> gscore(g.size(),INF);
    gscore[source]=0; open.push({heuristic[source],0,source});
    while(!open.empty()) {
        const auto [f,d,u]=open.top();open.pop();
        (void)f;
        if(d!=gscore[u])continue;
        if(u==goal)return d;
        for(const auto& edge:g[u]){
            if(edge.weight<0)throw std::invalid_argument("negative A* edge");
            if(edge.weight>INF-d)continue;
            const auto cand=d+edge.weight;
            if(cand<gscore[edge.to]) {
                gscore[edge.to]=cand;
                if(heuristic[edge.to]>INF-cand)continue;
                open.push({cand+heuristic[edge.to],cand,edge.to});
            }
        }
    }
    return std::nullopt;
}

// Bellman-Ford handles negative edges; an improving pass after V-1 rounds
// proves a reachable negative cycle under the standard finite-distance model.
std::optional<std::vector<std::int64_t>> bellman_ford(
    const Graph& g, std::size_t source) {
    if(source>=g.size())throw std::out_of_range("Bellman source");
    std::vector<std::int64_t> dist(g.size(),INF);
    dist[source]=0;
    for(std::size_t pass=1;pass<g.size();++pass) {
        bool changed=false;
        for(std::size_t u=0;u<g.size();++u)if(dist[u]!=INF)
            for(const auto& e:g[u]){
                if((e.weight<0 && dist[u]<-INF-e.weight) ||
                   (e.weight>0 && dist[u]> INF-e.weight))continue;
                const auto candidate=dist[u]+e.weight;
                if(candidate<dist[e.to]){
                    dist[e.to]=candidate;
                    changed=true;
                }
            }
        if(!changed)break;
    }
    for(std::size_t u=0;u<g.size();++u)if(dist[u]!=INF)
        for(const auto& e:g[u])
            if(!((e.weight<0 && dist[u]<-INF-e.weight) ||
                 (e.weight>0 && dist[u]> INF-e.weight)) &&
               dist[u]+e.weight<dist[e.to])return std::nullopt;
    return dist;
}

// All-pairs O(V^3) oracle on small graphs, suitable for testing other
// shortest-path algorithms with nonnegative edges and bounded sums.
std::vector<std::vector<std::int64_t>> floyd_warshall(const Graph& g) {
    const auto n=g.size();
    std::vector<std::vector<std::int64_t>> d(
        n,std::vector<std::int64_t>(n,INF));
    for(std::size_t i=0;i<n;++i) {
        d[i][i]=0;
        for(const auto& e:g[i])
            d[i][e.to]=std::min(d[i][e.to],e.weight);
    }
    for(std::size_t k=0;k<n;++k)
        for(std::size_t i=0;i<n;++i)if(d[i][k]!=INF)
            for(std::size_t j=0;j<n;++j)if(d[k][j]!=INF &&
                d[i][k]<=INF-d[k][j])
                    d[i][j]=std::min(d[i][j],d[i][k]+d[k][j]);
    return d;
}

struct WeightedUndirectedEdge {
    std::size_t a,b;
    std::int64_t weight;
};
std::int64_t kruskal(std::size_t n, std::vector<WeightedUndirectedEdge> edges) {
    std::sort(edges.begin(),edges.end(),[](const auto& a,const auto& b){
        return a.weight<b.weight;
    });
    DisjointSet dsu(n);
    std::int64_t cost=0;
    std::size_t selected=0;
    for(const auto& e:edges) {
        if(e.a>=n||e.b>=n)throw std::out_of_range("MST vertex");
        if(dsu.unite(e.a,e.b)){
            cost+=e.weight; // Test workload uses bounded nonnegative weights.
            ++selected;
        }
    }
    if(n>0 && selected!=n-1)throw std::runtime_error("disconnected MST");
    return cost;
}

// For nonnegative counts, an EXCLUSIVE scan yields nonoverlapping output
// offsets. Inclusive scan stores the running total after adding each item.
std::pair<std::vector<std::uint64_t>,std::uint64_t>
exclusive_scan(std::span<const std::uint64_t> counts) {
    std::vector<std::uint64_t> offsets;
    offsets.reserve(counts.size());
    std::uint64_t sum=0;
    for (const auto n:counts) {
        offsets.push_back(sum);
        if(n>std::numeric_limits<std::uint64_t>::max()-sum)
            throw std::overflow_error("prefix sum overflow");
        sum+=n;
    }
    return {offsets,sum};
}

// Interleave two 16-bit coordinates: x bits occupy 0,2,4,...;
// y bits occupy 1,3,5,... . Morton proximity is not a metric guarantee.
std::uint32_t morton2d(std::uint16_t x,std::uint16_t y) {
    auto spread=[](std::uint32_t v) {
        v=(v|(v<<8))&0x00FF00FFu;
        v=(v|(v<<4))&0x0F0F0F0Fu;
        v=(v|(v<<2))&0x33333333u;
        v=(v|(v<<1))&0x55555555u;
        return v;
    };
    return spread(x) | (spread(y)<<1);
}

// Owned LRU entries are linked in access order. Map iterators remain valid
// while list nodes are spliced. Expected O(1) operations rely on hashing,
// allocation and the standard containers' documented complexity conditions.
class Lru {
    using Item=std::pair<int,int>;
    std::size_t capacity_;
    std::list<Item> age_;
    std::unordered_map<int,typename std::list<Item>::iterator> index_;
public:
    explicit Lru(std::size_t capacity):capacity_(capacity){
        require(capacity>0,"LRU positive capacity");
    }
    void put(int key,int value) {
        if(auto found=index_.find(key);found!=index_.end()){
            found->second->second=value;
            age_.splice(age_.begin(),age_,found->second);
            return;
        }
        age_.push_front({key,value});
        try {
            index_.emplace(key,age_.begin());
        } catch (...) {
            age_.pop_front(); // Roll back list insertion.
            throw;
        }
        if(age_.size()>capacity_){
            const auto key_to_evict=age_.back().first;
            index_.erase(key_to_evict);
            age_.pop_back();
        }
    }
    std::optional<int> get(int key) {
        const auto found=index_.find(key);
        if(found==index_.end())return std::nullopt;
        age_.splice(age_.begin(),age_,found->second);
        return found->second->second;
    }
    std::size_t size() const noexcept{return age_.size();}
};

// DP proves greedy coin picking can fail: for denominations {1,3,4},
// target 6 needs two 3s, while greedy 4+1+1 uses three coins.
int min_coins(const std::vector<int>& coins,int total) {
    if(total<0)throw std::invalid_argument("negative coin target");
    std::vector<int> dp(static_cast<std::size_t>(total)+1,total+1);
    dp[0]=0;
    for(int t=1;t<=total;++t)
        for(int c:coins) {
            if(c<=0)throw std::invalid_argument("nonpositive coin");
            if(c<=t)dp[t]=std::min(dp[t],dp[t-c]+1);
        }
    return dp[total]>total?-1:dp[total];
}

// k=1 reservoir sample: at step i (zero-based), replace with probability
// 1/(i+1). Reproducibility needs a fixed generator and program/runtime.
template<class Generator>
int reservoir_one(std::span<const int> values,Generator& generator) {
    if(values.empty())throw std::invalid_argument("empty reservoir");
    int answer=values.front();
    for(std::size_t i=1;i<values.size();++i) {
        std::uniform_int_distribution<std::size_t> choose(0,i);
        if(choose(generator)==0)answer=values[i];
    }
    return answer;
}

void test_ring_and_hash() {
    Ring<int,3> r;
    require(!r.pop(),"empty FIFO");
    require(r.push(10)&&r.push(20)&&r.push(30),"fill FIFO");
    require(!r.push(40),"full FIFO");
    require(r.pop()==10 && r.pop()==20,"FIFO order");
    require(r.push(40)&&r.push(50),"wraparound pushes");
    require(r.pop()==30 && r.pop()==40 && r.pop()==50,"wraparound order");
    require(!r.pop(),"empty after wrap");

    LinearMap map;
    for(int i=0;i<120;++i)map.insert(i*8, i+1);
    require(map.capacity()>8 && map.size()==120,"rehash size");
    for(int i=0;i<120;++i)require(map.get(i*8)==i+1,"rehash lookup");
    for(int i=0;i<120;i+=2)require(map.erase(i*8),"erase");
    for(int i=0;i<120;++i)
        require(map.get(i*8).has_value()==(i%2!=0),"tombstone chain");
    for(int i=0;i<60;++i)map.insert(i*8,900+i);
    for(int i=0;i<60;++i)
        require(map.get(i*8)==900+i,"reinsert and update with tombstones");

    Bloom bloom;
    for(std::uint32_t i=0;i<100;++i)bloom.add(i*3);
    for(std::uint32_t i=0;i<100;++i)
        require(bloom.possibly_contains(i*3),"Bloom false negative");
    // Intentionally no assertion that absent values always report false.
}

void test_graphs_and_union() {
    Graph g(4);
    add_directed(g,0,1,2);add_directed(g,0,2,5);
    add_directed(g,1,2,1);add_directed(g,1,3,10);
    add_directed(g,2,3,1);
    require(bfs_hops(g,0)==std::vector<int>({0,1,1,2}),"BFS hops");
    const auto d=dijkstra(g,0);
    require(d==std::vector<std::int64_t>({0,2,3,4}),"Dijkstra");
    const std::array<std::int64_t,4> admissible{4,2,1,0};
    require(a_star(g,0,3,admissible)==4,"admissible A*");
    require(bellman_ford(g,0)==d,"Bellman equals Dijkstra on nonnegative");
    require(floyd_warshall(g)[0]==d,"Floyd-Warshall oracle");
    const auto order=topological_sort(g);
    require(order && order->front()==0 && order->back()==3,
            "DAG topological order");
    add_directed(g,3,0,1);
    require(!topological_sort(g),"directed cycle detected");

    Graph with_negative(3);
    add_directed(with_negative,0,1,2);
    add_directed(with_negative,1,2,-5);
    require(bellman_ford(with_negative,0)==
            std::vector<std::int64_t>({0,2,-3}),"negative edge path");
    add_directed(with_negative,2,1,1);
    require(!bellman_ford(with_negative,0),"negative cycle rejected");

    DisjointSet groups(5);
    require(groups.unite(0,1) && groups.unite(1,2),"union by size");
    require(!groups.unite(0,2),"redundant union");
    require(groups.find(2)==groups.find(0),"path compressed connectivity");
    require(groups.find(4)!=groups.find(0),"unconnected set");

    std::vector<WeightedUndirectedEdge> roads{
        {0,1,1},{1,2,2},{2,3,3},{0,3,10},{0,2,7}};
    require(kruskal(4,roads)==6,"Kruskal MST oracle");

    // Deterministic differential test: all-pairs independent O(V^3)
    // reference against repeated single-source Dijkstra on 48 graphs.
    std::mt19937 generator{417u};
    std::uniform_int_distribution<int> weight(1,15);
    std::bernoulli_distribution include(0.42);
    for(int trial=0;trial<48;++trial){
        Graph random(6);
        for(std::size_t u=0;u<random.size();++u)
            for(std::size_t v=0;v<random.size();++v)
                if(u!=v && include(generator))
                    add_directed(random,u,v,weight(generator));
        const auto reference=floyd_warshall(random);
        for(std::size_t source=0;source<random.size();++source)
            require(dijkstra(random,source)==reference[source],
                    "random differential shortest path");
    }
}

void test_scan_sort_cache_and_randomness() {
    const std::array<std::uint64_t,4> counts{3,0,4,2};
    const auto [offsets,total]=exclusive_scan(counts);
    require(offsets==std::vector<std::uint64_t>({0,3,3,7}) &&
            total==9,"exclusive scan");
    bool overflow=false;
    const std::array<std::uint64_t,2> large{
        std::numeric_limits<std::uint64_t>::max(),1};
    try{(void)exclusive_scan(large);}
    catch(const std::overflow_error&){overflow=true;}
    require(overflow,"scan overflow rejected");
    require(morton2d(1,2)==9 && morton2d(0,0)==0,
            "Morton coordinate interleaving");

    Lru cache{2};
    cache.put(1,100);cache.put(2,200);
    require(cache.get(1)==100,"LRU read hit");
    cache.put(3,300);
    require(!cache.get(2)&&cache.get(1)==100&&cache.get(3)==300,
            "least recently used eviction");
    require(cache.size()==2,"LRU capacity");

    require(min_coins({1,3,4},6)==2,"DP defeats naive greedy");
    std::vector<int> input{1,2,3,4,5,6,7};
    std::mt19937 a{7u},b{7u};
    require(reservoir_one(input,a)==reservoir_one(input,b),
            "fixed seed reproducibility");
    std::mt19937 shuffle_generator{17u};
    std::shuffle(input.begin(),input.end(),shuffle_generator);
    std::sort(input.begin(),input.end());
    require(input==std::vector<int>({1,2,3,4,5,6,7}),
            "Fisher-Yates library shuffle preserves multiset");
    const auto lower=std::lower_bound(input.begin(),input.end(),4);
    require(lower!=input.end() && *lower==4,"lower bound contract");
}
} // namespace allmight::phase5

int main() {
    using namespace allmight::phase5;
    try {
        test_ring_and_hash();
        test_graphs_and_union();
        test_scan_sort_cache_and_randomness();
        std::cout<<"phase5_algorithm_workload_lab: all invariants passed\n";
    } catch(const std::exception& error) {
        std::cerr<<"phase5_algorithm_workload_lab: FAIL: "<<error.what()<<"\n";
        return 1;
    }
    return 0;
}
