// PROJECTALLMIGHT Phase 29 reference witness: tensor layout -> quantized matvec -> attention runtime.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error(message);
}

void require(bool condition, const std::string& message) {
    if (!condition) fail(message);
}

bool near(float a, float b, float eps = 1.0e-5F) {
    return std::fabs(a - b) <= eps;
}

std::size_t element_count(const std::vector<std::size_t>& shape) {
    if (shape.empty()) return 1;
    std::size_t n = 1;
    for (const std::size_t dim : shape) {
        if (dim == 0) return 0;
        if (n > std::numeric_limits<std::size_t>::max() / dim) fail("tensor element-count overflow");
        n *= dim;
    }
    return n;
}

std::vector<std::size_t> contiguous_strides(const std::vector<std::size_t>& shape) {
    std::vector<std::size_t> strides(shape.size(), 1);
    std::size_t stride = 1;
    for (std::size_t i = shape.size(); i-- > 0;) {
        strides[i] = stride;
        if (shape[i] != 0 && stride > std::numeric_limits<std::size_t>::max() / shape[i]) {
            fail("tensor stride overflow");
        }
        stride *= shape[i];
    }
    return strides;
}

struct Tensor {
    std::vector<std::size_t> shape;
    std::vector<std::size_t> strides;
    std::vector<float> storage;

    Tensor(std::vector<std::size_t> s, std::vector<float> values)
        : shape(std::move(s)), strides(contiguous_strides(shape)), storage(std::move(values)) {
        require(storage.size() == element_count(shape), "tensor storage does not match shape");
    }

    std::size_t offset(const std::vector<std::size_t>& index) const {
        require(index.size() == shape.size(), "tensor rank mismatch");
        std::size_t off = 0;
        for (std::size_t axis = 0; axis < shape.size(); ++axis) {
            require(index[axis] < shape[axis], "tensor index out of bounds");
            off += index[axis] * strides[axis];
        }
        return off;
    }

    float at(const std::vector<std::size_t>& index) const {
        return storage[offset(index)];
    }
};

std::size_t broadcast_offset(
    const Tensor& input,
    const std::vector<std::size_t>& output_shape,
    const std::vector<std::size_t>& output_index) {

    require(output_shape.size() == output_index.size(), "broadcast output-rank mismatch");
    require(input.shape.size() <= output_shape.size(), "cannot broadcast a higher-rank input");

    const std::size_t rank_delta = output_shape.size() - input.shape.size();
    std::size_t offset = 0;
    for (std::size_t axis = 0; axis < output_shape.size(); ++axis) {
        require(output_index[axis] < output_shape[axis], "broadcast output index out of bounds");
        if (axis < rank_delta) continue;

        const std::size_t in_axis = axis - rank_delta;
        const std::size_t in_dim = input.shape[in_axis];
        const std::size_t out_dim = output_shape[axis];
        require(in_dim == 1 || in_dim == out_dim, "incompatible broadcast dimensions");
        const std::size_t in_index = (in_dim == 1) ? 0 : output_index[axis];
        offset += in_index * input.strides[in_axis];
    }
    return offset;
}

struct Q8Row {
    float scale = 0.0F;
    std::vector<std::int8_t> values;
};

Q8Row quantize_q8(const std::vector<float>& row) {
    Q8Row out;
    out.values.resize(row.size());

    float max_abs = 0.0F;
    for (const float x : row) {
        require(std::isfinite(x), "quantization input must be finite");
        max_abs = std::max(max_abs, std::fabs(x));
    }

    if (max_abs == 0.0F) return out;

    out.scale = max_abs / 127.0F;
    for (std::size_t i = 0; i < row.size(); ++i) {
        const float q = std::round(row[i] / out.scale);
        out.values[i] = static_cast<std::int8_t>(std::clamp(q, -127.0F, 127.0F));
    }
    return out;
}

float q8_dot(const Q8Row& weights, const std::vector<float>& x) {
    require(weights.values.size() == x.size(), "q8 matvec width mismatch");
    if (weights.scale == 0.0F) return 0.0F;

    float sum = 0.0F;
    for (std::size_t i = 0; i < x.size(); ++i) {
        sum += (static_cast<float>(weights.values[i]) * weights.scale) * x[i];
    }
    return sum;
}

std::vector<float> stable_softmax(const std::vector<float>& logits) {
    require(!logits.empty(), "softmax requires at least one value");
    const float max_logit = *std::max_element(logits.begin(), logits.end());
    require(std::isfinite(max_logit), "softmax logits must contain a finite maximum");

    std::vector<float> probabilities(logits.size());
    float sum = 0.0F;
    for (std::size_t i = 0; i < logits.size(); ++i) {
        require(std::isfinite(logits[i]), "softmax logits must be finite");
        probabilities[i] = std::exp(logits[i] - max_logit);
        sum += probabilities[i];
    }
    require(sum > 0.0F && std::isfinite(sum), "softmax normalization failed");
    for (float& p : probabilities) p /= sum;
    return probabilities;
}

