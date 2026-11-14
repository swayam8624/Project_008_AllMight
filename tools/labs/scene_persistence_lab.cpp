// C++20 portable scene persistence witness; text framing and explicit field codecs.
// Build: c++ -std=c++20 -Wall -Wextra -Wpedantic -Werror tools/labs/scene_persistence_lab.cpp -o /tmp/scene_persistence && /tmp/scene_persistence
#include <charconv>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

// A file ID is stable authored identity. It is intentionally distinct from a
// process-local ECS slot or pointer, neither of which survives a reload.
using Id = std::uint64_t;
struct Entity { Id id{}; Id asset{}; Id parent{}; std::int32_t x{}, y{}; };
struct Scene { std::vector<Entity> entities; };
struct Asset { Id id{}; std::string name; };

void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

// Validate only scene-local invariants: the writer has no asset catalog, while
// asset existence is checked by the reader against the catalog it receives.
void validate_identity_and_hierarchy(const Scene& scene) {
    require(scene.entities.size() <= 10000, "too many entities");
    std::map<Id, std::size_t> by_id;
    for (std::size_t i = 0; i < scene.entities.size(); ++i) {
        const Entity& e = scene.entities[i];
        require(e.id != 0 && by_id.emplace(e.id, i).second,
                "zero or duplicate entity ID");
    }
    for (const Entity& e : scene.entities) {
        require(e.parent == 0 || (e.parent != e.id && by_id.contains(e.parent)),
                "dangling or self parent reference");
    }

    // Identity lookup proves that each edge names a record; it does not prove
    // the parent graph is acyclic. A bounded color walk checks that separately.
    // Each entity has at most one parent, so every walk follows one outgoing edge.
    std::vector<unsigned char> color(scene.entities.size(), 0); // white, gray, black
    for (std::size_t start = 0; start < scene.entities.size(); ++start) {
        if (color[start] == 2) continue;
        std::vector<std::size_t> path;
        std::size_t at = start;
        while (color[at] == 0) {
            color[at] = 1;
            path.push_back(at);
            const Id parent = scene.entities[at].parent;
            if (parent == 0) break;
            at = by_id.at(parent);
        }
        if (color[at] == 1) {
            const bool ended_at_root = scene.entities[at].parent == 0 && path.back() == at;
            require(ended_at_root, "parent cycle");
        }
        // Every visited node becomes complete before the next bounded walk.
        for (std::size_t index : path) color[index] = 2;
    }
}

// The deliberately small text codec makes field order, units, and version
// visible. Positions are signed integer scene units; IDs are unsigned decimal.
// Production formats should set explicit byte order, limits, and escaping too.
std::string save(const Scene& scene) {
    // Apply the same local identity/hierarchy rules as load before emitting bytes.
    validate_identity_and_hierarchy(scene);
    std::string out = "SCENE 2\n";
    out += "COUNT " + std::to_string(scene.entities.size()) + "\n";
    for (const Entity& e : scene.entities) {
        out += "ENTITY " + std::to_string(e.id) + " " + std::to_string(e.asset) +
               " " + std::to_string(e.parent) + " " + std::to_string(e.x) +
               " " + std::to_string(e.y) + "\n";
    }
    return out;
}

// Parse into a candidate. Nothing in the caller's live scene changes until the
// complete file, all IDs, assets, and references have passed validation.
Scene load(std::string_view bytes, const std::map<Id, Asset>& assets) {
    require(bytes.size() <= 1'000'000, "scene exceeds byte limit");
    std::size_t cursor = 0;
    auto token = [&]() -> std::string_view {
        // This format permits only ASCII space and LF separators. A tab or CR
        // stays inside a token and is rejected instead of silently normalized.
        while (cursor < bytes.size() && (bytes[cursor] == ' ' || bytes[cursor] == '\n')) ++cursor;
        require(cursor < bytes.size(), "truncated scene");
        const std::size_t start = cursor;
        while (cursor < bytes.size() && bytes[cursor] != ' ' && bytes[cursor] != '\n') ++cursor;
        return bytes.substr(start, cursor - start);
    };
    auto unsigned_number = [&]() -> std::uint64_t {
        const std::string_view field = token();
        std::uint64_t value{};
        const auto [end, error] = std::from_chars(field.data(), field.data() + field.size(), value, 10);
        // from_chars rejects leading '+'; checking both error and end rejects
        // overflow, signs, and trailing junk without locale-sensitive parsing.
        require(error == std::errc{} && end == field.data() + field.size(),
                "malformed unsigned decimal field");
        return value;
    };
    auto signed_position = [&]() -> std::int32_t {
        const std::string_view field = token();
        std::int32_t value{};
        const auto [end, error] = std::from_chars(field.data(), field.data() + field.size(), value, 10);
        require(error == std::errc{} && end == field.data() + field.size(),
                "malformed or out-of-range position");
        return value;
    };

    require(token() == "SCENE", "wrong file type");
    const auto version = unsigned_number();
    require(version == 1 || version == 2, "unsupported scene version");
    require(token() == "COUNT", "missing entity count");
    const auto count = unsigned_number();
    require(count <= 10000, "entity count exceeds limit");

    // Candidate construction is transactional: failures cannot mutate live state.
    Scene candidate;
    candidate.entities.reserve(static_cast<std::size_t>(count));
    for (std::uint64_t i = 0; i < count; ++i) {
        require(token() == "ENTITY", "missing entity record");
        Entity e;
        e.id = unsigned_number();
        e.asset = unsigned_number();
        if (version == 2) e.parent = unsigned_number(); // v1 migration defaults parent to root.
        e.x = signed_position();
        e.y = signed_position();
        require(assets.contains(e.asset), "dangling asset reference");
        candidate.entities.push_back(e);
    }
    while (cursor < bytes.size() && bytes[cursor] == '\n') ++cursor;
    require(cursor == bytes.size(), "trailing bytes after scene");
    // Shared validation builds the complete ID map before checking parent edges,
    // then runs the separate iterative cycle walk; forward references are valid.
    validate_identity_and_hierarchy(candidate);
    return candidate;
}

