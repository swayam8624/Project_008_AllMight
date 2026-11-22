// CPU reference oracle for the staged GPU append/bin/hash lesson.
// This models the data contracts sequentially. It proves no GPU visibility,
// race-freedom, memory ordering, scheduling determinism, or performance claim.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

void Check(bool condition, const std::string& message)
{
    // Deliberately not assert(): checks stay active under -DNDEBUG and -O2.
    if (!condition)
        throw std::runtime_error("CHECK FAILED: " + message);
}

// The next attempted ticket is returned, then the counter advances.
// Refuse wraparound so a wrapped ticket cannot alias an earlier destination.
bool TryTakeCounterTicket(std::uint64_t& counter, std::uint64_t& ticket)
{
    if (counter == std::numeric_limits<std::uint64_t>::max())
        return false;

    ticket = counter;
    ++counter;
    return true;
}

struct AppendResult
{
    std::uint64_t attempted = 0;
    std::vector<int> stored;
    bool overflow = false;
    bool publishable = false;
};

AppendResult BoundedAppend(const std::vector<int>& input, std::size_t capacity)
{
    AppendResult result;
    result.stored.reserve(capacity);

    for (int value : input)
    {
        std::uint64_t ticket = 0;
        if (!TryTakeCounterTicket(result.attempted, ticket))
        {
            result.overflow = true;
            break;
        }

        if (ticket < capacity)
        {
            // Each ticket below capacity owns one unique slot.
            result.stored.push_back(value);
        }
        else
        {
            // Preserve the failure signal. Never present a truncated prefix as
            // a complete candidate set.
            result.overflow = true;
        }
    }

    result.publishable = !result.overflow;
    return result;
}

struct Grid
{
    double minX = -2.0;
    double minY = -2.0;
    double maxX = 2.0; // Upper edge is excluded: [min, max).
    double maxY = 2.0;
    double cellWidth = 1.0;
    int cellsX = 4;
    int cellsY = 4;
};

struct Point
{
    int id;
    double x;
    double y;
};

struct Cell
{
    int x;
    int y;
    int flat;
};

void ValidateGrid(const Grid& grid)
{
    Check(std::isfinite(grid.minX) && std::isfinite(grid.minY) &&
          std::isfinite(grid.maxX) && std::isfinite(grid.maxY) &&
          std::isfinite(grid.cellWidth) && grid.cellWidth > 0,
          "grid domain and positive width must be finite");
    Check(grid.minX < grid.maxX && grid.minY < grid.maxY &&
          grid.cellsX > 0 && grid.cellsY > 0,
          "grid extent and cell counts must be positive");
    const auto count = static_cast<std::int64_t>(grid.cellsX) * grid.cellsY;
    Check(count <= std::numeric_limits<int>::max(), "flat bin index fits int");
    const double nx = (grid.maxX - grid.minX) / grid.cellWidth;
    const double ny = (grid.maxY - grid.minY) / grid.cellWidth;
    Check(std::isfinite(nx) && std::isfinite(ny) &&
          std::ceil(nx) == grid.cellsX && std::ceil(ny) == grid.cellsY,
          "cell counts must cover exactly the declared domain");
}

std::optional<Cell> ToCell(const Grid& grid, double x, double y)
{
    ValidateGrid(grid);
    // Reject NaN and infinities before floor or integer conversion.
    if (!std::isfinite(x) || !std::isfinite(y))
        return std::nullopt;

    // Half-open domain gives one unambiguous owner to every boundary point.
    if (x < grid.minX || x >= grid.maxX ||
        y < grid.minY || y >= grid.maxY)
        return std::nullopt;

    // floor is essential for signed coordinates. Truncation toward zero would
    // map a negative fraction to cell zero instead of the cell to its left.
    const double fx = std::floor((x - grid.minX) / grid.cellWidth);
    const double fy = std::floor((y - grid.minY) / grid.cellWidth);
    // Validate in floating point before any potentially out-of-range cast.
    if (!std::isfinite(fx) || !std::isfinite(fy) ||
        fx < 0 || fx >= grid.cellsX || fy < 0 || fy >= grid.cellsY)
        return std::nullopt;
    const int ix = static_cast<int>(fx);
    const int iy = static_cast<int>(fy);

    return Cell{ix, iy, iy * grid.cellsX + ix};
}

