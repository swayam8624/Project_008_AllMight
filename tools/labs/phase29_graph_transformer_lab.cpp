// Phase 29 second bounded oracle: subset graph validation, safe scheduling,
// GGUF header discovery, causal attention and autoregressive KV equivalence.
// Does not parse full ONNX or GGUF metadata/tensor records; does not run Metal.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace allmight::runtime {
void require(bool cond,const char* msg){
    if(!cond)throw std::runtime_error(msg);
}
bool near(double a,double b,double eps=1e-9){
    return std::abs(a-b)<=eps*std::max({1.,std::abs(a),std::abs(b)});
}

struct Node {
    std::string name,operator_name;
    std::vector<std::string> inputs;
    std::string output;
};

// A subset of ONNX-like SSA validation, not actual ONNX protobuf parsing.
// A production importer must also validate dimensions, dtypes, attrs, opsets,
// subgraphs, external data, optional/variadic inputs and versioned schemas.
std::vector<std::string> topo_nodes(
    const std::vector<Node>& nodes,
    const std::set<std::string>& graph_inputs) {
    std::map<std::string,std::size_t> producer;
    for(std::size_t i=0;i<nodes.size();++i){
        require(!nodes[i].output.empty(),"empty output name");
        require(!graph_inputs.contains(nodes[i].output),
                "node shadows an input");
        require(producer.emplace(nodes[i].output,i).second,
                "SSA duplicate output");
    }
    std::vector<std::set<std::size_t>> edges(nodes.size());
    std::vector<unsigned> indegree(nodes.size(),0);
    for(std::size_t i=0;i<nodes.size();++i) {
        for(const auto& name:nodes[i].inputs) {
            if(graph_inputs.contains(name))continue;
            const auto it=producer.find(name);
            require(it!=producer.end(),"unresolved graph input");
            const auto j=it->second;
            if(edges[j].insert(i).second)++indegree[i];
        }
    }
    std::set<std::size_t> ready;
    for(std::size_t i=0;i<nodes.size();++i)
        if(indegree[i]==0) ready.insert(i);
    std::vector<std::string> order;
    while(!ready.empty()){
        const std::size_t i=*ready.begin();ready.erase(ready.begin());
        order.push_back(nodes[i].name);
        for(const auto j:edges[i])
            if(--indegree[j]==0)ready.insert(j);
    }
    require(order.size()==nodes.size(),"cyclic graph cannot be topologically sorted");
    return order;
}

struct Range {std::size_t begin{},end{};};
std::vector<Range> partition_ranges(std::size_t count,
                                    std::size_t workers,
                                    std::size_t minimum_grain) {
    require(workers>0 && minimum_grain>0,"scheduler bad params");
    const std::size_t max_tasks= count==0?0:1+(count-1)/minimum_grain;
    const std::size_t tasks=std::min(workers,max_tasks);
    std::vector<Range> out;
    if(tasks==0)return out;
    const std::size_t base=count/tasks,remainder=count%tasks;
    std::size_t begin=0;
    for(std::size_t i=0;i<tasks;++i){
        const std::size_t length=base+(i<remainder?1:0);
        out.push_back({begin,begin+length});
        begin+=length;
    }
    require(begin==count,"scheduler did not cover work");
    return out;
}

// GGUF v3 header discovery only. No metadata/tensor parsing is performed.
// The official format includes extensible key-value and aligned tensor data.
std::uint64_t le64(const std::vector<std::uint8_t>& data,std::size_t at) {
    require(at<=data.size()&&data.size()-at>=8,"short GGUF integer");
    std::uint64_t v=0;
    for(unsigned i=0;i<8;++i)v|=(std::uint64_t(data[at+i])<<(8*i));
    return v;
}
std::uint32_t le32(const std::vector<std::uint8_t>& data,std::size_t at) {
    require(at<=data.size()&&data.size()-at>=4,"short GGUF integer");
    std::uint32_t v=0;
    for(unsigned i=0;i<4;++i)v|=(std::uint32_t(data[at+i])<<(8*i));
    return v;
}
struct GGUFHeader {
    std::uint32_t version{};
    std::uint64_t tensors{},metadata{};
};
GGUFHeader inspect_gguf_v3_header(const std::vector<std::uint8_t>& bytes) {
    require(bytes.size()>=24,"incomplete GGUF v3 header");
    require(bytes[0]=='G'&&bytes[1]=='G'&&
            bytes[2]=='U'&&bytes[3]=='F',"GGUF magic missing");
    const auto version=le32(bytes,4);
    require(version==3,"this oracle accepts GGUF v3 only");
    GGUFHeader h{version,le64(bytes,8),le64(bytes,16)};
    // A count alone is not sufficient to reserve huge memory for an
    // untrusted file. The real parser must bound metadata and descriptors
    // by the remaining bytes and configured resource limits.
    require(h.tensors<=100000 && h.metadata<=100000,
            "untrusted GGUF count exceeds policy");
    return h;
}

