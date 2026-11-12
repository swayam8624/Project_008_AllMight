// Phase 29 reference laboratory: tensor layout, broadcasting, quantization,
// GGUF header contracts, ONNX-like graph validation, attention and KV caching.
// This is a dependency-free CPU teaching oracle, not a replacement for ggml,
// ONNX Runtime, or a production transformer backend.

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace allmight {

std::size_t product(const std::vector<std::size_t>& shape) {
    if (shape.empty()) return 1;
    std::size_t p = 1;
    for (std::size_t d : shape) {
        if (d == 0) throw std::invalid_argument("zero-sized dimension");
        if (p > std::numeric_limits<std::size_t>::max() / d) {
            throw std::overflow_error("shape product overflow");
        }
        p *= d;
    }
    return p;
}

std::vector<std::size_t> row_major_strides(const std::vector<std::size_t>& shape) {
    std::vector<std::size_t> strides(shape.size(), 1);
    for (std::size_t i = shape.size(); i-- > 1;) {
        strides[i - 1] = strides[i] * shape[i];
    }
    return strides;
}

struct Tensor {
    std::vector<std::size_t> shape;
    std::vector<std::size_t> strides;
    std::vector<float> data;

    explicit Tensor(std::vector<std::size_t> s)
        : shape(std::move(s)), strides(row_major_strides(shape)), data(product(shape), 0.0F) {}

    Tensor(std::vector<std::size_t> s, std::vector<float> values)
        : shape(std::move(s)), strides(row_major_strides(shape)), data(std::move(values)) {
        if (data.size() != product(shape)) throw std::invalid_argument("tensor data size mismatch");
    }

    std::size_t offset(const std::vector<std::size_t>& index) const {
        if (index.size() != shape.size()) throw std::invalid_argument("rank mismatch");
        std::size_t o = 0;
        for (std::size_t i = 0; i < shape.size(); ++i) {
            if (index[i] >= shape[i]) throw std::out_of_range("tensor index");
            o += index[i] * strides[i];
        }
        return o;
    }

    float& at(const std::vector<std::size_t>& index) { return data.at(offset(index)); }
    const float& at(const std::vector<std::size_t>& index) const { return data.at(offset(index)); }
};

std::vector<std::size_t> broadcast_shape(
    const std::vector<std::size_t>& a,
    const std::vector<std::size_t>& b) {

    const std::size_t rank = std::max(a.size(), b.size());
    std::vector<std::size_t> out(rank, 1);
    for (std::size_t k = 0; k < rank; ++k) {
        const std::size_t ai = (k < rank - a.size()) ? 1 : a[k - (rank - a.size())];
        const std::size_t bi = (k < rank - b.size()) ? 1 : b[k - (rank - b.size())];
        if (ai != bi && ai != 1 && bi != 1) throw std::invalid_argument("incompatible broadcast");
        out[k] = std::max(ai, bi);
    }
    return out;
}

std::vector<float> stable_softmax(const std::vector<float>& x) {
    if (x.empty()) throw std::invalid_argument("softmax on empty vector");
    const float m = *std::max_element(x.begin(), x.end());
    std::vector<float> e(x.size());
    float sum = 0.0F;
    for (std::size_t i = 0; i < x.size(); ++i) {
        e[i] = std::exp(x[i] - m);
        sum += e[i];
    }
    if (!(sum > 0.0F) || !std::isfinite(sum)) throw std::runtime_error("invalid softmax normalization");
    for (float& v : e) v /= sum;
    return e;
}

struct QuantizedI8 {
    float scale = 1.0F;
    std::vector<std::int8_t> q;
};

QuantizedI8 quantize_symmetric_i8(const std::vector<float>& x) {
    float max_abs = 0.0F;
    for (float v : x) {
        if (!std::isfinite(v)) throw std::invalid_argument("non-finite quantization input");
        max_abs = std::max(max_abs, std::abs(v));
    }

    QuantizedI8 out;
    out.scale = (max_abs == 0.0F) ? 1.0F : max_abs / 127.0F;
    out.q.resize(x.size());
    for (std::size_t i = 0; i < x.size(); ++i) {
        const float r = std::round(x[i] / out.scale);
        const float c = std::clamp(r, -127.0F, 127.0F);
        out.q[i] = static_cast<std::int8_t>(c);
    }
    return out;
}

float dequantized_dot(const QuantizedI8& w, const std::vector<float>& x) {
    if (w.q.size() != x.size()) throw std::invalid_argument("dot size mismatch");
    float acc = 0.0F;
    for (std::size_t i = 0; i < x.size(); ++i) {
        acc += static_cast<float>(w.q[i]) * w.scale * x[i];
    }
    return acc;
}

template <typename T>
void append_le(std::vector<std::uint8_t>& bytes, T value) {
    static_assert(std::is_integral_v<T>);
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        bytes.push_back(static_cast<std::uint8_t>((value >> (8U * i)) & static_cast<T>(0xFF)));
    }
}

