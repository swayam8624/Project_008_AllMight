// Project AllMight / Book 1 / Pass 05 (C++23).
// Same NES-01/NES-02 NROM type, now imported beside typed textures and meshes.
// This validates exactly the small documented educational wire formats,
// not PNG/KTX, glTF, device uploads, true NES execution, or GPU resource safety.
#include "nes_course/typed_asset_pipeline.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using namespace allmight::nes;
using namespace allmight::nes::assets;

void require(bool condition, std::string_view why) {
    if (!condition) throw std::runtime_error(std::string(why));
}
void write_u16le(std::vector<Byte>& file, std::size_t pos, std::uint16_t x) {
    require(pos <= file.size() && file.size() - pos >= 2,
            "fixture must have two writable bytes");
    file[pos] = static_cast<Byte>(x & 0xFFU);
    file[pos+1] = static_cast<Byte>((x >> 8U) & 0xFFU);
}

std::vector<Byte> nrom(Byte prg_banks, Byte chr_banks, Byte flags6 = 0) {
    auto file = std::vector<Byte>(
        header_bytes + std::size_t{prg_banks} * prg_bank_bytes
                     + std::size_t{chr_banks} * chr_bank_bytes, 0);
    file[0]='N'; file[1]='E'; file[2]='S'; file[3]=0x1A;
    file[4]=prg_banks; file[5]=chr_banks; file[6]=flags6;
    return file;
}
std::vector<Byte> rgba(std::uint16_t w, std::uint16_t h,
                       bool with_payload = true) {
    std::vector<Byte> file(12, 0);
    file[0]='T'; file[1]='X'; file[2]='R'; file[3]='4';
    write_u16le(file,4,w);
    write_u16le(file,6,h);
    if (with_payload) {
        // Test helper deliberately limits allocations; large dimensions
        // can be tested with a header-only malformed fixture.
        require(w <= 512 && h <= 512, "fixture payload exceeds test limit");
        file.resize(12 + std::size_t{w} * h * 4, 0);
        for (std::size_t i = 12; i < file.size(); ++i)
            file[i] = static_cast<Byte>((i - 12) * 17U + 9U);
    }
    return file;
}
std::vector<Byte> triangle() {
    // Three fixed-point vertices:
    // (0,0,0), (256,0,0), (0,-256,0); one triangle (0,1,2).
    // Signed values travel as two's-complement 16-bit words.
    std::vector<Byte> file(12 + 3*6 + 3*2, 0);
    file[0]='M'; file[1]='S'; file[2]='H'; file[3]='1';
    write_u16le(file, 4, 3);
    write_u16le(file, 6, 3);
    write_u16le(file, 12 + 6, 256);
    write_u16le(file, 12 + 12 + 2,
                static_cast<std::uint16_t>(std::int16_t{-256}));
    const std::size_t at = 12 + 3*6;
    write_u16le(file, at+0, 0);
    write_u16le(file, at+2, 1);
    write_u16le(file, at+4, 2);
    return file;
}
template<Supported Tag>
void reject_without_change(AssetStore& store,
                           std::span<const Byte> blob,
                           DecodeError expected) {
    const auto before = store.count();
    const auto attempt = store.import<Tag>(blob);
    require(!attempt, "malformed source must not publish");
    require(attempt.error().kind == PublishError::rejected_source,
            "source rejection should remain a source error");
    require(attempt.error().source == expected, "wrong typed rejection");
    require(store.count() == before, "rejected source changed record count");
}