struct BinBuild
{
    std::vector<std::uint64_t> counts;
    std::vector<std::uint64_t> offsets;
    std::vector<int> recordIds;
    std::uint64_t totalEntries = 0;
    bool overflow = false;
    bool publishable = false;
};

BinBuild BuildPointBins(const Grid& grid,
                        const std::vector<Point>& points,
                        std::size_t entryCapacity)
{
    ValidateGrid(grid);
    const int binCount = grid.cellsX * grid.cellsY;
    BinBuild result;
    result.counts.assign(static_cast<std::size_t>(binCount), 0); // Initialize counts.
    result.offsets.assign(static_cast<std::size_t>(binCount), 0);

    // Count pass. Invalid/out-of-domain points follow the declared reject rule.
    for (const Point& point : points)
    {
        const auto cell = ToCell(grid, point.x, point.y);
        if (!cell)
            continue;

        std::uint64_t ignoredTicket = 0;
        if (!TryTakeCounterTicket(result.counts[static_cast<std::size_t>(cell->flat)],
                                  ignoredTicket))
        {
            result.overflow = true;
            return result;
        }
    }

    // Exclusive scan. Check addition so neither total nor offsets can wrap.
    std::uint64_t running = 0;
    for (int bin = 0; bin < binCount; ++bin)
    {
        result.offsets[static_cast<std::size_t>(bin)] = running;
        const std::uint64_t count = result.counts[static_cast<std::size_t>(bin)];
        if (count > std::numeric_limits<std::uint64_t>::max() - running)
        {
            result.overflow = true;
            return result;
        }
        running += count;
    }
    result.totalEntries = running;

    // Capacity is checked before scatter. An undersized output is a failed build,
    // not a partially valid index.
    if (result.totalEntries > entryCapacity)
    {
        result.overflow = true;
        return result;
    }

    result.recordIds.assign(static_cast<std::size_t>(result.totalEntries), -1);
    std::vector<std::uint64_t> cursors(static_cast<std::size_t>(binCount), 0); // Initialize cursors.

    // Sequential CPU reference scatter preserves input order within each bin.
    // A GPU atomic cursor can assign a different within-bin order across runs.
    for (const Point& point : points)
    {
        const auto cell = ToCell(grid, point.x, point.y);
        if (!cell)
            continue;

        const std::size_t bin = static_cast<std::size_t>(cell->flat);
        const std::uint64_t local = cursors[bin]++;
        const std::uint64_t destination = result.offsets[bin] + local;
        Check(destination < result.recordIds.size(), "scatter destination is in range");
        result.recordIds[static_cast<std::size_t>(destination)] = point.id;
    }

    result.publishable = true;
    return result;
}

std::vector<std::pair<int, int>> DeterministicBinOrder(
    const Grid& grid, const std::vector<Point>& points)
{
    // Sorting by (bin, record ID) is a deterministic alternative when a stable
    // order is required instead of atomic arrival order.
    std::vector<std::pair<int, int>> entries;
    for (const Point& point : points)
    {
        const auto cell = ToCell(grid, point.x, point.y);
        if (cell)
            entries.emplace_back(cell->flat, point.id);
    }
    std::sort(entries.begin(), entries.end());
    return entries;
}

struct KeyValue
{
    int key;
    int value;
};

enum class DuplicatePolicy
{
    Reject,
    SumAfterDeterministicSort
};

