// Project AllMight / Book 01 / Pass 06 — C++23 generational identity.
//
// Uses the *same* real Spec<Tag> decoders and NromCartridge from the previous
// five passes. All inputs are synthetic and no invalid pointer is dereferenced.
// This is a single-threaded logical-identity model; a typed handle is not a
// synchronized borrow, network ID, GPU lifetime fence, or cryptographic token.
#include "nes_course/generational_asset_registry.hpp"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using namespace allmight::nes;
using namespace allmight::nes::assets;

void check(bool ok, std::string_view reason) {
    if (!ok) throw std::runtime_error(std::string(reason));
}
void word(std::vector<Byte>& input, std::size_t offset, std::uint16_t value) {
    check(offset <= input.size() && input.size() - offset >= 2,
          "fixture must contain the requested word");
    input[offset] = static_cast<Byte>(value & 255U);
    input[offset + 1] = static_cast<Byte>((value >> 8) & 255U);
}
std::vector<Byte> texture(Byte shade) {
    // "TXR4", width 2, height 1, reserved zeros, two RGBA pixels.
    std::vector<Byte> file(12 + 8, 0);
    file[0]='T'; file[1]='X'; file[2]='R'; file[3]='4';
    word(file,4,2); word(file,6,1);
    for (std::size_t i=12; i<file.size(); ++i) file[i] = shade;
    return file;
}
std::vector<Byte> mesh() {
    // "MSH1", 3 vertices x 6 bytes, 3 indices x 2 bytes, triangle (0,1,2).
    std::vector<Byte> file(12 + 18 + 6, 0);
    file[0]='M'; file[1]='S'; file[2]='H'; file[3]='1';
    word(file,4,3); word(file,6,3);
    word(file,12 + 18 + 2,1);
    word(file,12 + 18 + 4,2);
    return file;
}
std::vector<Byte> cartridge(Byte program_start) {
    // Legacy iNES: one 16KiB PRG bank, no CHR-ROM (8KiB owned CHR RAM).
    std::vector<Byte> file(header_bytes + prg_bank_bytes, 0);
    file[0]='N'; file[1]='E'; file[2]='S'; file[3]=0x1A;
    file[4]=1; file[5]=0;
    file[header_bytes] = program_start;
    return file;
}

template<Supported Tag>
AssetHandle<Tag> must_import(GenerationalAssetRegistry& registry,
                             std::span<const Byte> input) {
    auto created = registry.import<Tag>(input);
    check(created.has_value(), "valid import unexpectedly rejected");
    return *created;
}

void identity_is_not_an_address() {
    GenerationalAssetRegistry registry(1024);
    const auto old = must_import<TextureTag>(registry, texture(19));
    check(old.owner == registry.owner_id() && old.generation == 1 && old.slot == 0,
          "first typed handle must include owner, slot and generation");
    check(registry.active_count() == 1 && registry.allocated_slots() == 1,
          "one imported asset must occupy one live slot");
    check(registry.lookup(old).has_value(), "just-minted handle must resolve");

    // Growth can reallocate storage. NEVER read a previously obtained
    // pointer or reference after further imports. Re-lookup by handle.
    for (int i=0; i<700; ++i) {
        const auto fresh = must_import<TextureTag>(
            registry, texture(static_cast<Byte>(i)));
        check(fresh.slot == static_cast<std::size_t>(i+1) && fresh.generation == 1,
              "append must allocate consecutive new slots before any erase");
    }
    const auto again = registry.lookup(old);
    check(again && again->get().pixels[0] == 19,
          "logical handle must survive relocation of vector storage");
    check(registry.active_count() == 701 && registry.recycled_slots() == 0,
          "growth must not forge free slots");
}

