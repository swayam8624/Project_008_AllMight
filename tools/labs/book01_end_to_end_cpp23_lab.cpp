// AllMight Book01 / Pass19: one portable, deterministic end-to-end acceptance trace.
// This is real C++23 host execution using the actual Pass05/06/08/09/18 headers.
// "GPU" below means the existing CPU memory-contract ORACLE, never native GPU work.
#include "foundations_course/modern_asset_ingestion.hpp"
#include "nes_course/generational_asset_registry.hpp"
#include "nes_course/work_dependency_graph.hpp"
#include "nes_course/gpu_memory_contract_oracle.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace ingestion = allmight::foundations::ingestion;
namespace assets = allmight::nes::assets;
namespace work = allmight::nes::work;
namespace memory = allmight::nes::gpu_model;
namespace fs = std::filesystem;

namespace {
using Byte = std::uint8_t;

void require(bool valid, std::string_view explanation) {
    if (!valid) throw std::runtime_error(std::string(explanation));
}

// Actual on-disk fixtures; their format is the previously implemented TXR4/MSH1,
// NOT an unsupported claim of PNG, glTF or production texture/mesh import.
struct Fixture {
    fs::path root;
    Fixture() {
        static std::atomic<std::uint64_t> sequence{0};
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto ordinal = sequence.fetch_add(1, std::memory_order_relaxed);
        root = fs::temp_directory_path() /
            ("allmight-pass19-" + std::to_string(stamp) + "-" +
             std::to_string(ordinal));
        require(fs::create_directory(root), "fresh fixture root");
        require(fs::create_directory(root / "assets"), "asset fixture directory");
        require(fs::create_directory(root / "models"), "model fixture directory");
    }
    Fixture(const Fixture&) = delete;
    Fixture& operator=(const Fixture&) = delete;
    ~Fixture() {
        std::error_code ignored;
        fs::remove_all(root, ignored);
    }
    void file(std::string_view name, std::span<const Byte> bytes) const {
        std::ofstream out(root / fs::path(name),
                          std::ios::binary | std::ios::trunc);
        require(static_cast<bool>(out), "open fixture");
        for (auto byte : bytes) out.put(static_cast<char>(byte));
        out.flush();
        require(static_cast<bool>(out), "persist fixture");
    }
};

std::vector<Byte> valid_texture() {
    // 2 x 1 RGBA8 pixels. R+2G+B is exactly 265 + 32 = 297.
    std::vector<Byte> bytes{'T','X','R','4',2,0,1,0,0,0,0,0};
    bytes.insert(bytes.end(), {255,3,4,255, 7,8,9,255});
    return bytes;
}
std::vector<Byte> valid_mesh() {
    // Quantized XY triangle: (0,0), (256,0), (0,256).
    // Its signed twice-area is 65,536 in (1/256 world unit)^2.
    std::vector<Byte> bytes{'M','S','H','1',3,0,3,0,0,0,0,0};
    bytes.insert(bytes.end(), {
        0,0,0,0,0,0,  0,1,0,0,0,0,  0,0,0,1,0,0,
        0,0,1,0,2,0
    });
    return bytes;
}

enum class PipelineError {
    read_failure, decode_failure, publication_failure,
    graph_failure, compute_failure, retirement_failure
};

struct Receipt {
    // Never include process-local registry owner IDs, temp paths or timestamps.
    // This makes independently repeated runs byte-for-byte comparable.
    std::string canonical;
    std::uint64_t texture_source_fingerprint{};
    std::uint64_t mesh_source_fingerprint{};
    std::uint64_t luma{};
    std::int64_t signed_twice_area_q16{};
};

work::ResourceKey key(assets::AssetHandle<assets::TextureTag> handle) {
    return {work::ResourceKind::texture, handle.owner,
            static_cast<std::uint64_t>(handle.slot), handle.generation};
}
work::ResourceKey key(assets::AssetHandle<assets::MeshTag> handle) {
    return {work::ResourceKind::mesh, handle.owner,
            static_cast<std::uint64_t>(handle.slot), handle.generation};
}
work::Access read(work::ResourceKey r) { return {r,true,false}; }
work::Access write(work::ResourceKey r) { return {r,false,true}; }

std::expected<Receipt, PipelineError> run_pipeline(const fs::path& folder) {
    // Real Pass18 worker, with separate file, aggregate-byte, and request caps.
    // Each result owns its bytes; no iterator or span survives the worker.
    ingestion::Root root(folder, 4096);
    ingestion::WorkerBatch worker(std::move(root), {
        {"models/tri.msh1", ingestion::AssetKind::triangle_mesh},
        {"assets/tex.txr4", ingestion::AssetKind::texture_rgba8}
    }, 8192, 2);
    auto outcomes = worker.finish();

    if (outcomes.size() != 2) return std::unexpected(PipelineError::read_failure);
    std::optional<ingestion::OwnedAsset> texture;
    std::optional<ingestion::OwnedAsset> mesh;
    for (auto& outcome : outcomes) {
        if (!outcome) return std::unexpected(PipelineError::read_failure);
        auto value = std::move(*outcome);
        if (value.relative == "assets/tex.txr4" &&
            value.kind == ingestion::AssetKind::texture_rgba8 && !texture) {
            texture.emplace(std::move(value));
        } else if (value.relative == "models/tri.msh1" &&
                   value.kind == ingestion::AssetKind::triangle_mesh && !mesh) {
            mesh.emplace(std::move(value));
        } else {
            return std::unexpected(PipelineError::read_failure);
        }
    }
    if (!texture || !mesh) return std::unexpected(PipelineError::read_failure);

    // Decode BOTH before publishing EITHER. A wrong mesh header must not
    // leave a partially visible texture in an editor's new scene.
    auto decoded_texture =
        assets::Spec<assets::TextureTag>::decode(texture->bytes);
    auto decoded_mesh =
        assets::Spec<assets::MeshTag>::decode(mesh->bytes);
    if (!decoded_texture || !decoded_mesh)
        return std::unexpected(PipelineError::decode_failure);

    // This registry is local staging state, not the shared live scene. If a
    // later import fails, it is discarded without publishing a partial scene.
    assets::GenerationalAssetRegistry registry(2);
    auto texture_handle = registry.import<assets::TextureTag>(texture->bytes);
    auto mesh_handle = registry.import<assets::MeshTag>(mesh->bytes);
    if (!texture_handle || !mesh_handle || registry.active_count() != 2)
        return std::unexpected(PipelineError::publication_failure);

    const auto tk = key(*texture_handle), mk = key(*mesh_handle);
    const auto owner = registry.owner_id();
    const work::ResourceKey luma_key{work::ResourceKind::gpu_output,owner,10,1};
    const work::ResourceKey bounds_key{work::ResourceKind::gpu_output,owner,11,1};
    const work::ResourceKey model_key{work::ResourceKind::gpu_output,owner,12,1};
    const work::ResourceKey receipt_key{work::ResourceKind::gpu_output,owner,13,1};

    work::DependencyGraph graph;
    const auto must_add = [&](work::Task task) {
        return graph.add(std::move(task)).has_value();
    };
    if (!must_add({{1},"validate staged identities",work::Domain::host,1,
                   {},{write(tk),write(mk)}}) ||
        !must_add({{2},"texture integer reduction",work::Domain::host,2,
                   {},{read(tk),write(luma_key)}}) ||
        !must_add({{3},"mesh area oracle",work::Domain::host,2,
                   {},{read(mk),write(bounds_key)}}) ||
        !must_add({{4},"memory visibility and completion oracle",work::Domain::host,3,
                   {},{read(tk),write(model_key)}}) ||
        !must_add({{5},"canonical receipt",work::Domain::host,1,
                   {},{read(luma_key),read(bounds_key),read(model_key),
                       write(receipt_key)}}) ||
        !must_add({{6},"retire old registry identities",work::Domain::host,1,
                   {{5}},{read(receipt_key),write(tk),write(mk)}}))
        return std::unexpected(PipelineError::graph_failure);

    const auto plan = graph.plan();
    if (!plan || plan->ordered.size() != 6 ||
        !plan->gpu_edges_requiring_backend_review.empty())
        return std::unexpected(PipelineError::graph_failure);

    // Independent order oracle: all declared dependencies must precede use.
    for (const auto& edge : plan->edges) {
        const auto a = std::find_if(plan->ordered.begin(),plan->ordered.end(),
            [&](const auto& step){ return step.id == edge.from; });
        const auto b = std::find_if(plan->ordered.begin(),plan->ordered.end(),
            [&](const auto& step){ return step.id == edge.to; });
        if (a == plan->ordered.end() || b == plan->ordered.end() || a >= b)
            return std::unexpected(PipelineError::graph_failure);
    }

    Receipt receipt;
    receipt.texture_source_fingerprint = texture->checksum;
    receipt.mesh_source_fingerprint = mesh->checksum;
    for (const auto& task : plan->ordered) {
        switch (task.id.value) {
            case 1: {
                if (!registry.lookup(*texture_handle) ||
                    !registry.lookup(*mesh_handle))
                    return std::unexpected(PipelineError::publication_failure);
                break;
            }
            case 2: {
                const auto borrowed = registry.lookup(*texture_handle);
                if (!borrowed) return std::unexpected(PipelineError::compute_failure);
                const auto& pixels = borrowed->get().pixels;
                if (pixels.size() % 4 != 0)
                    return std::unexpected(PipelineError::compute_failure);
                for (std::size_t i = 0; i < pixels.size(); i += 4)
                    receipt.luma += static_cast<std::uint64_t>(pixels[i]) +
                                    2ULL * pixels[i + 1] + pixels[i + 2];
                // This borrowed reference ends before any registry erase.
                break;
            }
            case 3: {
                const auto borrowed = registry.lookup(*mesh_handle);
                if (!borrowed) return std::unexpected(PipelineError::compute_failure);
                const auto& vertices = borrowed->get().vertices;
                const auto& indices = borrowed->get().indices;
                if (indices.size() < 3)
                    return std::unexpected(PipelineError::compute_failure);
                const auto& p = vertices[indices[0]];
                const auto& q = vertices[indices[1]];
                const auto& r = vertices[indices[2]];
                // Promote BEFORE subtraction/multiplication; int16 * int16
                // can overflow a 32-bit intermediate on adversarial data.
                const auto ax = std::int64_t{q.x} - p.x;
                const auto ay = std::int64_t{q.y} - p.y;
                const auto bx = std::int64_t{r.x} - p.x;
                const auto by = std::int64_t{r.y} - p.y;
                receipt.signed_twice_area_q16 = ax * by - ay * bx;
                break;
            }
            case 4: {
                const auto borrowed = registry.lookup(*texture_handle);
                if (!borrowed) return std::unexpected(PipelineError::compute_failure);
                const auto& pixels = borrowed->get().pixels;
                const work::ResourceKey virtual_image{
                    work::ResourceKind::gpu_texture, owner, 23, 1};
                memory::MemoryContractOracle oracle(64);
                if (!oracle.allocate(virtual_image,pixels.size(),8) ||
                    !oracle.write(virtual_image,work::Domain::host,0,pixels))
                    return std::unexpected(PipelineError::compute_failure);
                // A mere execution-role transition does NOT publish bytes.
                const memory::BarrierDescription change{
                    virtual_image,work::Domain::host,work::Domain::gpu_compute,
                    memory::Layout::undefined,memory::Layout::shader_read_only,
                    0,pixels.size()};
                if (!oracle.execution_only_transition(change))
                    return std::unexpected(PipelineError::compute_failure);
                const auto invisible = oracle.read(
                    virtual_image,work::Domain::gpu_compute,0,pixels.size());
                if (invisible ||
                    invisible.error() != memory::MemoryError::consumer_not_visible)
                    return std::unexpected(PipelineError::compute_failure);
                if (!oracle.memory_dependency(
                        virtual_image,work::Domain::host,
                        work::Domain::gpu_compute,0,pixels.size()))
                    return std::unexpected(PipelineError::compute_failure);
                const auto observed = oracle.read(
                    virtual_image,work::Domain::gpu_compute,0,pixels.size());
                if (!observed || *observed != pixels ||
                    !oracle.mark_in_flight(virtual_image,1))
                    return std::unexpected(PipelineError::compute_failure);
                const auto premature = oracle.retire(virtual_image);
                if (premature ||
                    premature.error() != memory::MemoryError::completion_pending)
                    return std::unexpected(PipelineError::retirement_failure);
                if (!oracle.observe_completion(1) ||
                    !oracle.retire(virtual_image))
                    return std::unexpected(PipelineError::retirement_failure);
                const auto retired_read = oracle.read(
                    virtual_image,work::Domain::gpu_compute,0,pixels.size());
                if (retired_read ||
                    retired_read.error() != memory::MemoryError::already_retired)
                    return std::unexpected(PipelineError::retirement_failure);
                break;
            }
            case 5: {
                receipt.canonical =
                    "AllMight-Book01-CPU-Trace-v1\n"
                    "texture_source_fnv1a=" +
                    std::to_string(receipt.texture_source_fingerprint) + "\n" +
                    "mesh_source_fnv1a=" +
                    std::to_string(receipt.mesh_source_fingerprint) + "\n" +
                    "texture_weighted_rgb_sum=" +
                    std::to_string(receipt.luma) + "\n" +
                    "mesh_signed_double_area_q16=" +
                    std::to_string(receipt.signed_twice_area_q16) + "\n";
                break;
            }
            case 6: {
                if (receipt.canonical.empty() ||
                    !registry.erase(*texture_handle) ||
                    !registry.erase(*mesh_handle) ||
                    registry.active_count() != 0)
                    return std::unexpected(PipelineError::retirement_failure);
                const auto stale_texture = registry.lookup(*texture_handle);
                const auto stale_mesh = registry.lookup(*mesh_handle);
                if (stale_texture || stale_mesh ||
                    stale_texture.error() != assets::LookupError::stale_or_dead ||
                    stale_mesh.error() != assets::LookupError::stale_or_dead)
                    return std::unexpected(PipelineError::retirement_failure);
                break;
            }
            default: return std::unexpected(PipelineError::graph_failure);
        }
    }
    if (receipt.canonical.empty())
        return std::unexpected(PipelineError::graph_failure);
    return receipt;
}

// Optimistic single-threaded scene publication. A long-running future worker
// must acquire the live scene's mutex/actor before checking+committing revision;
// comparing a revision without a lock is NOT a thread-safe publication protocol.
struct Scene {
    std::uint64_t revision{7};
    std::optional<Receipt> visible;
    bool publish(std::uint64_t expected, Receipt result) {
        if (revision != expected) return false;
        if (revision == std::numeric_limits<std::uint64_t>::max()) return false;
        visible.emplace(std::move(result));
        ++revision;
        return true;
    }
};

void end_to_end_happy_path_and_scene_revision() {
    Fixture fixture;
    const auto tx = valid_texture(), ms = valid_mesh();
    fixture.file("assets/tex.txr4",tx);
    fixture.file("models/tri.msh1",ms);
    const auto first = run_pipeline(fixture.root);
    const auto second = run_pipeline(fixture.root);
    require(first && second, "two independent end-to-end traces must succeed");
    require(first->canonical == second->canonical,
            "deterministic receipt must not embed temp path or minted owner");
    require(first->luma == 297 &&
            first->signed_twice_area_q16 == 65536,
            "independent exact integer workload answers");
    require(first->texture_source_fingerprint == 8952040735453606912ULL &&
            first->mesh_source_fingerprint == 9136700281888616565ULL,
            "independent fixed FNV-1a fixture answers, not the same function again");
    require(first->texture_source_fingerprint == ingestion::fnv1a(tx) &&
            first->mesh_source_fingerprint == ingestion::fnv1a(ms),
            "receipt fingerprints reflect the actual on-disk source bytes");
    Scene scene;
    require(!scene.publish(6,*first) && !scene.visible &&
            scene.revision == 7,"outdated scene revision refuses publication");
    require(scene.publish(7,*first) && scene.visible &&
            scene.visible->canonical == first->canonical && scene.revision == 8,
            "current revision atomically publishes in this single-threaded test");
    require(!scene.publish(7,*second) &&
            scene.visible->canonical == first->canonical,
            "completed old work cannot overwrite a newer scene revision");
}

void end_to_end_failure_is_not_success() {
    Fixture fixture;
    const auto tx = valid_texture(), ms = valid_mesh();
    fixture.file("assets/tex.txr4",tx);
    const auto missing = run_pipeline(fixture.root);
    require(!missing && missing.error() == PipelineError::read_failure,
            "missing mesh rejects entire trace");
    auto corrupt = ms;
    corrupt[0] = 'X';
    fixture.file("models/tri.msh1",corrupt);
    const auto bad_mesh = run_pipeline(fixture.root);
    require(!bad_mesh && bad_mesh.error() == PipelineError::decode_failure,
            "one malformed asset cannot publish half a scene");
    fixture.file("models/tri.msh1",ms);
    auto broken_texture = tx;
    broken_texture.pop_back();
    fixture.file("assets/tex.txr4",broken_texture);
    const auto bad_texture = run_pipeline(fixture.root);
    require(!bad_texture &&
            bad_texture.error() == PipelineError::decode_failure,
            "invalid RGBA8 payload cannot appear as valid frame output");
}
// Metamorphic tests distinguish source identity from derived computation.
// If only alpha changes, a source byte hash changes but the RGB sum does not.
// Reversing triangle winding changes signed area, not unsigned shape extent.
void source_mutations_have_predictable_effects() {
    Fixture fixture;
    auto tx = valid_texture();
    auto ms = valid_mesh();
    fixture.file("assets/tex.txr4", tx);
    fixture.file("models/tri.msh1", ms);
    const auto baseline = run_pipeline(fixture.root);
    require(baseline.has_value(), "baseline metamorphic fixture must decode");
    tx[15] = 0; // first alpha channel only
    fixture.file("assets/tex.txr4", tx);
    const auto alpha = run_pipeline(fixture.root);
    require(alpha && alpha->luma == baseline->luma &&
            alpha->texture_source_fingerprint !=
                baseline->texture_source_fingerprint,
            "alpha-only source mutation changes provenance, not RGB reduction");
    tx = valid_texture();
    fixture.file("assets/tex.txr4", tx);
    std::swap(ms[32], ms[34]); // triangle index 1 and index 2 low bytes
    fixture.file("models/tri.msh1", ms);
    const auto reversed = run_pipeline(fixture.root);
    require(reversed &&
            reversed->signed_twice_area_q16 == -65536 &&
            reversed->mesh_source_fingerprint != baseline->mesh_source_fingerprint,
            "winding reversal changes sign and mesh source fingerprint");
}
} // namespace

int main() {
    try {
        end_to_end_happy_path_and_scene_revision();
        end_to_end_failure_is_not_success();
        source_mutations_have_predictable_effects();
        std::cout << "Pass19: REAL file->typed registry->host DAG->CPU reduction/"
                     "memory oracle->checked retirement->stable receipt: OK\n";
        std::cout << "Not executed: native Metal/Vulkan, real GPU synchronization,"
                     " concurrently contested scene publication, full 18-target CTest.\n";
    } catch (const std::exception& error) {
        std::cerr << "Pass19 FAILED: " << error.what() << '\n';
        return 1;
    }
}
