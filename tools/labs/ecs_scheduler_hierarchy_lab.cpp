// PROJECTALLMIGHT ECS scheduling + hierarchy reference.
// Proves conflict classification, deterministic parallel batches, and dirty world-transform propagation.
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

using Mask = std::uint64_t;
constexpr Mask Position = 1ull << 0;
constexpr Mask Velocity = 1ull << 1;
constexpr Mask Health   = 1ull << 2;
constexpr Mask Render   = 1ull << 3;

struct System {
    std::string name;
    Mask reads{};
    Mask writes{};
};

bool conflicts(const System& a,const System& b){
    // Two readers commute. Any write that overlaps the other system's read or write set conflicts.
    return (a.writes & (b.reads | b.writes)) != 0 ||
           (b.writes & (a.reads | a.writes)) != 0;
}

std::vector<std::vector<std::size_t>> deterministic_batches(const std::vector<System>& systems){
    std::vector<std::vector<std::size_t>> batches;
    for(std::size_t i=0;i<systems.size();++i){
        bool placed=false;
        for(auto& batch:batches){
            bool ok=true;
            for(std::size_t j:batch){
                if(conflicts(systems[i],systems[j])) { ok=false; break; }
            }
            if(ok){ batch.push_back(i); placed=true; break; }
        }
        if(!placed) batches.push_back({i});
    }
    return batches;
}

struct Node {
    int parent=-1;
    double local=0.0;  // 1D translation keeps the ownership/order contract visible.
    double world=0.0;
    bool dirty=true;
};

void mark_subtree_dirty(std::vector<Node>& nodes,int root){
    nodes.at(static_cast<std::size_t>(root)).dirty=true;
    for(std::size_t i=0;i<nodes.size();++i){
        if(nodes[i].parent==root) mark_subtree_dirty(nodes,static_cast<int>(i));
    }
}

void update_world(std::vector<Node>& nodes,int index){
    Node& n=nodes.at(static_cast<std::size_t>(index));
    if(!n.dirty) return;
    double parent_world=0.0;
    if(n.parent>=0){
        update_world(nodes,n.parent);
        parent_world=nodes.at(static_cast<std::size_t>(n.parent)).world;
    }
    n.world=parent_world+n.local;
    n.dirty=false;
}

void test_scheduler(){
    const std::vector<System> systems{
        {"integrate", Position|Velocity, Position},
        {"damage", Health, Health},
        {"cull", Position, Render},
        {"render-read", Position|Render, 0},
        {"velocity-read", Velocity, 0},
    };
    require(conflicts(systems[0],systems[2]),"position writer must conflict with culling position read");
    require(!conflicts(systems[1],systems[4]),"disjoint health and velocity systems should commute");
    require(!conflicts(systems[3],systems[4]),"read-only disjoint systems should commute");

    const auto batches=deterministic_batches(systems);
    require(!batches.empty(),"scheduler returned no batches");
    for(const auto& batch:batches){
        for(std::size_t a=0;a<batch.size();++a)
            for(std::size_t b=a+1;b<batch.size();++b)
                require(!conflicts(systems[batch[a]],systems[batch[b]]),
                        "parallel batch contains a conflicting pair");
    }
    // Deterministic first-fit should place damage with integrate because they are disjoint.
    require(batches[0].size()>=2 && batches[0][0]==0 && batches[0][1]==1,
            "deterministic batching contract changed");
}

void test_hierarchy(){
    std::vector<Node> nodes{
        {-1,10.0,0.0,true}, // root
        { 0, 3.0,0.0,true}, // child
        { 1, 2.0,0.0,true}, // grandchild
        { 0,-4.0,0.0,true}, // sibling
    };
    for(int i=0;i<4;++i) update_world(nodes,i);
    require(nodes[0].world==10.0 && nodes[1].world==13.0 &&
            nodes[2].world==15.0 && nodes[3].world==6.0,
            "initial hierarchy propagation wrong");

    nodes[1].local=5.0;
    mark_subtree_dirty(nodes,1);
    require(!nodes[0].dirty && nodes[1].dirty && nodes[2].dirty && !nodes[3].dirty,
            "dirty propagation touched wrong nodes");
    update_world(nodes,2);
    require(nodes[1].world==15.0 && nodes[2].world==17.0,
            "child-before-descendant recomputation failed");
    require(nodes[3].world==6.0,"unrelated sibling changed");
}

} // namespace

int main(){
    try{
        test_scheduler();
        test_hierarchy();
        std::cout
            <<"ecs_scheduler_hierarchy_lab: PASS\n"
            <<"proved: read/write conflict symmetry, deterministic conflict-free batches, parent-first world propagation, subtree-only invalidation\n"
            <<"not proved: lock-free structural mutation, job stealing, archetype chunk ownership, cycle repair, multithreaded memory ordering, or current KairoECS parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"ecs_scheduler_hierarchy_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
