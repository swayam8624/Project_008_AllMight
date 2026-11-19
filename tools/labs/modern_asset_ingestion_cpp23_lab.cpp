// Pass18: filesystem/ranges/format/coroutine + host-threaded asset flow.
// The primary worked clients are Maveb/Kairo texture/mesh asset boundaries.
// If the original Pass06 registry header is provided by the real course,
// this executable additionally imports the same TXR4/MSH1 fixture bytes
// through the ACTUAL earlier typed asset decoders/owner-aware registry.
#include "foundations_course/modern_asset_ingestion.hpp"
#if __has_include("nes_course/generational_asset_registry.hpp")
#include "nes_course/generational_asset_registry.hpp"
#define ALLMIGHT_HAS_REGISTRY 1
#else
#define ALLMIGHT_HAS_REGISTRY 0
#endif
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <span>
#include <stop_token>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace a=allmight::foundations::ingestion;
namespace fs=std::filesystem;
void require(bool ok,std::string_view message) {
    if(!ok) throw std::runtime_error(std::string(message));
}
struct Fixture {
    fs::path root;
    Fixture() {
        const auto clock=std::chrono::steady_clock::now().time_since_epoch().count();
        root=fs::temp_directory_path()/ ("allmight-cpp23-pass18-"+std::to_string(clock));
        if(!fs::create_directory(root))throw std::runtime_error("temp root already exists");
        fs::create_directory(root/"assets");
        fs::create_directory(root/"models");
    }
    ~Fixture() {
        std::error_code ec;
        fs::remove_all(root,ec);
    }
    Fixture(const Fixture&)=delete;
    Fixture& operator=(const Fixture&)=delete;
    void file(std::string_view relative,std::span<const a::Byte> bytes) const {
        std::ofstream out(root/fs::path(relative),std::ios::binary|std::ios::trunc);
        if(!out)throw std::runtime_error("create binary fixture");
        for(auto byte:bytes)out.put(static_cast<char>(byte));
        out.flush();
        if(!out)throw std::runtime_error("write binary fixture");
    }
};
std::vector<a::Byte> texture() {
    std::vector<a::Byte> bytes{'T','X','R','4',2,0,1,0,0,0,0,0};
    bytes.insert(bytes.end(),{255,3,4,255, 7,8,9,255});
    return bytes;
}
std::vector<a::Byte> mesh() {
    // Three vertices (6 LE bytes each), one triangle (3 u16 indices).
    std::vector<a::Byte> bytes{'M','S','H','1',3,0,3,0,0,0,0,0};
    bytes.insert(bytes.end(),{
        0,0,0,0,0,0,  0,1,0,0,0,0,  0,0,0,1,0,0,
        0,0,1,0,2,0
    });
    return bytes;
}
void test_real_bounded_file_io_and_path_policy() {
    Fixture fixture;
    const auto tx=texture(), ms=mesh();
    fixture.file("assets/tex.txr4",tx);
    fixture.file("models/tri.msh1",ms);
    const a::Root root(fixture.root,64);
    const auto good=root.load({"assets/tex.txr4",a::AssetKind::texture_rgba8});
    require(good && good->bytes==tx && good->checksum==a::fnv1a(tx),
            "binary file read preserves byte order/size and independent checksum");
    auto rejection=[&](std::string relative,a::Issue reason){
        const auto result=root.load({std::move(relative),a::AssetKind::triangle_mesh});
        require(!result && result.error()==reason,"path must fail with exact issue");
    };
    rejection("../outside.msh1",a::Issue::unsafe_relative_path);
    rejection("./assets/tex.txr4",a::Issue::unsafe_relative_path);
    rejection("assets/../models/tri.msh1",a::Issue::unsafe_relative_path);
    rejection("/etc/passwd",a::Issue::unsafe_relative_path);
    rejection("",a::Issue::unsafe_relative_path);
    rejection(std::string("models/tri.msh1\0/evil",20),a::Issue::unsafe_relative_path);
    rejection("missing.msh1",a::Issue::missing_file);
    rejection("assets",a::Issue::not_regular);
    const auto limited=a::Root(fixture.root,16).load({"assets/tex.txr4",a::AssetKind::texture_rgba8});
    require(!limited && limited.error()==a::Issue::too_large,"bounded file cap rejects header+payload");
    // POSIX/macOS can normally create symlinks; Windows security settings
    // may prevent it, so only assert rejection after creation succeeded.
    std::error_code ec;
    fs::create_symlink(fixture.root/"models/tri.msh1",fixture.root/"assets/link",ec);
    if(!ec)rejection("assets/link",a::Issue::symlink);
}
void test_coroutine_owned_bytes_ranges_and_cancellation() {
    Fixture fixture;
    const auto tx=texture(),ms=mesh();
    fixture.file("assets/tex.txr4",tx);
    fixture.file("models/tri.msh1",ms);
    const a::Root root(fixture.root,64);
    std::vector<a::Request> original{{"models/tri.msh1",a::AssetKind::triangle_mesh},
                                     {"assets/tex.txr4",a::AssetKind::texture_rgba8},
                                     {"ignore.tmp",a::AssetKind::triangle_mesh,false}};
    std::stop_source source;
    auto task=a::make_batch(root,original,source.get_token());
    original.clear(); // coroutine copied its manifest, no dangling view.
    auto first=task.next();
    require(first && *first && (*first)->relative=="assets/tex.txr4" &&
            (*first)->bytes==tx,"std::ranges sorting and owned coroutine yield");
    auto second=task.next();
    require(second && *second && (*second)->relative=="models/tri.msh1" &&
            (*second)->bytes==ms,"independent mesh result fully owned");
    require(!task.next(),"generator terminates with no dangling buffers");
    // Exact semantic of copying requests into coroutine *frame*:
    // yielded bytes survive a later source-file removal.
    fs::remove(fixture.root/"assets/tex.txr4");
    require((*first)->bytes==tx,"yield owns source bytes after filesystem mutation");

    std::stop_source before;
    before.request_stop();
    auto stopped=a::make_batch(root,{{"models/tri.msh1",a::AssetKind::triangle_mesh}},
                               before.get_token());
    auto cancelled=stopped.next();
    require(cancelled && !*cancelled &&
            cancelled->error()==a::Issue::cancelled && !stopped.next(),
            "cooperative stop before first file gives one explicit cancellation");

    auto duplicate=a::make_batch(root,{{"models/tri.msh1",a::AssetKind::triangle_mesh},
                                       {"models/tri.msh1",a::AssetKind::triangle_mesh}},
                                 source.get_token());
    auto ok=duplicate.next();
    auto repeated=duplicate.next();
    require(ok && *ok && repeated && !*repeated &&
            repeated->error()==a::Issue::duplicate_request,
            "repeated source names cannot be silently published twice");
}
void test_coawait_suspend_resume_and_frame_ownership() {
    auto shared=std::make_shared<std::atomic<int>>(0);
    std::weak_ptr<std::atomic<int>> weak=shared;
    std::stop_source source;
    auto task=a::publish_after_checkpoint(shared,source.get_token(),42);
    shared.reset(); // coroutine copied the owning shared_ptr parameter
    require(!weak.expired() && task.resume(),"co_await suspended with a live owning frame");
    require(!task.resume() && !weak.expired() && weak.lock()->load()==42,
            "explicit caller resume publishes only after suspension");
    // Cancellation after suspension blocks side effects even if resumed.
    auto value=std::make_shared<std::atomic<int>>(0);
    std::stop_source cancel;
    auto stopped=a::publish_after_checkpoint(value,cancel.get_token(),77);
    require(stopped.resume(),"paused before cancellation");
    cancel.request_stop();
    require(!stopped.resume() && value->load()==0,
            "post-suspension cooperative stop suppresses publication");
}
void test_real_host_async_batch() {
    Fixture fixture;
    fixture.file("assets/tex.txr4",texture());
    fixture.file("models/tri.msh1",mesh());
    a::WorkerBatch worker(a::Root(fixture.root,64),
        {{"assets/tex.txr4",a::AssetKind::texture_rgba8},
         {"models/tri.msh1",a::AssetKind::triangle_mesh}});
    const auto results=worker.finish();
    require(results.size()==2 && results[0] && results[1] &&
            results[0]->relative=="assets/tex.txr4" &&
            results[1]->relative=="models/tri.msh1",
            "actual jthread resumes coroutine once; owned results returned after join");
}
void test_aggregate_budget_prevents_unbounded_publication() {
    Fixture fixture;
    fixture.file("assets/tex.txr4",texture());
    fixture.file("models/tri.msh1",mesh());
    bool refused_zero_budget=false;
    try {
        a::WorkerBatch invalid(a::Root(fixture.root,64),{},0);
    } catch(const std::invalid_argument&) {
        refused_zero_budget=true;
    }
    require(refused_zero_budget,"zero budget refused before launching worker");
    a::WorkerBatch batch(a::Root(fixture.root,64),
        {{"assets/tex.txr4",a::AssetKind::texture_rgba8},
         {"models/tri.msh1",a::AssetKind::triangle_mesh}},24);
    const auto result=batch.finish();
    require(result.size()==2 && result[0] && !result[1] &&
            result[1].error()==a::Issue::aggregate_budget,
            "worker refuses next owned asset when cumulative byte budget exhausted");
}
void test_existing_registry_integration() {
#if ALLMIGHT_HAS_REGISTRY
    // This is the ACTUAL shared typed NROM/TXR4/MSH1 decoder + generational
    // registry from Pass05/06, not a copied mock of Kairo asset publication.
    namespace reg=allmight::nes::assets;
    Fixture fixture;
    fixture.file("assets/tex.txr4",texture());
    fixture.file("models/tri.msh1",mesh());
    a::Root root(fixture.root,64);
    auto t=root.load({"assets/tex.txr4",a::AssetKind::texture_rgba8});
    auto m=root.load({"models/tri.msh1",a::AssetKind::triangle_mesh});
    require(t && m,"file layer accepted two real typed formats");
    reg::GenerationalAssetRegistry registry(2);
    const auto tid=registry.import<reg::TextureTag>(t->bytes);
    const auto mid=registry.import<reg::MeshTag>(m->bytes);
    require(tid && mid && registry.active_count()==2,
            "real earlier decoder and generational publication succeed");
    const auto image=registry.lookup<reg::TextureTag>(*tid);
    const auto mesh=registry.lookup<reg::MeshTag>(*mid);
    require(image && image->get().width==2 && image->get().height==1 &&
            image->get().pixels.size()==8 && mesh &&
            mesh->get().vertices.size()==3 && mesh->get().indices.size()==3,
            "published typed asset retains decoded semantics");
    require(registry.erase<reg::TextureTag>(*tid).has_value(),
            "real registry erases owned texture");
    require(!registry.lookup<reg::TextureTag>(*tid),
            "old handle stale after removal, not a reused asset pointer");
#else
    std::cout << "Standalone source-only smoke: existing registry integration "
                 "requires the course include directory; not tested here.\n";
#endif
}
int main(){
    try {
        test_real_bounded_file_io_and_path_policy();
        test_coroutine_owned_bytes_ranges_and_cancellation();
        test_coawait_suspend_resume_and_frame_ownership();
        test_real_host_async_batch();
        test_aggregate_budget_prevents_unbounded_publication();
        test_existing_registry_integration();
        a::announce_success(2);
        std::cout << "Pass18: filesystem/ranges/format/coroutines and host jthread "
                     "checks passed; native sandbox TOCTOU defenses NOT proven.\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr << "Pass18 failed: " << e.what() << '\n';
        return 1;
    }
}