using Vec=std::vector<double>;
double dot(const Vec& a,const Vec& b){
    require(a.size()==b.size(),"attention head dimensions differ");
    double s=0;
    for(std::size_t i=0;i<a.size();++i)s+=a[i]*b[i];
    return s;
}
Vec attend_row(const Vec& query,
               const std::vector<Vec>& keys,
               const std::vector<Vec>& values) {
    require(!query.empty() && !keys.empty() &&
            keys.size()==values.size(),"attention inputs invalid");
    const auto width=query.size();
    const auto value_dim=values.front().size();
    require(value_dim>0,"value head dimension zero");
    for(std::size_t i=0;i<keys.size();++i) {
        require(keys[i].size()==width &&
                values[i].size()==value_dim,"attention row shape mismatch");
    }
    const double scale=1.0/std::sqrt(double(width));
    Vec logits;
    logits.reserve(keys.size());
    for(const auto& key:keys)
        logits.push_back(dot(query,key)*scale);
    const double high=*std::max_element(logits.begin(),logits.end());
    double total=0;
    for(auto& x:logits){x=std::exp(x-high);total+=x;}
    require(total>0 && std::isfinite(total),"invalid softmax normalizer");
    Vec result(value_dim,0);
    for(std::size_t j=0;j<keys.size();++j)
        for(std::size_t c=0;c<value_dim;++c)
            result[c]+=(logits[j]/total)*values[j][c];
    return result;
}
std::vector<Vec> causal_attention(
    const std::vector<Vec>& queries,
    const std::vector<Vec>& keys,
    const std::vector<Vec>& values) {
    require(queries.size()==keys.size() &&
            queries.size()==values.size(),"sequence length mismatch");
    std::vector<Vec> output;
    for(std::size_t t=0;t<queries.size();++t){
        // Causal mask excludes every future index j > t.
        output.push_back(attend_row(
           queries[t],
           std::vector<Vec>(keys.begin(),keys.begin()+static_cast<std::ptrdiff_t>(t+1)),
           std::vector<Vec>(values.begin(),values.begin()+static_cast<std::ptrdiff_t>(t+1))));
    }
    return output;
}
struct KVCache {
    std::vector<Vec> keys,values;
    Vec append_and_attend(const Vec& q,const Vec& k,const Vec& v) {
        keys.push_back(k);
        values.push_back(v);
        return attend_row(q,keys,values);
    }
};

void test_graph_and_schedule() {
    const std::vector<Node> out_of_order{
        {"relu","Relu",{"z"},"y"},
        {"dense","MatMul",{"input","weights"},"z"}
    };
    const auto sorted=topo_nodes(out_of_order,{"input","weights"});
    require(sorted==std::vector<std::string>({"dense","relu"}),
            "topological dependency order");
    bool cycle=false;
    try {
        (void)topo_nodes({{"a","Op",{"b"},"a"},
                          {"b","Op",{"a"},"b"}},{});
    }catch(const std::runtime_error&){cycle=true;}
    require(cycle,"cycle rejected");
    bool duplicate=false;
    try {
        (void)topo_nodes({{"a","Op",{"input"},"x"},
                          {"b","Op",{"input"},"x"}},{"input"});
    }catch(const std::runtime_error&){duplicate=true;}
    require(duplicate,"duplicate SSA name rejected");
    const auto ranges=partition_ranges(11,4,3);
    require(ranges.size()==4 && ranges.front().begin==0 &&
            ranges.back().end==11,"range work covered");
    for(std::size_t i=1;i<ranges.size();++i)
        require(ranges[i-1].end==ranges[i].begin,"no holes/overlap");
    require(partition_ranges(0,4,2).empty(),"no work means no tasks");
}
void test_gguf_guard() {
    std::vector<std::uint8_t> bytes(24,0);
    bytes[0]='G';bytes[1]='G';bytes[2]='U';bytes[3]='F';
    bytes[4]=3;bytes[8]=2;bytes[16]=1;
    const auto header=inspect_gguf_v3_header(bytes);
    require(header.version==3 && header.tensors==2 &&
            header.metadata==1,"GGUF header fields");
    bytes[4]=2;
    bool old=false;
    try{(void)inspect_gguf_v3_header(bytes);}
    catch(const std::runtime_error&){old=true;}
    require(old,"unsupported version rejected");
}
void test_attention_and_cache() {
    const std::vector<Vec> Q{{1,0},{0,1},{1,1}};
    const std::vector<Vec> K{{1,0},{0,1},{1,1}};
    const std::vector<Vec> V{{10,0},{0,20},{4,8}};
    const auto full=causal_attention(Q,K,V);
    require(full.size()==3 && near(full[0][0],10) &&
            near(full[0][1],0),"first token cannot attend to future");
    KVCache cache;
    for(std::size_t t=0;t<Q.size();++t) {
        const auto incremental=cache.append_and_attend(Q[t],K[t],V[t]);
        for(std::size_t c=0;c<incremental.size();++c)
            require(near(incremental[c],full[t][c]),
                    "incremental KV equals full causal attention");
    }
    // This is unscaled illustrative Q,K,V, not full transformer inference:
    // no layer norm, embeddings, FFN, learned weights or positional encoding.
}
} // namespace allmight::runtime
int main() {
    try {
        allmight::runtime::test_graph_and_schedule();
        allmight::runtime::test_gguf_guard();
        allmight::runtime::test_attention_and_cache();
        std::cout<<"phase29_graph_transformer_lab: all checks passed\n";
    }catch(const std::exception& e) {
        std::cerr<<"phase29_graph_transformer_lab: FAIL: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
}
