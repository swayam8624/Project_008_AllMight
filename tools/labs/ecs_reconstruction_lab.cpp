// PROJECTALLMIGHT Phase 15 witness: generational entities, sparse-set components, and archetype migration.
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

[[noreturn]] void fail(const std::string& m) { throw std::runtime_error(m); }
void require(bool c, const std::string& m) { if (!c) fail(m); }

struct Entity {
    std::uint32_t index = 0;
    std::uint32_t generation = 0;
    friend bool operator==(Entity, Entity) = default;
};

class EntityPool {
public:
    Entity create() {
        if (!free_.empty()) {
            const std::uint32_t index = free_.back();
            free_.pop_back();
            alive_[index] = true;
            return {index, generations_[index]};
        }
        require(generations_.size() < std::numeric_limits<std::uint32_t>::max(), "entity index exhausted");
        const auto index = static_cast<std::uint32_t>(generations_.size());
        generations_.push_back(1);
        alive_.push_back(true);
        return {index, 1};
    }

    void destroy(Entity e) {
        require(is_alive(e), "destroy requires a live entity");
        alive_[e.index] = false;
        require(generations_[e.index] != std::numeric_limits<std::uint32_t>::max(),
                "generation retirement reached");
        ++generations_[e.index];
        free_.push_back(e.index);
    }

    bool is_alive(Entity e) const {
        return e.index < generations_.size() && alive_[e.index] &&
               generations_[e.index] == e.generation;
    }

private:
    std::vector<std::uint32_t> generations_;
    std::vector<bool> alive_;
    std::vector<std::uint32_t> free_;
};

template <class T>
class SparseSet {
public:
    bool contains(Entity e) const {
        if (e.index >= sparse_.size()) return false;
        const std::size_t encoded = sparse_[e.index];
        if (encoded == 0) return false;
        const std::size_t dense = encoded - 1;
        return dense < entities_.size() && entities_[dense] == e;
    }

    void insert(Entity e, T value) {
        require(!contains(e), "duplicate component insertion");
        if (e.index >= sparse_.size()) sparse_.resize(static_cast<std::size_t>(e.index) + 1, 0);
        sparse_[e.index] = entities_.size() + 1;
        entities_.push_back(e);
        values_.push_back(std::move(value));
    }

    T& get(Entity e) {
        require(contains(e), "component lookup for absent or stale entity");
        return values_[sparse_[e.index] - 1];
    }

    const T& get(Entity e) const {
        require(contains(e), "component lookup for absent or stale entity");
        return values_[sparse_[e.index] - 1];
    }

    void erase(Entity e) {
        require(contains(e), "component erase for absent or stale entity");
        const std::size_t dense = sparse_[e.index] - 1;
        const std::size_t last = entities_.size() - 1;

        if (dense != last) {
            entities_[dense] = entities_[last];
            values_[dense] = std::move(values_[last]);
            sparse_[entities_[dense].index] = dense + 1; // repair reverse mapping.
        }

        sparse_[e.index] = 0;
        entities_.pop_back();
        values_.pop_back();
    }

    std::size_t size() const { return entities_.size(); }
    const std::vector<Entity>& entities() const { return entities_; }

private:
    std::vector<std::size_t> sparse_; // zero = absent; otherwise dense index + 1.
    std::vector<Entity> entities_;
    std::vector<T> values_;
};

struct Position { float x = 0.0F; float y = 0.0F; };
struct Velocity { float x = 0.0F; float y = 0.0F; };

struct Location {
    enum class Archetype : std::uint8_t { none, position_only, position_velocity };
    Archetype archetype = Archetype::none;
    std::size_t row = 0;
    std::uint32_t generation = 0;
};

struct PosChunk {
    std::vector<Entity> entities;
    std::vector<Position> positions;
};

struct PosVelChunk {
    std::vector<Entity> entities;
    std::vector<Position> positions;
    std::vector<Velocity> velocities;
};

class MiniArchetypeWorld {
public:
    Entity create(Position p) {
        Entity e = pool_.create();
        ensure_location(e.index);
        const std::size_t row = pos_.entities.size();
        pos_.entities.push_back(e);
        pos_.positions.push_back(p);
        locations_[e.index] = {Location::Archetype::position_only, row, e.generation};
        return e;
    }

    bool alive(Entity e) const { return pool_.is_alive(e); }

    void add_velocity(Entity e, Velocity v) {
        require(pool_.is_alive(e), "migration requires live entity");
        Location loc = location(e);
        require(loc.archetype == Location::Archetype::position_only, "entity already has velocity");

        const Position p = pos_.positions[loc.row];

        // Destination-first makes the source untouched until all destination vector
        // growth and copies have succeeded. This is not a general strong exception
        // guarantee for arbitrary component types, but it avoids half-removing the
        // source before this teaching migration has a destination.
        const std::size_t new_row = posvel_.entities.size();
        posvel_.entities.push_back(e);
        posvel_.positions.push_back(p);
        posvel_.velocities.push_back(v);

        erase_pos_row(loc.row);
        locations_[e.index] = {Location::Archetype::position_velocity, new_row, e.generation};
    }