template <typename T>
T read_le(const std::vector<std::uint8_t>& bytes, std::size_t& pos) {
    static_assert(std::is_integral_v<T>);
    if (bytes.size() - std::min(bytes.size(), pos) < sizeof(T)) {
        throw std::runtime_error("truncated binary field");
    }
    T value = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(bytes[pos + i]) << (8U * i);
    }
    pos += sizeof(T);
    return value;
}

struct GgufHeader {
    std::uint32_t version = 0;
    std::uint64_t tensor_count = 0;
    std::uint64_t metadata_count = 0;
};

GgufHeader parse_gguf_header(const std::vector<std::uint8_t>& bytes) {
    std::size_t pos = 0;
    const std::uint32_t magic = read_le<std::uint32_t>(bytes, pos);
    constexpr std::uint32_t gguf_magic =
        static_cast<std::uint32_t>('G') |
        (static_cast<std::uint32_t>('G') << 8U) |
        (static_cast<std::uint32_t>('U') << 16U) |
        (static_cast<std::uint32_t>('F') << 24U);

    if (magic != gguf_magic) throw std::runtime_error("not a GGUF stream");

    GgufHeader h;
    h.version = read_le<std::uint32_t>(bytes, pos);
    h.tensor_count = read_le<std::uint64_t>(bytes, pos);
    h.metadata_count = read_le<std::uint64_t>(bytes, pos);

    if (h.version == 0) throw std::runtime_error("invalid GGUF version");
    constexpr std::uint64_t defensive_limit = 100000000ULL;
    if (h.tensor_count > defensive_limit || h.metadata_count > defensive_limit) {
        throw std::runtime_error("unreasonable GGUF counts");
    }
    return h;
}

struct Node {
    std::string op;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
};

void validate_onnx_like_graph(
    const std::vector<std::string>& graph_inputs,
    const std::vector<std::string>& initializers,
    const std::vector<Node>& nodes) {

    std::unordered_set<std::string> available;
    std::unordered_set<std::string> produced;

    for (const std::string& name : graph_inputs) {
        if (name.empty() || !available.insert(name).second) {
            throw std::runtime_error("duplicate/empty graph input");
        }
    }
    for (const std::string& name : initializers) {
        if (name.empty()) throw std::runtime_error("empty initializer");
        available.insert(name);
    }

    for (const Node& node : nodes) {
        if (node.op.empty() || node.outputs.empty()) throw std::runtime_error("malformed node");
        for (const std::string& input : node.inputs) {
            if (!input.empty() && !available.contains(input)) {
                throw std::runtime_error("node reads value before definition");
            }
        }
        for (const std::string& output : node.outputs) {
            if (output.empty() || available.contains(output) || !produced.insert(output).second) {
                throw std::runtime_error("SSA output violation");
            }
            available.insert(output);
        }
    }
}

float dot(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size()) throw std::invalid_argument("dot rank mismatch");
    return std::inner_product(a.begin(), a.end(), b.begin(), 0.0F);
}

std::vector<float> attention_one_query(
    const std::vector<float>& q,
    const std::vector<std::vector<float>>& keys,
    const std::vector<std::vector<float>>& values,
    std::size_t visible_count) {

    if (q.empty() || keys.empty() || keys.size() != values.size()) {
        throw std::invalid_argument("invalid attention inputs");
    }
    if (visible_count == 0 || visible_count > keys.size()) {
        throw std::invalid_argument("invalid causal visibility");
    }

    const float inv_sqrt_d = 1.0F / std::sqrt(static_cast<float>(q.size()));
    std::vector<float> logits;
    logits.reserve(visible_count);
    for (std::size_t i = 0; i < visible_count; ++i) {
        if (keys[i].size() != q.size()) throw std::invalid_argument("key width mismatch");
        logits.push_back(dot(q, keys[i]) * inv_sqrt_d);
    }
    const std::vector<float> probs = stable_softmax(logits);

    const std::size_t value_width = values.front().size();
    std::vector<float> out(value_width, 0.0F);
    for (std::size_t i = 0; i < visible_count; ++i) {
        if (values[i].size() != value_width) throw std::invalid_argument("value width mismatch");
        for (std::size_t j = 0; j < value_width; ++j) out[j] += probs[i] * values[i][j];
    }
    return out;
}

struct KvCache {
    std::size_t capacity;
    std::vector<std::vector<float>> keys;
    std::vector<std::vector<float>> values;

