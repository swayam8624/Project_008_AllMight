// Phase 30 reference laboratory: host-neutral production publication contracts.
// The goal is to make DCC/tool output validation, path safety, deterministic
// diagnostics, frame-sequence checks, staging and idempotent publication
// executable without requiring Blender, Maya, Houdini or Nuke in CI.

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace allmight {

enum class Severity { info, warning, error };

struct Diagnostic {
    std::string code;
    Severity severity;
    std::string message;

    bool operator==(const Diagnostic&) const = default;
};

struct AssetRecord {
    std::string logical_id;
    std::string relative_path;
    std::uint32_t schema_version = 1;
    std::uint64_t content_hash = 0;
};

std::uint64_t fnv1a64(std::string_view text) {
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : text) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ULL;
    }
    return h;
}

bool is_absolute_or_drive_path(std::string_view p) {
    if (p.empty()) return false;
    if (p.front() == '/' || p.front() == '\\') return true;
    return p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) != 0 && p[1] == ':';
}

std::vector<std::string> split_path(std::string_view path) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : path) {
        if (c == '/' || c == '\\') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) parts.push_back(current);
    return parts;
}

std::string normalize_relative_path(std::string_view raw) {
    if (raw.empty()) throw std::invalid_argument("empty asset path");
    if (is_absolute_or_drive_path(raw)) throw std::invalid_argument("absolute path rejected");

    std::vector<std::string> stack;
    for (const std::string& part : split_path(raw)) {
        if (part == "." || part.empty()) continue;
        if (part == "..") {
            if (stack.empty()) throw std::invalid_argument("path escapes publication root");
            stack.pop_back();
            continue;
        }
        if (part.find(':') != std::string::npos) throw std::invalid_argument("colon in path component");
        stack.push_back(part);
    }
    if (stack.empty()) throw std::invalid_argument("path normalizes to empty");

    std::ostringstream out;
    for (std::size_t i = 0; i < stack.size(); ++i) {
        if (i != 0) out << '/';
        out << stack[i];
    }
    return out.str();
}

std::vector<Diagnostic> validate_manifest(std::vector<AssetRecord>& records) {
    std::vector<Diagnostic> diagnostics;
    std::set<std::string> ids;
    std::set<std::string> paths;

    for (AssetRecord& record : records) {
        if (record.logical_id.empty()) {
            diagnostics.push_back({"ASSET001", Severity::error, "empty logical asset id"});
        } else if (!ids.insert(record.logical_id).second) {
            diagnostics.push_back({"ASSET002", Severity::error, "duplicate logical asset id: " + record.logical_id});
        }

        try {
            record.relative_path = normalize_relative_path(record.relative_path);
            if (!paths.insert(record.relative_path).second) {
                diagnostics.push_back({"ASSET003", Severity::error, "duplicate publication path: " + record.relative_path});
            }
        } catch (const std::invalid_argument& e) {
            diagnostics.push_back({"ASSET004", Severity::error, e.what()});
        }

        if (record.schema_version == 0) {
            diagnostics.push_back({"ASSET005", Severity::error, "schema version zero is invalid"});
        }
    }

    std::stable_sort(diagnostics.begin(), diagnostics.end(), [](const Diagnostic& a, const Diagnostic& b) {
        if (a.code != b.code) return a.code < b.code;
        return a.message < b.message;
    });
    return diagnostics;
}

struct FrameSequence {
    int first = 0;
    int last = -1;
    std::set<int> present;

    std::vector<int> missing() const {
        if (last < first) throw std::invalid_argument("invalid frame range");
        std::vector<int> out;
        for (int f = first; f <= last; ++f) {
            if (!present.contains(f)) out.push_back(f);
        }
        return out;
    }
};

struct PublishPlan {
    std::vector<AssetRecord> records;
    std::string transaction_id;
    std::uint64_t plan_hash = 0;
};

std::uint64_t hash_plan(const std::vector<AssetRecord>& records) {
    std::ostringstream canonical;
    for (const AssetRecord& r : records) {
        canonical << r.logical_id << '\n'
                  << r.relative_path << '\n'
                  << r.schema_version << '\n'
                  << r.content_hash << '\n';
    }
    return fnv1a64(canonical.str());
}

