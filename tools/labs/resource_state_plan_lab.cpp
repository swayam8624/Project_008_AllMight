// C++20. Symbolic resource hazards: NOT valid Vulkan barrier emission.
// Run: c++ -std=c++20 -Wall -Wextra -Werror -pedantic -O2 resource_state_plan_lab.cpp -o planner && ./planner
#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

enum class Queue { Transfer, Graphics };
enum class Stage { Transfer, Compute, Fragment, ColorOutput };
enum class Access { Read, Write };
enum class Layout { Undefined, TransferDst, ShaderRead, ColorAttachment };

struct Key {
    std::string image;
    unsigned mip = 0, layer = 0;
    bool operator<(const Key& rhs) const {
        return std::tie(image, mip, layer) < std::tie(rhs.image, rhs.mip, rhs.layer);
    }
};
struct Image {
    std::string name;
    unsigned mips=1, layers=1;
    bool imported=false;
    Layout initial=Layout::Undefined;
    Queue owner=Queue::Graphics;
};
struct Use { Key key; Queue queue; Stage stage; Access access; Layout layout; };
struct Pass { std::string name; std::vector<Use> uses; };
enum class Kind { Initialize, SameQueue, Release, Acquire };
struct Transition {
    Kind kind;
    std::size_t before, after; Key key;
    Queue from, to; Layout oldLayout, newLayout;
    std::set<Stage> producerStages;
    Stage consumerStage;
    bool memoryWrite;
    unsigned handoff=0;
};
struct State {
    bool initialized=false;
    Layout layout=Layout::Undefined;
    Queue owner=Queue::Graphics;
    std::size_t previousPass=0;
    bool priorWrite=false;
    std::set<Stage> readers, writers;
};
void require(bool ok,const std::string& why) {
    if(!ok) throw std::runtime_error(why);
}
std::vector<Transition> plan(const std::vector<Image>& images,
                             const std::vector<Pass>& passes) {
    std::map<Key, State> states; // independent state for every mip/layer
    for(const Image& image:images) {
        require(!image.name.empty() && image.mips>0 && image.layers>0,
                "invalid image declaration");
        for(unsigned m=0;m<image.mips;++m) for(unsigned l=0;l<image.layers;++l) {
            Key key{image.name,m,l};
            State state;
            state.initialized=image.imported;
            state.layout=image.imported?image.initial:Layout::Undefined;
            state.owner=image.owner;
            require(states.emplace(key,state).second,"duplicate image");
        }
    }
    std::vector<Transition> result;
    unsigned nextHandoff=1;
    for(std::size_t i=0;i<passes.size();++i) {
        require(!passes[i].name.empty(),"unnamed pass");
        std::set<Key> inPass;
        for(const Use& use:passes[i].uses) {
            auto it=states.find(use.key);
            require(it!=states.end(),"unknown image, mip or layer");
            require(inPass.insert(use.key).second,
                    "same-subresource feedback needs richer pass declarations");
            require(use.layout!=Layout::Undefined,"cannot access undefined layout");
            if(use.layout==Layout::TransferDst)
                require(use.stage==Stage::Transfer && use.access==Access::Write,
                        "transfer destination misuse");
            if(use.layout==Layout::ColorAttachment)
                require(use.stage==Stage::ColorOutput && use.access==Access::Write,
                        "color output misuse");
            if(use.layout==Layout::ShaderRead)
                require(use.access==Access::Read &&
                       (use.stage==Stage::Fragment || use.stage==Stage::Compute),
                        "shader sampled read misuse");
            State& state=it->second;
            require(state.initialized || use.access==Access::Write,
                    "uninitialized read");
            std::set<Stage> producers=state.readers;
            producers.insert(state.writers.begin(),state.writers.end());
            if(!state.initialized) {
                // First write establishes one subresource's initialized state.
                result.push_back({Kind::Initialize,i,i,use.key,
                                  use.queue,use.queue,Layout::Undefined,
                                  use.layout,{},use.stage,false,0});
            } else if(state.owner!=use.queue) {
                const unsigned handoff=nextHandoff++;
                // Symbolic release/acquire pair. Real Vulkan requires actual
                // VkImageMemoryBarrier2 on each queue + matching submission
                // semaphore ordering and correct scope/layout legality.
                result.push_back({Kind::Release,state.previousPass,i,use.key,
                                  state.owner,use.queue,state.layout,use.layout,
                                  producers,use.stage,state.priorWrite,handoff});
                result.push_back({Kind::Acquire,state.previousPass,i,use.key,
                                  state.owner,use.queue,state.layout,use.layout,
                                  producers,use.stage,state.priorWrite,handoff});
            } else if(state.layout!=use.layout ||
                      state.priorWrite || use.access==Access::Write) {
                // Consecutive read-only operations at identical layout and
                // queue intentionally do not create a new transition.
                result.push_back({Kind::SameQueue,state.previousPass,i,use.key,
                                  state.owner,use.queue,state.layout,use.layout,
                                  producers,use.stage,state.priorWrite,0});
            }
            if(use.access==Access::Write) {
                state.readers.clear();
                state.writers={use.stage};
            } else {
                state.readers.insert(use.stage);
            }
            state.priorWrite=(use.access==Access::Write);
            state.previousPass=i;
            state.initialized=true;
            state.layout=use.layout;
            state.owner=use.queue;
        }
    }
    return result;
}
int main() {
    const Key mip0{"hdr",0,0},mip1{"hdr",1,0};
    const std::vector<Image> images{
        {"hdr",2,1,false,Layout::Undefined,Queue::Graphics}
    };
    const auto use=[](Key key,Queue q,Stage s,Access a,Layout l) {
        return Use{key,q,s,a,l};
    };
    const auto transitions=plan(images,{
        {"upload0",{use(mip0,Queue::Transfer,Stage::Transfer,Access::Write,Layout::TransferDst)}},
        {"sample0",{use(mip0,Queue::Graphics,Stage::Fragment,Access::Read,Layout::ShaderRead)}},
        {"sample0-again",{use(mip0,Queue::Graphics,Stage::Fragment,Access::Read,Layout::ShaderRead)}},
        {"overwrite0",{use(mip0,Queue::Graphics,Stage::ColorOutput,Access::Write,Layout::ColorAttachment)}},
        {"upload1",{use(mip1,Queue::Transfer,Stage::Transfer,Access::Write,Layout::TransferDst)}}
    });
    require(transitions.size()==5,"unexpected transition count");
    require(transitions[0].kind==Kind::Initialize,"missing first-write initialization");
    require(transitions[1].kind==Kind::Release &&
            transitions[2].kind==Kind::Acquire,"missing ownership pair");
    require(transitions[1].handoff==transitions[2].handoff,
            "release/acquire handoff IDs differ");
    require(transitions[3].kind==Kind::SameQueue &&
            transitions[3].after==3,"write-after-read transition missing");
    require(transitions[4].kind==Kind::Initialize &&
            transitions[4].key.mip==1,"mip isolation violated");
    bool rejected=false;
    try {
        (void)plan(images,{{"bad",{use(mip1,Queue::Graphics,Stage::Fragment,
                                     Access::Read,Layout::ShaderRead)}}});
    } catch(const std::runtime_error&) {rejected=true;}
    require(rejected,"read of uninitialized mip accepted");
    rejected=false;
    try {
        (void)plan(images,{{"feedback",{
            use(mip0,Queue::Graphics,Stage::Fragment,Access::Read,Layout::ShaderRead),
            use(mip0,Queue::Graphics,Stage::ColorOutput,Access::Write,Layout::ColorAttachment)
        }}});
    } catch(const std::runtime_error&) {rejected=true;}
    require(rejected,"ambiguous same-pass feedback accepted");
    std::cout<<"planned="<<transitions.size()
             <<" symbolic transitions; ownership pair, mip isolation and negatives PASS\n";
}