    explicit KvCache(std::size_t cap) : capacity(cap) {
        if (cap == 0) throw std::invalid_argument("zero KV capacity");
    }

    void append(std::vector<float> k, std::vector<float> v) {
        if (keys.size() >= capacity) throw std::overflow_error("KV cache capacity exceeded");
        if (k.empty() || v.empty()) throw std::invalid_argument("empty KV vector");
        if (!keys.empty() && k.size() != keys.front().size()) throw std::invalid_argument("key width changed");
        if (!values.empty() && v.size() != values.front().size()) throw std::invalid_argument("value width changed");
        keys.push_back(std::move(k));
        values.push_back(std::move(v));
    }
};

bool near(float a, float b, float eps = 1.0e-5F) {
    return std::abs(a - b) <= eps;
}

void run_tests() {
    {
        Tensor t({2, 3, 4});
        assert((t.strides == std::vector<std::size_t>{12, 4, 1}));
        t.at({1, 2, 3}) = 7.0F;
        assert(t.data.at(23) == 7.0F);
    }

    {
        assert((broadcast_shape({4, 1, 8}, {1, 7, 8}) == std::vector<std::size_t>{4, 7, 8}));
        bool rejected = false;
        try { (void)broadcast_shape({2, 3}, {4, 3}); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }

    {
        const auto p = stable_softmax({1000.0F, 1001.0F, 1002.0F});
        assert(std::isfinite(p[0]) && std::isfinite(p[1]) && std::isfinite(p[2]));
        assert(near(p[0] + p[1] + p[2], 1.0F));
        assert(p[2] > p[1] && p[1] > p[0]);
    }

    {
        const std::vector<float> w{0.25F, -0.50F, 1.0F, -0.125F};
        const std::vector<float> x{2.0F, 1.0F, 0.5F, 4.0F};
        const auto q = quantize_symmetric_i8(w);
        const float exact = dot(w, x);
        const float approx = dequantized_dot(q, x);
        assert(std::abs(exact - approx) < 0.03F);
    }

    {
        std::vector<std::uint8_t> bytes;
        append_le<std::uint32_t>(bytes,
            static_cast<std::uint32_t>('G') |
            (static_cast<std::uint32_t>('G') << 8U) |
            (static_cast<std::uint32_t>('U') << 16U) |
            (static_cast<std::uint32_t>('F') << 24U));
        append_le<std::uint32_t>(bytes, 3U);
        append_le<std::uint64_t>(bytes, 12ULL);
        append_le<std::uint64_t>(bytes, 27ULL);
        const GgufHeader h = parse_gguf_header(bytes);
        assert(h.version == 3U && h.tensor_count == 12ULL && h.metadata_count == 27ULL);

        bytes[0] = static_cast<std::uint8_t>('X');
        bool rejected = false;
        try { (void)parse_gguf_header(bytes); }
        catch (const std::runtime_error&) { rejected = true; }
        assert(rejected);
    }

    {
        validate_onnx_like_graph(
            {"x"},
            {"w", "b"},
            {
                {"MatMul", {"x", "w"}, {"mm"}},
                {"Add", {"mm", "b"}, {"y"}},
            });

        bool rejected = false;
        try {
            validate_onnx_like_graph(
                {"x"}, {},
                {
                    {"Add", {"future", "x"}, {"y"}},
                    {"Identity", {"x"}, {"future"}},
                });
        } catch (const std::runtime_error&) { rejected = true; }
        assert(rejected);
    }

    {
        const std::vector<float> q{1.0F, 0.0F};
        const std::vector<std::vector<float>> k{{1.0F, 0.0F}, {0.0F, 1.0F}, {1.0F, 1.0F}};
        const std::vector<std::vector<float>> v{{10.0F, 0.0F}, {0.0F, 10.0F}, {5.0F, 5.0F}};

        const auto first_token = attention_one_query(q, k, v, 1);
        assert(near(first_token[0], 10.0F) && near(first_token[1], 0.0F));

        const auto third_token = attention_one_query(q, k, v, 3);
        assert(third_token[0] > third_token[1]);
    }

    {
        KvCache cache(2);
        cache.append({1.0F, 2.0F}, {3.0F, 4.0F});
        cache.append({5.0F, 6.0F}, {7.0F, 8.0F});
        bool rejected = false;
        try { cache.append({9.0F, 10.0F}, {11.0F, 12.0F}); }
        catch (const std::overflow_error&) { rejected = true; }
        assert(rejected);
    }
}

}  // namespace allmight

int main() {
    allmight::run_tests();
    std::cout << "phase29_ml_runtime_lab: PASS\n";
    return 0;
}