    Position position(Entity e) const {
        require(pool_.is_alive(e), "position lookup requires live entity");
        const Location loc = location(e);
        if (loc.archetype == Location::Archetype::position_only) return pos_.positions[loc.row];
        if (loc.archetype == Location::Archetype::position_velocity) return posvel_.positions[loc.row];
        fail("entity has no position");
    }

    Velocity velocity(Entity e) const {
        require(pool_.is_alive(e), "velocity lookup requires live entity");
        const Location loc = location(e);
        require(loc.archetype == Location::Archetype::position_velocity, "entity has no velocity");
        return posvel_.velocities[loc.row];
    }

    std::size_t position_only_count() const { return pos_.entities.size(); }
    std::size_t moving_count() const { return posvel_.entities.size(); }

private:
    EntityPool pool_;
    PosChunk pos_;
    PosVelChunk posvel_;
    std::vector<Location> locations_;

    void ensure_location(std::uint32_t index) {
        if (index >= locations_.size()) locations_.resize(static_cast<std::size_t>(index) + 1);
    }

    Location location(Entity e) const {
        require(e.index < locations_.size(), "missing entity location");
        const Location loc = locations_[e.index];
        require(loc.generation == e.generation, "stale archetype location generation");
        return loc;
    }

    void erase_pos_row(std::size_t row) {
        require(row < pos_.entities.size(), "position row out of range");
        const std::size_t last = pos_.entities.size() - 1;
        if (row != last) {
            pos_.entities[row] = pos_.entities[last];
            pos_.positions[row] = pos_.positions[last];
            const Entity moved = pos_.entities[row];
            locations_[moved.index] = {Location::Archetype::position_only, row, moved.generation};
        }
        pos_.entities.pop_back();
        pos_.positions.pop_back();
    }
};

void test_generations() {
    EntityPool pool;
    Entity first = pool.create();
    require(pool.is_alive(first), "new entity should be alive");
    pool.destroy(first);
    require(!pool.is_alive(first), "destroyed handle must be stale");
    Entity reused = pool.create();
    require(reused.index == first.index, "free-list reuse expected");
    require(reused.generation != first.generation, "generation must change on reuse");
}

void test_sparse_set_swap_repair() {
    EntityPool pool;
    Entity a = pool.create();
    Entity b = pool.create();
    Entity c = pool.create();

    SparseSet<int> set;
    set.insert(a, 10);
    set.insert(b, 20);
    set.insert(c, 30);
    set.erase(b);

    require(!set.contains(b), "erased component must be absent");
    require(set.contains(a) && set.get(a) == 10, "first component corrupted");
    require(set.contains(c) && set.get(c) == 30, "moved dense entry reverse map was not repaired");
    require(set.size() == 2, "sparse-set size mismatch");

    pool.destroy(c);
    require(!set.contains(Entity{c.index, c.generation + 1}),
            "sparse membership must compare full entity identity, not only index");
}

void test_archetype_migration_and_location_repair() {
    MiniArchetypeWorld world;
    Entity a = world.create({1.0F, 2.0F});
    Entity b = world.create({3.0F, 4.0F});
    Entity c = world.create({5.0F, 6.0F});

    world.add_velocity(b, {0.5F, -1.0F});
    require(world.position_only_count() == 2 && world.moving_count() == 1, "archetype counts wrong");
    require(world.position(b).x == 3.0F && world.velocity(b).y == -1.0F, "migration lost component values");

    // b was in the middle of the source chunk, so c must have been swap-moved.
    require(world.position(c).x == 5.0F && world.position(c).y == 6.0F,
            "source-chunk swap removal failed to repair moved entity location");
    require(world.position(a).x == 1.0F, "unrelated entity changed during migration");
}

} // namespace

int main() {
    try {
        test_generations();
        test_sparse_set_swap_repair();
        test_archetype_migration_and_location_repair();
        std::cout
            << "ecs_reconstruction_lab: PASS\n"
            << "proved: generation invalidation, free-list reuse, sparse membership by full identity, "
               "swap-and-pop reverse-map repair, and a destination-first two-archetype migration\n"
            << "not proved: arbitrary component exception safety, parallel structural changes, "
               "chunk packing policies, scheduler conflicts, hierarchy propagation, or production KairoECS parity\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ecs_reconstruction_lab: FAIL: " << e.what() << '\n';
        return 1;
    }
}