// Commit only after parsing/validation succeeds. std::vector::swap with the
// default allocator is noexcept, so the live scene changes in one commit step.
void load_into(Scene& live, std::string_view bytes, const std::map<Id, Asset>& assets) {
    Scene candidate = load(bytes, assets);
    live.entities.swap(candidate.entities);
}

int main() {
    const std::map<Id, Asset> assets{{42, {42, "crate.mesh"}}, {77, {77, "lamp.mesh"}}};
    Scene live{{Entity{1001, 42, 1002, -12, 35}, Entity{1002, 77, 0, 0, 80}}};
    const std::string original_bytes = save(live);
    const Scene restored = load(original_bytes, assets);
    require(save(restored) == original_bytes, "round trip changed canonical scene data");
    require(restored.entities[0].parent == 1002, "forward reference was not restored");

    // A version-1 fixture migrates by assigning the absent parent field to root.
    const Scene migrated = load("SCENE 1\nCOUNT 1\nENTITY 5 42 -3 9\n", assets);
    require(migrated.entities[0].parent == 0, "version-1 migration failed");

    const std::vector<std::string> malformed{
        "SCENE 9\nCOUNT 0\n",                                      // unsupported version
        "SCENE 2\nCOUNT 10001\n",                                 // count bound
        "SCENE 2\nCOUNT 1\nENTITY 1 42 0 2147483648 0\n",        // position overflow
        "SCENE 2\nCOUNT 2\nENTITY 1 42 0 0 0\nENTITY 1 42 0 0 0\n", // duplicate ID
        "SCENE 2\nCOUNT 1\nENTITY 1 999 0 0 0\n",                 // unknown asset
        "SCENE 2\nCOUNT 1\nENTITY 1 42 900 0 0\n",                // dangling parent
        "SCENE 2\nCOUNT 2\nENTITY 1 42 2 0 0\nENTITY 2 42 1 0 0\n", // 2-node cycle
        "SCENE 2\nCOUNT 3\nENTITY 1 42 2 0 0\nENTITY 2 42 3 0 0\nENTITY 3 42 1 0 0\n", // 3-node cycle
        "SCENE 2\nCOUNT 1\nENTITY +1 42 0 0 0\n",               // plus sign is not decimal grammar
        "SCENE 2\nCOUNT 1\nENTITY 1junk 42 0 0 0\n",           // trailing junk inside a number
        "\tSCENE 2\nCOUNT 0\n",                                   // leading tab
        "SCENE 2\nCOUNT 18446744073709551616\n",                   // uint64 overflow
        "SCENE 2\nCOUNT 1\nENTITY 1 42 0 0",                       // truncated record
        "SCENE 2\nCOUNT 0\nGARBAGE\n"                             // trailing token
    };
    for (const std::string& bad : malformed) {
        bool rejected = false;
        try { (void)load(bad, assets); } catch (const std::exception&) { rejected = true; }
        require(rejected, "malformed scene was accepted");
    }

    // The writer applies shared local validation even without an asset catalog.
    for (const Scene& bad : {
             Scene{{Entity{1, 42, 2, 0, 0}, Entity{2, 42, 1, 0, 0}}},
             Scene{{Entity{1, 42, 0, 0, 0}, Entity{1, 42, 0, 0, 0}}},
             Scene{{Entity{1, 42, 999, 0, 0}}}}) {
        bool rejected = false;
        try { (void)save(bad); } catch (const std::exception&) { rejected = true; }
        require(rejected, "writer accepted invalid local hierarchy or identity");
    }

    // Failed replacement leaves the old live document byte-for-byte unchanged.
    const std::string before_failed_load = save(live);
    bool failed = false;
    try { load_into(live, malformed[6], assets); } catch (const std::exception&) { failed = true; }
    require(failed && save(live) == before_failed_load,
            "failed load changed the live scene");

    // A valid candidate commits through vector::swap after every validation succeeds.
    const std::string replacement = "SCENE 2\nCOUNT 1\nENTITY 2001 77 0 5 -8\n";
    load_into(live, replacement, assets);
    require(save(live) == replacement, "valid scene replacement did not commit");
    std::cout << "scene persistence: round-trip, migration, cycles, strict parsing, transaction PASS\n";
}