void check_texture_contract() {
    AssetStore store(64);
    auto file = rgba(2,3);
    auto id = store.import<TextureTag>(file);
    require(id.has_value() && id->slot == 0, "first texture must publish");
    const auto* tex = store.find(*id);
    require(tex && tex->width == 2 && tex->height == 3, "texture dimensions");
    require(tex->pixels.size() == 24 && tex->pixels[0] == 9 &&
            tex->pixels[23] == static_cast<Byte>(23U*17U+9U),
            "RGBA8 payload must be independent and exact");
    file[12] = 0;
    file.clear();
    file.shrink_to_fit();
    require(store.find(*id)->pixels[0] == 9,
            "store must own pixels, not borrow file buffer");

    auto zero = rgba(0,1);
    reject_without_change<TextureTag>(store,zero,DecodeError::zero_dimension);
    auto huge = rgba(600,600,false);
    reject_without_change<TextureTag>(store,huge,DecodeError::too_large);
    auto wrong = rgba(1,1); wrong[0]='P';
    reject_without_change<TextureTag>(store,wrong,DecodeError::wrong_magic);
    auto extra_flag = rgba(1,1); extra_flag[11]=1;
    reject_without_change<TextureTag>(store,extra_flag,
                                      DecodeError::unsupported_version_or_flags);
    auto missing_pixel = rgba(1,1); missing_pixel.pop_back();
    reject_without_change<TextureTag>(store,missing_pixel,DecodeError::bad_length);
    auto trailing = rgba(1,1); trailing.push_back(1);
    reject_without_change<TextureTag>(store,trailing,DecodeError::bad_length);
    const std::vector<Byte> short_file{'T','X','R','4'};
    reject_without_change<TextureTag>(store,short_file,DecodeError::short_header);

    auto max = rgba(512,512); // 512*512*4 = exactly 1 MiB (accepted)
    const auto edge = store.import<TextureTag>(max);
    require(edge.has_value() && store.find(*edge)->pixels.size() == 1024*1024,
            "texture budget boundary must be inclusive");
}
void check_mesh_contract() {
    AssetStore store(20);
    auto file = triangle();
    const auto id = store.import<MeshTag>(file);
    require(id.has_value() && id->slot == 0, "triangle must publish");
    const auto* mesh = store.find(*id);
    require(mesh && mesh->vertices.size()==3 && mesh->indices.size()==3,
            "triangle counts must be owned");
    require(mesh->vertices[1].x == 256 && mesh->vertices[2].y == -256,
            "signed fixed-point LE coordinates must round-trip");
    require(mesh->indices[0]==0 && mesh->indices[1]==1 && mesh->indices[2]==2,
            "triangle index order");
    file.clear(); file.shrink_to_fit();
    require(store.find(*id)->vertices[2].y == -256,
            "owned mesh must outlive source buffer");

    auto bad_index = triangle(); write_u16le(bad_index,12+18+4,3);
    reject_without_change<MeshTag>(store,bad_index,DecodeError::invalid_index);
    auto bad_count = triangle(); write_u16le(bad_count,6,4);
    reject_without_change<MeshTag>(store,bad_count,DecodeError::invalid_triangle_count);
    auto bad_magic = triangle(); bad_magic[0]='X';
    reject_without_change<MeshTag>(store,bad_magic,DecodeError::wrong_magic);
    auto flags = triangle(); flags[8]=1;
    reject_without_change<MeshTag>(store,flags,DecodeError::unsupported_version_or_flags);
    auto truncated = triangle(); truncated.pop_back();
    reject_without_change<MeshTag>(store,truncated,DecodeError::bad_length);
    auto extra = triangle(); extra.push_back(0);
    reject_without_change<MeshTag>(store,extra,DecodeError::bad_length);
    auto overbudget = triangle(); write_u16le(overbudget,4,8193);
    reject_without_change<MeshTag>(store,overbudget,DecodeError::too_large);
    auto empty_triangles = triangle(); write_u16le(empty_triangles,6,0);
    reject_without_change<MeshTag>(store,empty_triangles,
                                    DecodeError::invalid_triangle_count);
}
void check_mixed_pipeline_and_publication() {
    AssetStore store(256);
    auto rom = nrom(1,0);
    rom[16] = 0xB7;
    const auto cartridge = store.import<CartridgeTag>(rom);
    require(cartridge.has_value() && cartridge->slot==0, "NROM typed import");
    rom[16] = 0;
    rom.clear(); rom.shrink_to_fit();
    require(store.find(*cartridge)->read_prg(0x8000) == 0xB7,
            "original NROM core must own its PRG bytes");

    auto tile = rgba(1,1);
    const auto texture = store.import<TextureTag>(tile);
    require(texture.has_value() && texture->slot==1, "texture slot after ROM");
    auto tri = triangle();
    const auto mesh = store.import<MeshTag>(tri);
    require(mesh.has_value() && mesh->slot==2, "mesh slot after texture");
    require(store.count()==3, "three different types share one store");
    require(store.find(AssetId<TextureTag>{cartridge->slot}) == nullptr,
            "forged wrong-kind slot must be rejected");
    require(store.find(AssetId<CartridgeTag>{texture->slot}) == nullptr,
            "wrong-kind access must not reinterpret variant bits");

    auto unsupported = nrom(1,0,0x10); // NROM-only importer rejects mapper 1
    reject_without_change<CartridgeTag>(store,unsupported,
                                        DecodeError::unsupported_mapper);
    auto invalid = nrom(1,0); invalid[0]='?';
    reject_without_change<CartridgeTag>(store,invalid,
                                        DecodeError::invalid_cartridge);
    auto wrong_geometry = nrom(3,0);
    reject_without_change<CartridgeTag>(store,wrong_geometry,
                                        DecodeError::unsupported_nrom_geometry);

    // Real parse succeeds, but publishing is interrupted before commit.
    for (const auto fault : {TestFault::before_stage,TestFault::after_stage,
                             TestFault::throw_bad_alloc_after_stage}) {
        const auto count = store.count();
        const auto tried = store.import<TextureTag>(tile, fault);
        require(!tried && store.count()==count, "injected failure published");
        const auto error = fault == TestFault::throw_bad_alloc_after_stage
            ? PublishError::allocation_failure : PublishError::injected_failure;
        require(tried.error().kind == error, "wrong operational error channel");
    }

    // An append-only ID survives reallocation, even when a borrowed POINTER
    // into vector storage does not. Never keep that pointer across imports.
    for (int i=3; i<256; ++i) {
        auto added = store.import<TextureTag>(tile);
        require(added.has_value() && added->slot==static_cast<std::size_t>(i),
                "append sequence must preserve contiguous typed IDs");
    }
    require(store.count()==256, "capacity boundary");
    require(store.find(*cartridge)->read_prg(0x8000) == 0xB7,
            "cartridge relookup must work across variant relocations");
    require(store.find(*texture)->pixels[0] == 9,
            "texture relookup across relocations");
    require(store.find(*mesh)->indices[2]==2,
            "mesh relookup across relocations");

    const auto over = store.import<TextureTag>(tile);
    require(!over && over.error().kind==PublishError::capacity_exhausted,
            "capacity exhaustion should be explicit");
    require(store.count()==256, "capacity failure must not change count");
}
void check_checked_math_and_batch_scope() {
    std::size_t value = 0;
    const auto max = std::numeric_limits<std::size_t>::max();
    require(!multiply(max, 2, value),
            "multiplication must refuse overflow before evaluating the product");
    require(multiply(0, max, value) && value == 0,
            "zero-by-large multiplication must be representable");
    require(add(12, 24, value) && value == 36,
            "checked length addition must give the exact result");
    require(!add(max, 1, value),
            "length addition must refuse overflow");

    AssetStore store(2);
    const auto first = store.import<TextureTag>(rgba(1, 1));
    require(first.has_value(), "first batch-like call must succeed");
    auto second = triangle();
    second[0] = '?';
    const auto rejected = store.import<MeshTag>(second);
    require(!rejected && rejected.error().kind == PublishError::rejected_source,
            "second batch-like call must reject bad mesh");
    // These are TWO independent operations, not an all-or-nothing batch.
    require(store.count() == 1 && store.find(*first) != nullptr,
            "an earlier successful import remains committed on later failure");
}

void check_contracts_at_compile_time() {
    static_assert(Supported<CartridgeTag> && Supported<TextureTag> &&
                  Supported<MeshTag>);
    static_assert(!Supported<UnsupportedAsset>);
    static_assert(!std::is_convertible_v<AssetId<CartridgeTag>, AssetId<TextureTag>>);
    static_assert(!std::is_copy_constructible_v<NromCartridge>);
    static_assert(std::is_nothrow_move_constructible_v<AssetRecord>);
}
} // namespace
int main() {
    try {
        check_contracts_at_compile_time();
        check_checked_math_and_batch_scope();
        check_texture_contract();
        check_mesh_contract();
        check_mixed_pipeline_and_publication();
        std::cout<<"NES-05A typed C++23 asset pipeline: texture, mesh, NROM, failures and 256-slot relocation passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr<<"NES-05A FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
