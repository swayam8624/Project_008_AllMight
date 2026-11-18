// AllMight M013 / Parts 201-216: generic, constrained asset preparation.
// C++20. This is a CPU-side laboratory; it does not use Vulkan or Metal.
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <concepts>

namespace allmight {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message); // also works with NDEBUG
}

// A mesh stores indices. Two constructors record whether the incoming vector
// was borrowed and copied or was an rvalue whose storage could be transferred.
struct Mesh {
    std::vector<std::uint32_t> indices;
    bool copied_input;
    explicit Mesh(const std::vector<std::uint32_t>& input)
        : indices(input), copied_input(true) {}
    explicit Mesh(std::vector<std::uint32_t>&& input)
        : indices(std::move(input)), copied_input(false) {}
};

// The non-type template parameter is part of the actual texture type.
// Channel count is validated at compile time, while texel_count is checked
// at runtime because it comes from decoded input.
template <std::size_t Channels>
struct Texture {
    static_assert(Channels >= 1 && Channels <= 4, "supported channel counts: 1..4");
    std::array<std::uint8_t, Channels> first_texel{};
    std::size_t texel_count{};
};

// A declared, undefined primary trait does NOT claim every C++ type is an
// asset. An explicit specialization enables Mesh; a partial specialization
// enables every supported Texture<Channels> family member.
template <class T>
struct AssetTraits;

template <>
struct AssetTraits<Mesh> {
    static constexpr std::string_view label = "mesh";
    static std::size_t byte_size(const Mesh& mesh) {
        constexpr std::size_t element_size = sizeof(std::uint32_t);
        if (mesh.indices.size() > std::numeric_limits<std::size_t>::max() / element_size)
            throw std::overflow_error("mesh size overflow");
        return mesh.indices.size() * element_size;
    }
};

template <std::size_t Channels>
struct AssetTraits<Texture<Channels>> {
    static constexpr std::string_view label = "texture";
    static std::size_t byte_size(const Texture<Channels>& texture) {
        if (texture.texel_count > std::numeric_limits<std::size_t>::max() / Channels)
            throw std::overflow_error("texture size overflow");
        return texture.texel_count * Channels;
    }
};

// A requires-expression tests well-formedness and result types. It does not
// run byte_size(), so data-dependent overflow still needs runtime checking.
template <class T>
concept Uploadable = requires(const T& asset) {
    { AssetTraits<T>::label } -> std::convertible_to<std::string_view>;
    { AssetTraits<T>::byte_size(asset) } -> std::same_as<std::size_t>;
};

struct UnknownAsset {};
static_assert(Uploadable<Mesh>);
static_assert(Uploadable<Texture<3>>);
static_assert(Uploadable<Texture<4>>);
static_assert(!Uploadable<UnknownAsset>);

// This is the earlier SFINAE idiom: when substitution of an expression fails,
// specialization is removed instead of producing a hard error in this context.
template <class T, class = void>
struct HasByteSize : std::false_type {};

template <class T>
struct HasByteSize<T, std::void_t<decltype(AssetTraits<T>::byte_size(
                                       std::declval<const T&>()))>> : std::true_type {};

static_assert(HasByteSize<Mesh>::value);
static_assert(HasByteSize<Texture<4>>::value);
static_assert(!HasByteSize<UnknownAsset>::value);

// Two viable function templates have otherwise equivalent parameter patterns.
// For Uploadable types, the constrained overload is the more specialized one.
template <class T>
constexpr std::string_view route(const T&) { return "unsupported"; }

template <Uploadable T>
constexpr std::string_view route(const T&) { return AssetTraits<T>::label; }

// A function template deduces T from the argument. No caller must write <Mesh>.
template <Uploadable T>
std::size_t asset_bytes(const T& asset) {
    return AssetTraits<T>::byte_size(asset);
}

// A comma fold evaluates each append from left to right. The empty pack is
// accepted and returns zero. All additions are checked for overflow.
template <Uploadable... T>
std::size_t total_bytes(const T&... assets) {
    std::size_t result = 0;
    [[maybe_unused]] const auto add_checked = [&result](std::size_t next) {
        if (next > std::numeric_limits<std::size_t>::max() - result)
            throw std::overflow_error("batch size overflow");
        result += next;
    };
    (add_checked(asset_bytes(assets)), ...);
    return result;
}

struct AssetId { std::size_t slot; };

// Class template: T specifies the stored semantic type; Capacity is a
// non-type template argument. A fixed array makes stable slots explicit.
// This is not a network format or a GPU resource descriptor.
template <Uploadable T, std::size_t Capacity>
class AssetDepot {
    static_assert(Capacity > 0, "at least one asset slot is required");
    std::array<std::optional<T>, Capacity> slots_{};
    std::size_t count_ = 0;
public:
    std::size_t size() const noexcept { return count_; }
    static constexpr std::size_t capacity() noexcept { return Capacity; }