PublishPlan build_plan(std::vector<AssetRecord> records) {
    const auto diagnostics = validate_manifest(records);
    const bool has_error = std::any_of(diagnostics.begin(), diagnostics.end(), [](const Diagnostic& d) {
        return d.severity == Severity::error;
    });
    if (has_error) throw std::runtime_error("cannot publish invalid manifest");

    std::sort(records.begin(), records.end(), [](const AssetRecord& a, const AssetRecord& b) {
        if (a.logical_id != b.logical_id) return a.logical_id < b.logical_id;
        return a.relative_path < b.relative_path;
    });

    const std::uint64_t h = hash_plan(records);
    std::ostringstream id;
    id << "tx-" << std::hex << h;
    return {std::move(records), id.str(), h};
}

class PublicationStore {
public:
    enum class Result { published, already_current };

    Result commit(const PublishPlan& plan) {
        if (current_hash_ && *current_hash_ == plan.plan_hash) return Result::already_current;

        // Simulate staging: nothing becomes visible until every record has been
        // copied into the temporary map successfully.
        std::map<std::string, AssetRecord> staged;
        for (const AssetRecord& record : plan.records) {
            if (record.content_hash == 0) throw std::runtime_error("zero content hash rejected during staging");
            staged.emplace(record.logical_id, record);
        }

        visible_ = std::move(staged);
        current_hash_ = plan.plan_hash;
        return Result::published;
    }

    const AssetRecord* find(std::string_view logical_id) const {
        const auto it = visible_.find(std::string(logical_id));
        return it == visible_.end() ? nullptr : &it->second;
    }

private:
    std::map<std::string, AssetRecord> visible_;
    std::optional<std::uint64_t> current_hash_;
};

struct DccExport {
    std::string host;
    std::string scene_id;
    std::string object_id;
    std::string payload_path;
    std::string coordinate_convention;
    double unit_scale_meters = 1.0;
};

AssetRecord adapt_dcc_export(const DccExport& input) {
    if (input.host.empty() || input.scene_id.empty() || input.object_id.empty()) {
        throw std::invalid_argument("incomplete DCC identity");
    }
    if (!(input.unit_scale_meters > 0.0)) throw std::invalid_argument("invalid unit scale");
    if (input.coordinate_convention.empty()) throw std::invalid_argument("missing coordinate convention");

    AssetRecord record;
    record.logical_id = input.scene_id + ":" + input.object_id;
    record.relative_path = normalize_relative_path(input.payload_path);
    record.schema_version = 1;

    std::ostringstream canonical;
    canonical << input.host << '|'
              << input.scene_id << '|'
              << input.object_id << '|'
              << record.relative_path << '|'
              << input.coordinate_convention << '|'
              << input.unit_scale_meters;
    record.content_hash = fnv1a64(canonical.str());
    return record;
}

void tests() {
    {
        assert(normalize_relative_path("characters/./hero/../hero.glb") == "characters/hero.glb");

        bool rejected = false;
        try { (void)normalize_relative_path("../../secrets.txt"); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);

        rejected = false;
        try { (void)normalize_relative_path("/tmp/asset.glb"); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }

    {
        FrameSequence seq{1001, 1005, {1001, 1002, 1004, 1005}};
        assert((seq.missing() == std::vector<int>{1003}));
    }

    {
        const DccExport blender{
            "Blender", "workshop", "hero_crate", "geo/workshop/crate.glb", "RH_Y_UP", 1.0
        };
        const DccExport maya{
            "Maya", "workshop", "door", "geo/workshop/door.glb", "RH_Y_UP", 0.01
        };

        std::vector<AssetRecord> records{adapt_dcc_export(blender), adapt_dcc_export(maya)};
        const PublishPlan a = build_plan(records);
        std::reverse(records.begin(), records.end());
        const PublishPlan b = build_plan(records);
        assert(a.plan_hash == b.plan_hash);
        assert(a.transaction_id == b.transaction_id);

        PublicationStore store;
        assert(store.commit(a) == PublicationStore::Result::published);
        assert(store.find("workshop:hero_crate") != nullptr);
        assert(store.commit(a) == PublicationStore::Result::already_current);
    }

    {
        std::vector<AssetRecord> bad{
            {"same", "a.glb", 1, 10},
            {"same", "b.glb", 1, 20},
        };
        const auto diagnostics = validate_manifest(bad);
        assert(!diagnostics.empty());
        assert(diagnostics.front().code == "ASSET002");
    }

    {
        const DccExport invalid{"Houdini", "fx", "smoke", "../escape.vdb", "RH_Y_UP", 1.0};
        bool rejected = false;
        try { (void)adapt_dcc_export(invalid); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }
}

}  // namespace allmight

int main() {
    allmight::tests();
    std::cout << "phase30_pipeline_contract_lab: PASS\n";
    return 0;
}