float dot(const std::vector<float>& a, const std::vector<float>& b) {
    require(a.size() == b.size(), "dot-product width mismatch");
    return std::inner_product(a.begin(), a.end(), b.begin(), 0.0F);
}

struct KVCache {
    std::size_t width = 0;
    std::vector<std::vector<float>> keys;
    std::vector<std::vector<float>> values;

    explicit KVCache(std::size_t d) : width(d) {
        require(width > 0, "KV cache width must be nonzero");
    }

    void append(std::vector<float> key, std::vector<float> value) {
        require(key.size() == width && value.size() == width, "KV cache vector width mismatch");
        keys.push_back(std::move(key));
        values.push_back(std::move(value));
    }

    std::vector<float> attend(const std::vector<float>& query) const {
        require(query.size() == width, "query width mismatch");
        require(!keys.empty(), "cannot attend over an empty cache");

        const float inverse_sqrt_d = 1.0F / std::sqrt(static_cast<float>(width));
        std::vector<float> logits(keys.size());
        for (std::size_t i = 0; i < keys.size(); ++i) {
            logits[i] = dot(query, keys[i]) * inverse_sqrt_d;
        }
        const std::vector<float> probabilities = stable_softmax(logits);

        std::vector<float> output(width, 0.0F);
        for (std::size_t token = 0; token < values.size(); ++token) {
            for (std::size_t d = 0; d < width; ++d) {
                output[d] += probabilities[token] * values[token][d];
            }
        }
        return output;
    }
};

template <class UInt>
UInt read_le(const std::vector<std::uint8_t>& bytes, std::size_t& cursor) {
    static_assert(std::is_unsigned_v<UInt>);
    require(cursor + sizeof(UInt) <= bytes.size(), "truncated little-endian field");

    UInt value = 0;
    for (std::size_t i = 0; i < sizeof(UInt); ++i) {
        value |= static_cast<UInt>(bytes[cursor + i]) << (8U * i);
    }
    cursor += sizeof(UInt);
    return value;
}

template <class UInt>
void append_le(std::vector<std::uint8_t>& bytes, UInt value) {
    static_assert(std::is_unsigned_v<UInt>);
    for (std::size_t i = 0; i < sizeof(UInt); ++i) {
        bytes.push_back(static_cast<std::uint8_t>((value >> (8U * i)) & static_cast<UInt>(0xFFU)));
    }
}

struct GgufPrefix {
    std::uint32_t version = 0;
    std::uint64_t tensor_count = 0;
    std::uint64_t metadata_count = 0;
};

GgufPrefix parse_gguf_prefix(const std::vector<std::uint8_t>& bytes) {
    require(bytes.size() >= 4, "GGUF prefix is truncated");
    require(bytes[0] == 'G' && bytes[1] == 'G' && bytes[2] == 'U' && bytes[3] == 'F',
            "GGUF magic mismatch");

    std::size_t cursor = 4;
    GgufPrefix prefix;
    prefix.version = read_le<std::uint32_t>(bytes, cursor);
    prefix.tensor_count = read_le<std::uint64_t>(bytes, cursor);
    prefix.metadata_count = read_le<std::uint64_t>(bytes, cursor);
    require(prefix.version != 0, "GGUF version zero is invalid for this teaching parser");
    return prefix;
}

struct Node {
    std::string op;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
};

void validate_ssa_topological_graph(
    const std::vector<std::string>& graph_inputs,
    const std::vector<std::string>& initializers,
    const std::vector<Node>& nodes) {

    std::unordered_set<std::string> available;
    std::unordered_set<std::string> produced;

    for (const auto& name : graph_inputs) {
        require(!name.empty(), "graph input name must not be empty");
        require(available.insert(name).second, "duplicate graph input");
    }
    for (const auto& name : initializers) {
        require(!name.empty(), "initializer name must not be empty");
        available.insert(name);
    }

    for (const Node& node : nodes) {
        require(!node.op.empty(), "node operator name must not be empty");
        for (const auto& input : node.inputs) {
            if (!input.empty()) require(available.contains(input), "node consumes a value before it is available");
        }
        for (const auto& output : node.outputs) {
            require(!output.empty(), "node output name must not be empty");
            require(produced.insert(output).second, "SSA violation: duplicate node output");
            require(available.insert(output).second, "node output collides with an existing value");
        }
    }
}

