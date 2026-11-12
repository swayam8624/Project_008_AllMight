// PROJECTALLMIGHT Phase 30 witness: deterministic manifest validation and staged publication planning.
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
[[noreturn]] void fail(const std::string& m){ throw std::runtime_error(m); }
void require(bool c,const std::string& m){ if(!c) fail(m); }

bool is_hex64(const std::string& s){
    if(s.size()!=64) return false;
    for(unsigned char c:s){
        if(!std::isxdigit(c)) return false;
    }
    return true;
}

std::string normalize_relative_path(const std::string& raw){
    require(!raw.empty(),"path must not be empty");
    require(raw.front()!='/' && raw.front()!='\\',"absolute paths are forbidden");
    require(raw.find(':')==std::string::npos,"drive/scheme-like path is forbidden");
    require(raw.find('\\')==std::string::npos,"manifest paths use forward slashes only");

    std::vector<std::string> parts;
    std::string current;
    auto flush=[&](){
        if(current.empty() || current=="."){ current.clear(); return; }
        require(current!="..","parent traversal is forbidden");
        parts.push_back(current);
        current.clear();
    };
    for(char c:raw){
        if(c=='/') flush();
        else current.push_back(c);
    }
    flush();
    require(!parts.empty(),"path normalized to empty");

    std::string out;
    for(std::size_t i=0;i<parts.size();++i){
        if(i) out+='/';
        out+=parts[i];
    }
    return out;
}

struct AssetRecord{
    std::string logical_id;
    std::string source_path;
    std::string output_path;
    std::string sha256;
};

struct Manifest{
    int schema_version=0;
    std::vector<AssetRecord> assets;
};

struct PublicationItem{
    std::string output_path;
    std::string sha256;
    friend bool operator==(const PublicationItem&,const PublicationItem&)=default;
};

std::vector<PublicationItem> validate_and_plan(const Manifest& m){
    require(m.schema_version==1,"unsupported manifest schema");
    require(!m.assets.empty(),"manifest must contain assets");

    std::map<std::string,std::string> id_to_output;
    std::map<std::string,std::string> output_to_hash;
    std::vector<PublicationItem> plan;

    for(const AssetRecord& a:m.assets){
        require(!a.logical_id.empty(),"asset logical id must not be empty");
        const std::string source=normalize_relative_path(a.source_path);
        const std::string output=normalize_relative_path(a.output_path);
        (void)source;
        require(is_hex64(a.sha256),"asset sha256 must be exactly 64 hexadecimal digits");

        require(id_to_output.emplace(a.logical_id,output).second,"duplicate logical asset id");
        auto [it,inserted]=output_to_hash.emplace(output,a.sha256);
        require(inserted,"two manifest records target the same output path");
        plan.push_back({output,a.sha256});
    }

    std::sort(plan.begin(),plan.end(),[](const auto& a,const auto& b){
        if(a.output_path!=b.output_path) return a.output_path<b.output_path;
        return a.sha256<b.sha256;
    });
    return plan;
}

class PublishedStore{
public:
    void commit(const std::vector<PublicationItem>& plan){
        // Validate the entire transaction before mutating the store.
        for(const auto& item:plan){
            auto it=published_.find(item.output_path);
            if(it!=published_.end()){
                require(it->second==item.sha256,
                        "publication would replace an existing path with different content without an explicit replacement policy");
            }
        }
        for(const auto& item:plan) published_[item.output_path]=item.sha256;
    }

    std::size_t size() const { return published_.size(); }
    std::string hash(const std::string& path) const { return published_.at(path); }

private:
    std::map<std::string,std::string> published_;
};

void validate_frame_sequence(const std::vector<int>& frames,int expected_first,int expected_last){
    require(expected_first<=expected_last,"invalid frame range");
    require(frames.size()==static_cast<std::size_t>(expected_last-expected_first+1),
            "frame count does not match declared range");
    for(int f=expected_first;f<=expected_last;++f){
        const std::size_t i=static_cast<std::size_t>(f-expected_first);
        require(frames[i]==f,"frame sequence has a gap, duplicate, or ordering error");
    }
}

void test_path_rules(){
    require(normalize_relative_path("assets//hero/./mesh.glb")=="assets/hero/mesh.glb","path normalization failed");
    bool traversal=false;
    try{ (void)normalize_relative_path("assets/../secret.bin"); }
    catch(const std::runtime_error&){ traversal=true; }
    require(traversal,"parent traversal must be rejected");

    bool absolute=false;
    try{ (void)normalize_relative_path("/tmp/file"); }
    catch(const std::runtime_error&){ absolute=true; }
    require(absolute,"absolute path must be rejected");
}

void test_deterministic_plan_and_idempotent_commit(){
    const std::string a(64,'a');
    const std::string b(64,'b');
    Manifest m{1,{
        {"hero.mesh","source/hero.blend","published/hero.glb",a},
        {"hero.thumb","source/hero.blend","published/hero.png",b}
    }};

    const auto first=validate_and_plan(m);
    const auto second=validate_and_plan(m);
    require(first==second,"same manifest must produce deterministic plan");

    PublishedStore store;
    store.commit(first);
    require(store.size()==2,"publish count mismatch");
    store.commit(second);
    require(store.size()==2,"idempotent republish should not duplicate outputs");
    require(store.hash("published/hero.glb")==a,"published identity mismatch");

    auto changed=first;
    changed[0].sha256=std::string(64,'c');
    bool rejected=false;
    try{ store.commit(changed); }
    catch(const std::runtime_error&){ rejected=true; }
    require(rejected,"implicit destructive replacement must be rejected");
}

void test_manifest_collisions_and_frames(){
    const std::string h(64,'1');
    bool collision=false;
    try{
        (void)validate_and_plan({1,{
            {"a","src/a","out/shared",h},
            {"b","src/b","out/shared",h}
        }});
    }catch(const std::runtime_error&){ collision=true; }
    require(collision,"duplicate output path must be rejected");

    validate_frame_sequence({1001,1002,1003,1004},1001,1004);
    bool gap=false;
    try{ validate_frame_sequence({1001,1003,1004},1001,1004); }
    catch(const std::runtime_error&){ gap=true; }
    require(gap,"missing frame must be rejected");
}

} // namespace

int main(){
    try{
        test_path_rules();
        test_deterministic_plan_and_idempotent_commit();
        test_manifest_collisions_and_frames();
        std::cout<<"production_pipeline_contract_lab: PASS\n"
                 <<"proved: portable relative-path policy, traversal rejection, schema/hash validation, deterministic dry-run plan, "
                   "idempotent publication, collision handling, and contiguous frame-sequence validation\n"
                 <<"not proved: real SHA-256 computation, filesystem symlink resolution, atomic rename semantics, Blender/Maya/Houdini/Nuke host APIs, "
                   "render-farm behavior, or production asset parity\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"production_pipeline_contract_lab: FAIL: "<<e.what()<<'\n';
        return 1;
    }
}