std::optional<std::vector<KeyValue>> PreprocessKeys(
    std::vector<KeyValue> input, DuplicatePolicy policy)
{
    std::stable_sort(input.begin(), input.end(),
        [](const KeyValue& a, const KeyValue& b) { return a.key < b.key; });

    std::vector<KeyValue> output;
    for (std::size_t i = 0; i < input.size();)
    {
        std::size_t end = i + 1;
        while (end < input.size() && input[end].key == input[i].key)
            ++end;

        if (end - i > 1 && policy == DuplicatePolicy::Reject)
            return std::nullopt;

        if (policy == DuplicatePolicy::SumAfterDeterministicSort)
        {
            // Input order among equal keys is retained by stable_sort, so this
            // reference reduction has an explicitly fixed order.
            std::int64_t sum = 0;
            for (std::size_t j = i; j < end; ++j)
            {
                const auto term = static_cast<std::int64_t>(input[j].value);
                Check((term >= 0 && sum <= std::numeric_limits<std::int64_t>::max() - term) ||
                      (term < 0 && sum >= std::numeric_limits<std::int64_t>::min() - term),
                      "duplicate accumulator must not overflow");
                sum += term;
            }
            Check(sum >= std::numeric_limits<int>::min() &&
                  sum <= std::numeric_limits<int>::max(),
                  "duplicate reduction fits payload type");
            output.push_back(KeyValue{input[i].key, static_cast<int>(sum)});
        }
        else
        {
            output.push_back(input[i]);
        }
        i = end;
    }
    return output;
}

class FixedHashTable
{
public:
    static constexpr std::size_t kCapacity = 8;
    static constexpr int kEmpty = std::numeric_limits<int>::min();

    enum class InsertStatus { Inserted, Duplicate, Full, InvalidSentinel };

    FixedHashTable()
    {
        // Initialize every key and payload before the build phase.
        keys_.fill(kEmpty);
        values_.fill(0);
    }

    InsertStatus InsertBuildOnly(KeyValue item, std::size_t& probes)
    {
        Check(!published_, "cannot insert after query publication");
        probes = 0;
        if (item.key == kEmpty)
        {
            failed_ = true;
            return InsertStatus::InvalidSentinel;
        }

        const std::size_t first = Hash(item.key);
        for (std::size_t attempt = 0; attempt < kCapacity; ++attempt)
        {
            ++probes;
            const std::size_t slot = (first + attempt) % kCapacity;

            if (keys_[slot] == kEmpty)
            {
                // The successful CAS-equivalent claim owns the payload write.
                // This sequential oracle models slot ownership only; it does
                // not prove a concurrent GPU implementation is race-free.
                keys_[slot] = item.key;
                values_[slot] = item.value;
                return InsertStatus::Inserted;
            }

            if (keys_[slot] == item.key)
            {
                // Duplicates must be resolved before this build pass. Never
                // let two racing writers choose a payload by arrival order.
                failed_ = true;
                return InsertStatus::Duplicate;
            }
        }

        failed_ = true;
        return InsertStatus::Full; // Bounded: at most eight probes.
    }

    bool FinishBuild()
    {
        // A real GPU must additionally establish API-specific device-memory
        // visibility before its separate query dispatch.
        published_ = !failed_;
        return published_;
    }

    std::optional<int> LookupPublished(int key, std::size_t& probes) const
    {
        Check(published_, "query is rejected until complete build is published");
        probes = 0;
        const std::size_t first = Hash(key);
        for (std::size_t attempt = 0; attempt < kCapacity; ++attempt)
        {
            ++probes;
            const std::size_t slot = (first + attempt) % kCapacity;
            if (keys_[slot] == kEmpty)
                return std::nullopt;
            if (keys_[slot] == key)
                return values_[slot];
        }
        return std::nullopt;
    }

private:
    static std::size_t Hash(int key)
    {
        // This is the lesson's deliberately simple positive-key hash.
        const int remainder = key % static_cast<int>(kCapacity);
        return static_cast<std::size_t>(remainder < 0 ? remainder + kCapacity : remainder);
    }

