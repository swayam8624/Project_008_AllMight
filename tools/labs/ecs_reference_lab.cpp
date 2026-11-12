// Reference ECS laboratory: generational entity identity and sparse-set storage.
// It proves stale-handle rejection, swap-and-pop repair, component lifetime
// boundaries and randomized agreement with a simple reference model.

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace allmight {

struct Entity {
    std::uint32_t index = 0;
    std::uint32_t generation = 0;
    bool operator==(const Entity&) const = default;
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
        if (generations_.size() >= std::numeric_limits<std::uint32_t>::max()) {
            throw std::overflow_error("entity index space exhausted");
        }
        const auto index = static_cast<std::uint32_t>(generations_.size());
        generations_.push_back(1U);
        alive_.push_back(true);
        return {index, 1U};
    }

    bool alive(Entity e) const {
        return e.index < generations_.size() &&
               alive_[e.index] &&
               generations_[e.index] == e.generation;
    }

    bool destroy(Entity e) {
        if (!alive(e)) return false;
        alive_[e.index] = false;
        if (generations_[e.index] == std::numeric_limits<std::uint32_t>::max()) {
            // Retire this slot permanently instead of wrapping generation to a
            // value that could make an ancient handle valid again.
            return true;
        }
        ++generations_[e.index];
        free_.push_back(e.index);
        return true;
    }

    std::uint32_t generation_of(std::uint32_t index) const {
        return generations_.at(index);
    }

private:
    std::vector<std::uint32_t> generations_;
    std::vector<bool> alive_;
    std::vector<std::uint32_t> free_;
};

template <typename T>
class SparsePool {
public:
    bool contains(Entity e) const {
        if (e.index >= sparse_.size()) return false;
        const std::uint32_t row = sparse_[e.index];
        return row != npos &&
               row < entities_.size() &&
               entities_[row] == e;
    }

    T& insert(Entity e, T value) {
        if (contains(e)) throw std::logic_error("duplicate component");
        ensure_sparse(e.index);
        if (sparse_[e.index] != npos) {
            // A different generation with the same index may still be present;
            // structural cleanup must happen before reusing the index.
            throw std::logic_error("stale generation occupies sparse slot");
        }
        const auto row = static_cast<std::uint32_t>(entities_.size());
        entities_.push_back(e);
        values_.push_back(std::move(value));
        sparse_[e.index] = row;
        return values_.back();
    }

    bool erase(Entity e) {
        if (!contains(e)) return false;
        const std::uint32_t row = sparse_[e.index];
        const std::uint32_t last = static_cast<std::uint32_t>(entities_.size() - 1U);

        if (row != last) {
            entities_[row] = std::move(entities_[last]);
            values_[row] = std::move(values_[last]);
            sparse_[entities_[row].index] = row;
        }

        entities_.pop_back();
        values_.pop_back();
        sparse_[e.index] = npos;
        return true;
    }

    T& get(Entity e) {
        if (!contains(e)) throw std::out_of_range("component missing");
        return values_[sparse_[e.index]];
    }

    const T& get(Entity e) const {
        if (!contains(e)) throw std::out_of_range("component missing");
        return values_[sparse_[e.index]];
    }

    std::size_t size() const noexcept { return entities_.size(); }

    void validate() const {
        assert(entities_.size() == values_.size());
        for (std::size_t row = 0; row < entities_.size(); ++row) {
            const Entity e = entities_[row];
            assert(e.index < sparse_.size());
            assert(sparse_[e.index] == row);
        }
    }

private:
    static constexpr std::uint32_t npos = std::numeric_limits<std::uint32_t>::max();

    void ensure_sparse(std::uint32_t index) {
        if (index >= sparse_.size()) sparse_.resize(static_cast<std::size_t>(index) + 1U, npos);
    }

    std::vector<std::uint32_t> sparse_;
    std::vector<Entity> entities_;
    std::vector<T> values_;
};

struct Position {
    int x = 0;
    int y = 0;
    bool operator==(const Position&) const = default;
};

void deterministic_tests() {
    EntityPool entities;
    SparsePool<Position> positions;

    const Entity a = entities.create();
    const Entity b = entities.create();
    const Entity c = entities.create();

    positions.insert(a, {1, 2});
    positions.insert(b, {3, 4});
    positions.insert(c, {5, 6});
    positions.validate();

    assert(positions.erase(b));
    positions.validate();
    assert(!positions.contains(b));
    assert(positions.contains(a));
    assert(positions.contains(c));
    assert((positions.get(c) == Position{5, 6}));

    assert(entities.destroy(b));
    const Entity reused = entities.create();
    assert(reused.index == b.index);
    assert(reused.generation != b.generation);
    assert(!entities.alive(b));
    assert(entities.alive(reused));

    positions.insert(reused, {9, 10});
    assert(!positions.contains(b));
    assert(positions.contains(reused));
    positions.validate();
}

void randomized_differential_test() {
    EntityPool entities;
    SparsePool<int> pool;
    std::unordered_map<std::uint64_t, int> reference;
    std::vector<Entity> live;

    auto key = [](Entity e) {
        return (static_cast<std::uint64_t>(e.generation) << 32U) | e.index;
    };

    std::mt19937 rng(0xA11A11U);
    for (int step = 0; step < 4000; ++step) {
        const unsigned choice = rng() % 4U;
        if (choice == 0U || live.empty()) {
            live.push_back(entities.create());
        } else {
            const std::size_t i = static_cast<std::size_t>(rng()) % live.size();
            const Entity e = live[i];
            if (choice == 1U) {
                if (!pool.contains(e)) {
                    const int value = static_cast<int>(rng() % 10000U);
                    pool.insert(e, value);
                    reference[key(e)] = value;
                }
            } else if (choice == 2U) {
                const bool a = pool.erase(e);
                const bool b = reference.erase(key(e)) != 0U;
                assert(a == b);
            } else {
                pool.erase(e);
                reference.erase(key(e));
                assert(entities.destroy(e));
                live[i] = live.back();
                live.pop_back();
            }
        }

        pool.validate();
        for (const Entity e : live) {
            const auto it = reference.find(key(e));
            assert(pool.contains(e) == (it != reference.end()));
            if (it != reference.end()) assert(pool.get(e) == it->second);
        }
    }
}

} // namespace allmight

int main() {
    allmight::deterministic_tests();
    allmight::randomized_differential_test();
    std::cout << "ecs_reference_lab: PASS\n";
    return 0;
}