void reuse_invalidates_old_handle_and_preserves_type() {
    GenerationalAssetRegistry registry(3);
    const auto tex = must_import<TextureTag>(registry, texture(11));
    const auto m = must_import<MeshTag>(registry, mesh());
    const auto rom = must_import<CartridgeTag>(registry, cartridge(0xB3));
    check(registry.active_count() == 3, "registry should start full");

    const auto no_capacity = registry.import<TextureTag>(texture(9));
    check(!no_capacity &&
          no_capacity.error().kind == RegistryImportError::capacity_exhausted,
          "full registry must refuse additional allocation");

    // Even though the payload's C++ type is known, an attacker could forge
    // a wrong-kind handle. The runtime variant check is still necessary.
    const AssetHandle<TextureTag> forged{
        m.owner, m.slot, m.generation};
    const auto wrong_kind = registry.lookup(forged);
    check(!wrong_kind && wrong_kind.error() == LookupError::wrong_kind,
          "forged typed handle must not reinterpret a mesh as a texture");
    const auto wrong_erase = registry.erase(forged);
    check(!wrong_erase && wrong_erase.error() == EraseError::wrong_kind,
          "forged wrong-kind deletion must preserve the mesh");

    const auto released = registry.erase(tex);
    check(released.has_value() && registry.active_count() == 2 &&
          registry.recycled_slots() == 1,
          "erase must free only the chosen live slot");
    const auto deleted_lookup = registry.lookup(tex);
    check(!deleted_lookup && deleted_lookup.error() == LookupError::stale_or_dead,
          "deleted handle must stop resolving before slot is reused");
    const auto repeated = registry.erase(tex);
    check(!repeated && repeated.error() == EraseError::stale_or_dead,
          "repeated erase must not enqueue a slot twice");

    // Invalid imports must NOT consume the free slot or advance generation.
    auto invalid = texture(44); invalid[0] = '?';
    const auto rejected = registry.import<TextureTag>(invalid);
    check(!rejected && rejected.error().kind == RegistryImportError::rejected_source,
          "bad source must retain its diagnostic");
    check(registry.recycled_slots() == 1 && registry.active_count() == 2,
          "source rejection must not disturb the reuse queue");

    for (const auto fault : {TestFault::before_stage, TestFault::after_stage,
                             TestFault::throw_bad_alloc_after_stage}) {
        const auto failed = registry.import<TextureTag>(texture(47), fault);
        check(!failed && registry.recycled_slots() == 1,
              "failure injection must preserve the free slot and old assets");
    }
    const auto replacement = must_import<TextureTag>(registry, texture(89));
    check(replacement.slot == tex.slot &&
          replacement.generation == tex.generation + 1 &&
          replacement.owner == tex.owner,
          "reuse must preserve slot location but change logical identity");
    const auto fresh_value = registry.lookup(replacement);
    check(fresh_value && fresh_value->get().pixels[0] == 89,
          "reused slot must contain the newly decoded texture");
    check(!registry.lookup(tex), "old generation must not resolve the new asset");

    check(registry.lookup(m) && registry.lookup(rom),
          "unrelated handles must survive another slot's reuse");
    check(registry.lookup(rom)->get().read_prg(0x8000) == 0xB3,
          "real NROM cartridge content must remain intact");
    check(registry.active_count() == 3 && registry.recycled_slots() == 0,
          "reuse should consume exactly one free slot");
}

void owner_namespace_and_move() {
    GenerationalAssetRegistry first(10);
    GenerationalAssetRegistry other(10);
    const auto a = must_import<TextureTag>(first, texture(8));
    const auto b = must_import<TextureTag>(other, texture(9));
    check(a.slot == b.slot && a.generation == b.generation &&
          a.owner != b.owner, "different stores must have disjoint owners");
    const auto foreign = other.lookup(a);
    check(!foreign && foreign.error() == LookupError::wrong_owner,
          "a syntactically valid handle must not cross registry boundaries");
    const auto rejected = other.erase(a);
    check(!rejected && rejected.error() == EraseError::wrong_owner,
          "foreign erasure must not mutate owner B");

    // Move construction transfers the logical owner namespace.
    GenerationalAssetRegistry destination(std::move(first));
    check(destination.owner_id() == a.owner &&
          destination.lookup(a).has_value(),
          "handles must follow a moved registry's logical identity");
    check(!first.lookup(a) &&
          first.lookup(a).error() == LookupError::wrong_owner,
          "moved-from registry cannot masquerade as its successor");
    const auto no_import = first.import<TextureTag>(texture(1));
    check(!no_import &&
          no_import.error().kind == RegistryImportError::invalid_moved_from_owner,
          "moved-from registry cannot mint duplicate ownership");
    check(other.lookup(b) && other.lookup(b)->get().pixels[0] == 9,
          "moving A must not change B");
}