    std::array<int, kCapacity> keys_{};
    std::array<int, kCapacity> values_{};
    bool failed_ = false;
    bool published_ = false;
};

void RunChecks()
{
    // Check invalid contracts explicitly rather than relying on undefined casts.
    Grid invalidGrid;
    invalidGrid.cellWidth = 0;
    bool badGridRejected = false;
    try { (void)ToCell(invalidGrid, 0, 0); }
    catch (const std::runtime_error&) { badGridRejected = true; }
    Check(badGridRejected, "zero-width grid is rejected before division");

    FixedHashTable unpublished;
    std::size_t beforeProbes = 0;
    bool earlyQueryRejected = false;
    try { (void)unpublished.LookupPublished(1, beforeProbes); }
    catch (const std::runtime_error&) { earlyQueryRejected = true; }
    Check(earlyQueryRejected, "query before publication is rejected");
    Check(unpublished.InsertBuildOnly({FixedHashTable::kEmpty, 0}, beforeProbes) ==
          FixedHashTable::InsertStatus::InvalidSentinel, "reserved key is rejected");
    Check(!unpublished.FinishBuild(), "sentinel failure invalidates the build");

    // Counter-width boundary: no wraparound ticket is ever returned.
    std::uint64_t counter = std::numeric_limits<std::uint64_t>::max() - 1;
    std::uint64_t ticket = 0;
    Check(TryTakeCounterTicket(counter, ticket), "last representable increment succeeds");
    Check(ticket == std::numeric_limits<std::uint64_t>::max() - 1,
          "ticket equals pre-increment counter");
    Check(counter == std::numeric_limits<std::uint64_t>::max(), "counter reaches max");
    Check(!TryTakeCounterTicket(counter, ticket), "counter refuses wraparound");

    // Append capacity is explicit; attempted is not the same as stored.
    const AppendResult append = BoundedAppend({10, 20, 30}, 2);
    Check(append.attempted == 3, "append counts all requests");
    Check(append.stored == std::vector<int>({10, 20}), "only in-capacity tickets store");
    Check(append.overflow, "append overflow is visible");
    Check(!append.publishable, "truncated append result is not published as complete");

    const AppendResult appendFits = BoundedAppend({4, 5}, 2);
    Check(appendFits.attempted == 2 && appendFits.stored.size() == 2,
          "in-capacity append stores every input");
    Check(!appendFits.overflow && appendFits.publishable, "complete append is publishable");

    // Signed coordinate, finite-value, and half-open-boundary cases.
    const Grid grid;
    const auto lower = ToCell(grid, -2.0, -2.0);
    Check(lower && lower->x == 0 && lower->y == 0 && lower->flat == 0,
          "lower boundary is included");
    Check(!ToCell(grid, 2.0, 0.0), "upper x boundary is excluded");
    Check(!ToCell(grid, 0.0, 2.0), "upper y boundary is excluded");
    Check(!ToCell(grid, -2.1, 0.0), "out-of-domain point is rejected");
    Check(!ToCell(grid, std::numeric_limits<double>::infinity(), 0.0),
          "nonfinite point is rejected");
    const double negativeFraction = std::floor((-0.2 - (-1.0)) / 1.0);
    Check(negativeFraction == 0.0, "offset floor gives expected signed-grid cell");
    Check(std::trunc(-0.2) == 0.0 && std::floor(-0.2) == -1.0,
          "truncation differs from floor for a negative fraction");

    const std::vector<Point> points{
        {1, -1.2, -0.1}, // cell (0,1), flat bin 4
        {2,  0.2, -0.1}, // cell (2,1), flat bin 6
        {3,  0.9,  0.9}, // cell (2,2), flat bin 10
        {4,  2.0,  0.0}, // excluded upper boundary
        {5, -2.1,  0.0}  // outside
    };
    const BinBuild bins = BuildPointBins(grid, points, 3);
    Check(bins.publishable && !bins.overflow, "exact-capacity bin build succeeds");
    Check(bins.totalEntries == 3, "only three in-domain points are binned");
    Check(bins.counts[4] == 1 && bins.counts[6] == 1 && bins.counts[10] == 1,
          "expected bins have one entry each");
    Check(bins.offsets[4] == 0 && bins.offsets[6] == 1 && bins.offsets[10] == 2,
          "exclusive scan offsets are 0,1,2");
    Check(bins.recordIds == std::vector<int>({1, 2, 3}),
          "scatter writes records to scanned ranges");
    const BinBuild binsTooSmall = BuildPointBins(grid, points, 2);
    Check(binsTooSmall.overflow && !binsTooSmall.publishable,
          "undersized bin array fails before publishing partial output");

    const std::vector<Point> sameBin{
        {8, -1.2, -0.1}, {6, -1.4, -0.4}
    };
    const auto stable = DeterministicBinOrder(grid, sameBin);
    Check(stable == std::vector<std::pair<int, int>>({{4, 6}, {4, 8}}),
          "(bin,ID) sort yields deterministic within-bin order");

    // Duplicate-key policy: reject by default, or explicitly pre-reduce.
    Check(!PreprocessKeys({{9, 4}, {9, 7}}, DuplicatePolicy::Reject),
          "duplicate policy can reject conflicting keys");
    const auto reduced = PreprocessKeys({{9, 4}, {9, 7}},
                                        DuplicatePolicy::SumAfterDeterministicSort);
    Check(reduced && reduced->size() == 1 && (*reduced)[0].key == 9 &&
          (*reduced)[0].value == 11, "duplicate sum is explicit and deterministic");

    // Collision trace: 1, 9 and 17 all begin at slot 1 in the 8-slot table.
    FixedHashTable table;
    std::size_t probes = 0;
    Check(table.InsertBuildOnly({1, 100}, probes) == FixedHashTable::InsertStatus::Inserted &&
          probes == 1, "key 1 claims slot 1");
    Check(table.InsertBuildOnly({9, 900}, probes) == FixedHashTable::InsertStatus::Inserted &&
          probes == 2, "key 9 probes slots 1 then 2");
    Check(table.InsertBuildOnly({17, 1700}, probes) == FixedHashTable::InsertStatus::Inserted &&
          probes == 3, "key 17 probes slots 1,2,3");
    Check(table.FinishBuild(), "complete collision table is published");
    Check(table.LookupPublished(9, probes) == std::optional<int>(900) && probes == 2,
          "lookup follows probe chain after build publication");
    Check(!table.LookupPublished(25, probes), "missing lookup stops at empty slot");

    // Duplicate insertions invalidate this build rather than race to choose a value.
    FixedHashTable duplicateTable;
    Check(duplicateTable.InsertBuildOnly({9, 4}, probes) == FixedHashTable::InsertStatus::Inserted,
          "first unique key inserted");
    Check(duplicateTable.InsertBuildOnly({9, 7}, probes) == FixedHashTable::InsertStatus::Duplicate,
          "duplicate key is rejected");
    Check(!duplicateTable.FinishBuild(), "failed duplicate build is not published");

    // Full-table failure is bounded to exactly eight probes and not published.
    FixedHashTable full;
    for (int key = 0; key < 8; ++key)
        Check(full.InsertBuildOnly({key, key * 10}, probes) ==
              FixedHashTable::InsertStatus::Inserted, "fill one table slot per key");
    Check(full.InsertBuildOnly({17, 170}, probes) == FixedHashTable::InsertStatus::Full,
          "ninth unique key fails in a full table");
    Check(probes == FixedHashTable::kCapacity, "full-table search is bounded to capacity probes");
    Check(!full.FinishBuild(), "full-table build is not published as complete");

    std::cout << "PASS: CPU reference contracts (append, bins, duplicates, bounded hash build)\n";
}

} // namespace

int main()
{
    try
    {
        RunChecks();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