void test_tensor_layout_and_broadcast() {
    Tensor matrix({2, 3}, {0, 1, 2, 3, 4, 5});
    require(matrix.strides == std::vector<std::size_t>({3, 1}), "row-major stride mismatch");
    require(matrix.at({1, 2}) == 5.0F, "tensor offset calculation failed");

    Tensor bias({3}, {10, 20, 30});
    require(bias.storage[broadcast_offset(bias, {2, 3}, {0, 1})] == 20.0F, "broadcast first row mismatch");
    require(bias.storage[broadcast_offset(bias, {2, 3}, {1, 2})] == 30.0F, "broadcast second row mismatch");

    bool rejected = false;
    try {
        (void)broadcast_offset(bias, {2, 4}, {1, 2});
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    require(rejected, "incompatible broadcasting must be rejected");
}

void test_quantized_matvec() {
    const std::vector<float> weights{0.25F, -0.5F, 1.0F, 0.125F};
    const std::vector<float> x{2.0F, 1.0F, -1.0F, 4.0F};
    const float reference = dot(weights, x);
    const Q8Row q = quantize_q8(weights);
    const float approximate = q8_dot(q, x);
    require(std::fabs(reference - approximate) < 0.03F, "q8 dot exceeded teaching error budget");

    const Q8Row zero = quantize_q8({0.0F, 0.0F});
    require(zero.scale == 0.0F && q8_dot(zero, {3.0F, -7.0F}) == 0.0F, "zero-row quantization failed");
}

void test_stable_softmax() {
    const auto p = stable_softmax({1000.0F, 1001.0F, 1002.0F});
    const float sum = std::accumulate(p.begin(), p.end(), 0.0F);
    require(near(sum, 1.0F, 1.0e-6F), "softmax probabilities do not sum to one");
    require(p[2] > p[1] && p[1] > p[0], "softmax ordering mismatch");
}

void test_attention_and_cache() {
    KVCache cache(2);
    cache.append({1.0F, 0.0F}, {10.0F, 0.0F});
    cache.append({0.0F, 1.0F}, {0.0F, 20.0F});

    const auto out_x = cache.attend({4.0F, 0.0F});
    require(out_x[0] > out_x[1], "query aligned to first key should favor first value");

    const auto out_y = cache.attend({0.0F, 4.0F});
    require(out_y[1] > out_y[0], "query aligned to second key should favor second value");

    KVCache causal(2);
    causal.append({1.0F, 0.0F}, {1.0F, 2.0F});
    const auto one_token = causal.attend({1.0F, 0.0F});
    require(near(one_token[0], 1.0F) && near(one_token[1], 2.0F), "single-token causal attention mismatch");
}

void test_gguf_prefix() {
    std::vector<std::uint8_t> bytes{'G', 'G', 'U', 'F'};
    append_le<std::uint32_t>(bytes, 3U);
    append_le<std::uint64_t>(bytes, 7U);
    append_le<std::uint64_t>(bytes, 11U);

    const GgufPrefix p = parse_gguf_prefix(bytes);
    require(p.version == 3 && p.tensor_count == 7 && p.metadata_count == 11, "GGUF prefix parse mismatch");

    bytes[0] = 'B';
    bool rejected = false;
    try {
        (void)parse_gguf_prefix(bytes);
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    require(rejected, "invalid GGUF magic must be rejected");
}

void test_graph_contract() {
    const std::vector<Node> valid{
        {"MatMul", {"x", "w"}, {"hidden"}},
        {"Add", {"hidden", "b"}, {"biased"}},
        {"Softmax", {"biased"}, {"probabilities"}},
    };
    validate_ssa_topological_graph({"x"}, {"w", "b"}, valid);

    bool rejected_missing = false;
    try {
        validate_ssa_topological_graph({"x"}, {"w"}, {{"Add", {"future", "x"}, {"y"}}});
    } catch (const std::runtime_error&) {
        rejected_missing = true;
    }
    require(rejected_missing, "use-before-definition must be rejected");

    bool rejected_ssa = false;
    try {
        validate_ssa_topological_graph({"x"}, {}, {{"Identity", {"x"}, {"y"}}, {"Identity", {"x"}, {"y"}}});
    } catch (const std::runtime_error&) {
        rejected_ssa = true;
    }
    require(rejected_ssa, "duplicate node output must violate SSA");
}

} // namespace

int main() {
    try {
        test_tensor_layout_and_broadcast();
        test_quantized_matvec();
        test_stable_softmax();
        test_attention_and_cache();
        test_gguf_prefix();
        test_graph_contract();
        std::cout
            << "phase29_ml_runtime_lab: PASS\n"
            << "proved: contiguous strides, broadcast indexing, q8 dot error bound, stable softmax, "
               "causal KV-cache attention, GGUF prefix validation, and backend-neutral SSA/topological graph contracts\n"
            << "not proved: full GGUF metadata/tensor decoding, ONNX protobuf parsing, operator coverage, "
               "SIMD/Metal kernels, tokenizer behavior, or production-model parity\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "phase29_ml_runtime_lab: FAIL: " << e.what() << '\n';
        return 1;
    }
}