    // Args&& are forwarding references because Args are deduced here.
    // std::forward<Args> preserves the original argument categories.
    // Construct into the next *unpublished* optional; increment count only
    // after construction succeeds. Existing assets stay untouched on throw.
    template <class... Args>
        requires std::constructible_from<T, Args&&...>
    AssetId emplace(Args&&... args) {
        if (count_ == Capacity) throw std::length_error("asset depot full");
        slots_[count_].emplace(std::forward<Args>(args)...);
        return AssetId{count_++};
    }

    const T& get(AssetId id) const {
        if (id.slot >= count_) throw std::out_of_range("invalid asset slot");
        return *slots_[id.slot]; // only slots below count_ are engaged
    }
};

// The language's reference-collapsing rules make T deduce as U& for an
// lvalue, and U for an rvalue; T&& then collapses respectively to U& or U&&.
template <class T>
constexpr bool was_lvalue(T&&) { return std::is_lvalue_reference_v<T>; }

// A failed constrained call can be queried instead of being compiled as an
// intentionally broken expression. This also verifies the actual API surface.
template <class Store, class... Args>
concept CanEmplace = requires(Store& store, Args&&... args) {
    { store.emplace(std::forward<Args>(args)...) } -> std::same_as<AssetId>;
};

using MeshDepot = AssetDepot<Mesh, 2>;
static_assert(CanEmplace<MeshDepot, std::vector<std::uint32_t>&>);
static_assert(CanEmplace<MeshDepot, std::vector<std::uint32_t>>);
static_assert(!CanEmplace<MeshDepot, int>);

// A throwing asset makes the unpublished-slot invariant observable.
struct CheckedShader {
    int version;
    explicit CheckedShader(int v) : version(v) {
        if (v <= 0) throw std::invalid_argument("shader version must be positive");
    }
};

template <>
struct AssetTraits<CheckedShader> {
    static constexpr std::string_view label = "shader";
    static std::size_t byte_size(const CheckedShader&) { return sizeof(int); }
};

void run() {
    MeshDepot meshes;
    std::vector<std::uint32_t> input{3, 7, 9};
    require(was_lvalue(input), "lvalue category should be preserved");
    require(!was_lvalue(std::vector<std::uint32_t>{}), "temporary is an rvalue");

    // An lvalue chooses the const& constructor, making a semantic copy.
    const AssetId copied = meshes.emplace(input);
    require(meshes.get(copied).copied_input, "lvalue copied, not moved");
    require(input.size() == 3, "copy leaves the original usable");

    // An rvalue chooses the && constructor, making the source transferable.
    const AssetId moved = meshes.emplace(std::move(input));
    require(!meshes.get(moved).copied_input, "rvalue selects move path");
    require(meshes.get(moved).indices.size() == 3, "moved payload is retained");
    // Do not assume a moved-from vector has a particular size.

    bool full_rejected = false;
    try { meshes.emplace(std::vector<std::uint32_t>{11}); }
    catch (const std::length_error&) { full_rejected = true; }
    require(full_rejected && meshes.size() == 2, "full store preserves prior assets");

    AssetDepot<CheckedShader, 2> shaders;
    const AssetId good = shaders.emplace(3);
    bool bad_rejected = false;
    try { shaders.emplace(-1); }
    catch (const std::invalid_argument&) { bad_rejected = true; }
    require(bad_rejected && shaders.size() == 1, "throwing construction stays unpublished");
    require(shaders.get(good).version == 3, "prior shader is intact");
    const AssetId second = shaders.emplace(4);
    require(second.slot == 1 && shaders.size() == 2, "failed slot can be reused");

    Texture<4> rgba{{255, 0, 128, 255}, 12};
    Texture<3> rgb{{12, 23, 34}, 5};
    require(asset_bytes(meshes.get(copied)) == 3 * sizeof(std::uint32_t),
            "mesh trait counts stored indices");
    require(asset_bytes(rgba) == 48 && asset_bytes(rgb) == 15,
            "texture partial specialization accounts for channels");
    require(total_bytes(meshes.get(copied), rgba, rgb) == 75,
            "heterogeneous checked comma fold sums bytes");
    require(total_bytes() == 0, "empty fold produces zero");
    require(route(rgba) == "texture", "constrained overload wins for Texture");
    require(route(meshes.get(copied)) == "mesh", "constrained overload wins for Mesh");
    require(route(UnknownAsset{}) == "unsupported", "fallback is deliberate");

    bool overflow_rejected = false;
    Texture<4> enormous{{0, 0, 0, 0}, std::numeric_limits<std::size_t>::max()};
    try { (void)asset_bytes(enormous); }
    catch (const std::overflow_error&) { overflow_rejected = true; }
    require(overflow_rejected, "runtime byte-size overflow is checked");

    std::cout << "generic_asset_pipeline_lab: all checks passed\n";
}
} // namespace allmight

int main() {
    try { allmight::run(); }
    catch (const std::exception& error) {
        std::cerr << "generic_asset_pipeline_lab failed: " << error.what() << '\n';
        return 1;
    }
}
