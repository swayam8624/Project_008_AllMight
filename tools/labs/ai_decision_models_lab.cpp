#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct World {
    int health = 100;
    int ammo = 0;
    bool enemy_visible = false;
    bool has_weapon = false;
    bool enemy_defeated = false;
};

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

// ---- Behavior tree: authored control structure ----
enum class Status { Success, Failure, Running };

Status behavior_tree_decide(const World& w, std::string& action) {
    // Selector priority is explicit: survive first, then attack, then search.
    if (w.health < 30) {
        action = "seek_cover";
        return Status::Success;
    }
    if (w.enemy_visible && w.has_weapon && w.ammo > 0) {
        action = "attack";
        return Status::Success;
    }
    action = "search";
    return Status::Success;
}

// ---- Utility AI: score alternatives from current facts ----
std::string utility_decide(const World& w) {
    struct Candidate { std::string name; double score; };
    std::vector<Candidate> c{
        {"seek_cover", std::clamp((40.0 - w.health) / 40.0, 0.0, 1.0)},
        {"attack", (w.enemy_visible && w.has_weapon && w.ammo > 0) ? 0.85 : 0.0},
        {"search", w.enemy_visible ? 0.1 : 0.55},
    };
    return std::max_element(c.begin(), c.end(),
        [](const Candidate& a, const Candidate& b) {
            if (a.score != b.score) return a.score < b.score;
            return a.name > b.name; // deterministic tie-break
        })->name;
}

// ---- GOAP: search over symbolic actions and effects ----
struct Facts {
    bool has_weapon = false;
    bool has_ammo = false;
    bool enemy_visible = false;
    bool enemy_defeated = false;
    friend bool operator==(const Facts&, const Facts&) = default;
};

struct Action {
    std::string name;
    int cost;
    std::function<bool(const Facts&)> pre;
    std::function<Facts(Facts)> apply;
};

int encode(const Facts& f) {
    return (f.has_weapon ? 1 : 0)
         | (f.has_ammo ? 2 : 0)
         | (f.enemy_visible ? 4 : 0)
         | (f.enemy_defeated ? 8 : 0);
}

std::optional<std::vector<std::string>> goap_plan(Facts start) {
    const std::vector<Action> actions{
        {"find_weapon", 2,
            [](const Facts& f){ return !f.has_weapon; },
            [](Facts f){ f.has_weapon=true; return f; }},
        {"find_ammo", 2,
            [](const Facts& f){ return f.has_weapon && !f.has_ammo; },
            [](Facts f){ f.has_ammo=true; return f; }},
        {"find_enemy", 1,
            [](const Facts& f){ return !f.enemy_visible; },
            [](Facts f){ f.enemy_visible=true; return f; }},
        {"attack", 1,
            [](const Facts& f){
                return f.has_weapon && f.has_ammo && f.enemy_visible && !f.enemy_defeated;
            },
            [](Facts f){ f.enemy_defeated=true; f.has_ammo=false; return f; }},
    };

    struct Node {
        int cost;
        Facts facts;
        std::vector<std::string> plan;
    };
    struct Greater {
        bool operator()(const Node& a, const Node& b) const {
            if (a.cost != b.cost) return a.cost > b.cost;
            return a.plan > b.plan; // deterministic queue tie-break
        }
    };

    std::priority_queue<Node,std::vector<Node>,Greater> q;
    std::array<int,16> best;
    best.fill(std::numeric_limits<int>::max());
    best[encode(start)] = 0;
    q.push({0,start,{}});

    while(!q.empty()) {
        Node n=q.top(); q.pop();
        if(n.facts.enemy_defeated) return n.plan;
        if(n.cost != best[encode(n.facts)]) continue;

        for(const auto& a:actions) {
            if(!a.pre(n.facts)) continue;
            Facts next=a.apply(n.facts);
            int cost=n.cost+a.cost;
            int key=encode(next);
            if(cost >= best[key]) continue;
            best[key]=cost;
            auto plan=n.plan;
            plan.push_back(a.name);
            q.push({cost,next,std::move(plan)});
        }
    }
    return std::nullopt;
}

int main() {
    {
        World w{20,10,true,true,false};
        std::string bt;
        require(behavior_tree_decide(w,bt)==Status::Success && bt=="seek_cover",
                "behavior-tree priority should choose survival branch");
        require(utility_decide(w)=="seek_cover",
                "utility score should also favor cover at very low health");
    }

    {
        World w{80,10,true,true,false};
        std::string bt;
        behavior_tree_decide(w,bt);
        require(bt=="attack" && utility_decide(w)=="attack",
                "both models should attack when attack is clearly useful");
    }

    {
        Facts start{};
        auto plan=goap_plan(start);
        require(plan.has_value(), "planner should find a plan");
        const std::vector<std::string> expected{
            "find_enemy","find_weapon","find_ammo","attack"
        };
        // Several equal-cost orderings can be logically valid. Verify the plan
        // reaches the goal and has the expected total number of required steps.
        require(plan->size()==4, "planner should need four prerequisite/action steps");
        Facts f=start;
        const std::unordered_map<std::string,Action> table{
            {"find_weapon",{"find_weapon",2,[](const Facts& x){return !x.has_weapon;},
                [](Facts x){x.has_weapon=true;return x;}}},
            {"find_ammo",{"find_ammo",2,[](const Facts& x){return x.has_weapon&&!x.has_ammo;},
                [](Facts x){x.has_ammo=true;return x;}}},
            {"find_enemy",{"find_enemy",1,[](const Facts& x){return !x.enemy_visible;},
                [](Facts x){x.enemy_visible=true;return x;}}},
            {"attack",{"attack",1,[](const Facts& x){return x.has_weapon&&x.has_ammo&&x.enemy_visible&&!x.enemy_defeated;},
                [](Facts x){x.enemy_defeated=true;x.has_ammo=false;return x;}}},
        };
        for(const auto& name:*plan) {
            const auto& a=table.at(name);
            require(a.pre(f), "GOAP action precondition must hold when executed");
            f=a.apply(f);
        }
        require(f.enemy_defeated, "GOAP plan must actually establish the goal");
    }

    {
        Facts impossible{false,false,false,true};
        auto plan=goap_plan(impossible);
        require(plan.has_value() && plan->empty(),
                "already-satisfied goal should produce an empty plan");
    }

    std::cout << "AI decision models: 4 suites PASS\n";
}