void generation_overflow_retires_instead_of_wrapping() {
    GenerationalAssetRegistry registry(
        1, std::numeric_limits<std::uint64_t>::max());
    const auto maximal = must_import<TextureTag>(registry, texture(7));
    check(maximal.generation == std::numeric_limits<std::uint64_t>::max(),
          "explicit test generation must reach boundary");
    check(registry.erase(maximal).has_value(), "last possible generation must be erasable");
    check(registry.active_count() == 0 && registry.recycled_slots() == 0 &&
          registry.allocated_slots() == 1,
          "generation-exhausted slot must be retired, not recycled");
    check(!registry.lookup(maximal), "retired slot cannot resolve stale handle");
    const auto refused = registry.import<TextureTag>(texture(5));
    check(!refused &&
          refused.error().kind == RegistryImportError::capacity_exhausted,
          "permanently retired slot must not silently wrap its generation");
}

void long_reuse_sequence_and_model_oracle() {
    GenerationalAssetRegistry registry(8);
    // Independent expected generation, slot and payload; no private registry
    // structure is inspected. Two assets are left live while one slot reuses
    // over and over. That probes the ABA hazard across 2,000 transitions.
    const auto permanent = must_import<CartridgeTag>(registry, cartridge(0x59));
    auto current = must_import<TextureTag>(registry, texture(0));
    for (std::uint64_t cycle=0; cycle<2000; ++cycle) {
        const auto visible = registry.lookup(current);
        check(visible && visible->get().pixels[0] ==
              static_cast<Byte>(cycle & 255U),
              "independent payload oracle mismatch before deletion");
        const auto before = current;
        check(registry.erase(before).has_value(), "model erase failed");
        const auto next = must_import<TextureTag>(
            registry, texture(static_cast<Byte>((cycle + 1U) & 255U)));
        check(next.owner == before.owner && next.slot == before.slot &&
              next.generation == before.generation + 1U,
              "reuse generation must advance one per cycle");
        check(!registry.lookup(before), "old handle must never resurrect");
        current = next;
        check(registry.lookup(permanent) &&
              registry.lookup(permanent)->get().read_prg(0x8000) == 0x59,
              "repeated slot reuse must preserve unrelated NROM asset");
    }
    check(registry.active_count() == 2 && registry.allocated_slots() == 2,
          "reuse should not allocate thousands of slots");
}

void zero_capacity_and_missing_slot() {
    GenerationalAssetRegistry empty(0);
    const auto result = empty.import<MeshTag>(mesh());
    check(!result &&
          result.error().kind == RegistryImportError::capacity_exhausted,
          "zero capacity must reject even a valid mesh");
    const AssetHandle<MeshTag> missing{
        empty.owner_id(), 99, 1
    };
    const auto lookup = empty.lookup(missing);
    check(!lookup && lookup.error() == LookupError::missing_slot,
          "missing index must be distinguished from stale generation");
    check(!empty.erase(missing) &&
          empty.erase(missing).error() == EraseError::missing_slot,
          "erase must not accept an out-of-range handle");
}
} // namespace

int main() {
    try {
        identity_is_not_an_address();
        reuse_invalidates_old_handle_and_preserves_type();
        owner_namespace_and_move();
        generation_overflow_retires_instead_of_wrapping();
        long_reuse_sequence_and_model_oracle();
        zero_capacity_and_missing_slot();
        std::cout
            << "NES-06G: owner-aware typed handles, relocation, 2000 reuse cycles,"
               " stale rejection and wrap-safe retirement passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "NES-06G FAILED: " << error.what() << '\n';
        return 1;
    }
}
