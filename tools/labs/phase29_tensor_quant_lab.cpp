// Phase 29 bounded CPU oracle: tensors, broadcasting, stable probabilities,
// dense gradients and quantized storage. No GGUF/ONNX parser, GPU backend,
// generalized autograd, production numerical BLAS or SIMD performance claim.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace allmight::ml {
void require(bool yes,const char* why) {
    if (!yes) throw std::runtime_error(why);
}
bool near(double a,double b,double eps=1e-8) {
    return std::abs(a-b)<=eps*std::max({1.,std::abs(a),std::abs(b)});
}
std::size_t checked_product(const std::vector<std::size_t>& shape) {
    std::size_t count=1;
    for(auto n:shape) {
        if(n==0) return 0;
        require(count<=std::numeric_limits<std::size_t>::max()/n,
                "tensor extent multiplication overflow");
        count*=n;
    }
    return count;
}
std::vector<std::size_t> contiguous_strides(
    const std::vector<std::size_t>& shape) {
    std::vector<std::size_t> strides(shape.size());
    std::size_t accum=1;
    for(std::size_t d=shape.size();d-->0;) {
        strides[d]=accum;
        if(shape[d]==0) {accum=0;continue;}
        require(accum<=std::numeric_limits<std::size_t>::max()/shape[d],
                "stride multiplication overflow");
        accum*=shape[d];
    }
    return strides;
}
struct Tensor {
    std::vector<std::size_t> shape,strides;
    std::vector<double> values;
    explicit Tensor(std::vector<std::size_t> dims)
        : shape(std::move(dims)),
          strides(contiguous_strides(shape)),
          values(checked_product(shape),0) {}
    std::size_t offset(std::span<const std::size_t> ix) const {
        require(ix.size()==shape.size(),"tensor rank mismatch");
        std::size_t at=0;
        for(std::size_t d=0;d<ix.size();++d) {
            require(ix[d]<shape[d],"tensor index out of bounds");
            // The construction of contiguous strides already checks extents.
            at+=ix[d]*strides[d];
        }
        require(at<values.size(),"tensor index overflow");
        return at;
    }
    double& at(std::initializer_list<std::size_t> ix) {
        return values[offset(ix)];
    }
    double at(std::initializer_list<std::size_t> ix) const {
        return values[offset(ix)];
    }
};

struct MatrixView {
    std::span<const double> storage;
    std::size_t rows{},cols{},row_stride{},col_stride{};
    double at(std::size_t r,std::size_t c) const {
        require(r<rows && c<cols,"matrix-view bounds");
        const std::size_t ix=r*row_stride+c*col_stride;
        require(ix<storage.size(),"view backing extent");
        return storage[ix];
    }
};
MatrixView view2(const Tensor& t) {
    require(t.shape.size()==2,"matrix rank required");
    return {t.values,t.shape[0],t.shape[1],t.strides[0],t.strides[1]};
}
MatrixView transpose(MatrixView a) {
    return {a.storage,a.cols,a.rows,a.col_stride,a.row_stride};
}
Tensor broadcast_add_2d(MatrixView a,MatrixView b) {
    // Lab uses explicit 2D broadcasting. General ranks align trailing axes.
    require((a.rows==b.rows || a.rows==1 || b.rows==1) &&
            (a.cols==b.cols || a.cols==1 || b.cols==1),
            "incompatible broadcast dimensions");
    Tensor out({std::max(a.rows,b.rows),std::max(a.cols,b.cols)});
    for(std::size_t r=0;r<out.shape[0];++r)
        for(std::size_t c=0;c<out.shape[1];++c)
            out.at({r,c})=a.at(a.rows==1?0:r,a.cols==1?0:c)+
                          b.at(b.rows==1?0:r,b.cols==1?0:c);
    return out;
}
std::vector<double> matvec(MatrixView m,std::span<const double> x) {
    require(m.cols==x.size(),"matvec inner dimension mismatch");
    std::vector<double> out(m.rows,0);
    for(std::size_t r=0;r<m.rows;++r)
        for(std::size_t c=0;c<m.cols;++c)
            out[r]+=m.at(r,c)*x[c];
    return out;
}
Tensor matmul(MatrixView a,MatrixView b) {
    require(a.cols==b.rows,"matmul incompatible dimensions");
    Tensor out({a.rows,b.cols});
    for(std::size_t i=0;i<a.rows;++i)
        for(std::size_t k=0;k<a.cols;++k)
            for(std::size_t j=0;j<b.cols;++j)
                out.at({i,j})+=a.at(i,k)*b.at(k,j);
    return out;
}
std::vector<double> softmax(std::span<const double> logits) {
    require(!logits.empty(),"softmax empty");
    for(double v:logits) require(std::isfinite(v),"nonfinite logit");
    const double maximum=*std::max_element(logits.begin(),logits.end());
    std::vector<double> probs;
    probs.reserve(logits.size());
    double sum=0;
    for(double value:logits) {
        const double w=std::exp(value-maximum);
        probs.push_back(w);
        sum+=w;
    }
    require(sum>0 && std::isfinite(sum),"invalid softmax denominator");
    for(auto& p:probs) p/=sum;
    return probs;
}
double cross_entropy_logits(std::span<const double> logits,
                            std::size_t target) {
    require(target<logits.size(),"target class out of bounds");
    for(double v:logits) require(std::isfinite(v),"nonfinite logit");
    const double maximum=*std::max_element(logits.begin(),logits.end());
    double sum=0;
    for(double x:logits) sum+=std::exp(x-maximum);
    return maximum+std::log(sum)-logits[target];
}
std::vector<double> dense_logits(MatrixView W,
                                 std::span<const double> x,
                                 std::span<const double> bias) {
    require(W.rows==bias.size(),"bias size mismatch");
    auto z=matvec(W,x);
    for(std::size_t i=0;i<z.size();++i) z[i]+=bias[i];
    return z;
}
double dense_loss_gradients(MatrixView W,
                            std::span<const double> x,
                            std::span<const double> bias,
                            std::size_t target,
                            std::vector<double>& dw,
                            std::vector<double>& db) {
    const auto logits=dense_logits(W,x,bias);
    const double loss=cross_entropy_logits(logits,target);
    auto probs=softmax(logits);
    require(target<probs.size(),"invalid target");
    probs[target]-=1.;
    dw.assign(W.rows*W.cols,0.);
    db=probs;
    for(std::size_t i=0;i<W.rows;++i)
        for(std::size_t j=0;j<W.cols;++j)
            dw[i*W.cols+j]=probs[i]*x[j];
    return loss;
}

struct Adam {
    std::vector<double> first,second;
    std::size_t step{};
    double beta1{0.9},beta2{0.999},eps{1e-8};
    explicit Adam(std::size_t n):first(n,0),second(n,0){}
    void update(std::vector<double>& weights,
                std::span<const double> grads,double lr) {
        require(weights.size()==grads.size() &&
                weights.size()==first.size(),"Adam extent mismatch");
        require(lr>0 && std::isfinite(lr),"invalid learning rate");
        ++step;
        const double b1corr=1.-std::pow(beta1,double(step));
        const double b2corr=1.-std::pow(beta2,double(step));
        for(std::size_t i=0;i<weights.size();++i) {
            first[i]=beta1*first[i]+(1-beta1)*grads[i];
            second[i]=beta2*second[i]+(1-beta2)*grads[i]*grads[i];
            const double mean=first[i]/b1corr;
            const double variance=second[i]/b2corr;
            weights[i]-=lr*mean/(std::sqrt(variance)+eps);
        }
    }
};

struct SymmetricQ8 {
    double scale{};
    std::vector<std::int8_t> codes;
};
SymmetricQ8 quantize_q8(std::span<const double> input) {
    require(!input.empty(),"empty quant group");
    double maxabs=0;
    for(double v:input) {
        require(std::isfinite(v),"quant input must be finite");
        maxabs=std::max(maxabs,std::abs(v));
    }
    // All-zero group has a representable zero with nonzero scale.
    const double scale=maxabs>0?maxabs/127.:1.;
    SymmetricQ8 result{scale,{}};
    for(double v:input) {
        const double rounded=std::nearbyint(v/scale);
        result.codes.push_back(std::int8_t(
            std::clamp(rounded,-127.,127.)));
    }
    return result;
}
std::vector<double> dequant_q8(const SymmetricQ8& q) {
    std::vector<double> out;
    for(auto code:q.codes) out.push_back(double(code)*q.scale);
    return out;
}
double quantized_dot(std::span<const std::int8_t> codes,
                     double scale,std::span<const double> x) {
    require(codes.size()==x.size(),"quantized dot extent");
    double sum=0;
    for(std::size_t i=0;i<x.size();++i)
        sum+=double(codes[i])*scale*x[i];
    return sum;
}
std::vector<std::uint8_t> pack_signed_int4(
    std::span<const std::int8_t> values) {
    std::vector<std::uint8_t> packed((values.size()+1)/2,0);
    for(std::size_t i=0;i<values.size();++i) {
        const int v=values[i];
        require(v>=-8 && v<=7,"signed int4 out of range");
        const auto nibble=std::uint8_t(v&15);
        packed[i/2]|=std::uint8_t(nibble<<((i%2)*4));
    }
    return packed;
}
std::int8_t unpack_signed_int4(std::span<const std::uint8_t> bytes,
                               std::size_t index) {
    require(index/2<bytes.size(),"packed index out of bounds");
    const unsigned nibble=(bytes[index/2]>>((index%2)*4))&15u;
    return std::int8_t(nibble>=8?nibble-16:nibble);
}
bool two_of_four_or_less(std::span<const double> weights) {
    require(weights.size()%4==0,"2:4 requires groups of four");
    for(std::size_t i=0;i<weights.size();i+=4) {
        unsigned nonzeros=0;
        for(std::size_t j=0;j<4;++j)
            if(weights[i+j]!=0) ++nonzeros;
        if(nonzeros>2) return false;
    }
    return true;
}
struct BinaryCentroids {
    double low{},high{};
    std::vector<std::uint8_t> bits;
    std::size_t count{};
};
BinaryCentroids quantize_two_centroids(std::span<const double> weights) {
    require(!weights.empty(),"binary quant requires samples");
    const double mean=std::accumulate(weights.begin(),weights.end(),0.) /
                      double(weights.size());
    double low_sum=0,high_sum=0;
    std::size_t low_count=0,high_count=0;
    for(double v:weights) {
        require(std::isfinite(v),"nonfinite binary weight");
        if(v<mean) {low_sum+=v;++low_count;}
        else {high_sum+=v;++high_count;}
    }
    // This is a one-step mean partition, not an optimal k-means solver.
    const double low=low_count?low_sum/double(low_count):mean;
    const double high=high_count?high_sum/double(high_count):mean;
    BinaryCentroids q{low,high,
        std::vector<std::uint8_t>((weights.size()+7)/8,0),
        weights.size()};
    for(std::size_t i=0;i<weights.size();++i)
        if(weights[i]>=mean)
            q.bits[i/8]|=std::uint8_t(1u<<(i%8));
    return q;
}
double binary_dequant(const BinaryCentroids& q,std::size_t index) {
    require(index<q.count,"binary index");
    return ((q.bits[index/8]>>(index%8))&1u)?q.high:q.low;
}

void test_shape_and_matmul() {
    Tensor a({2,3});
    a.values={1,2,3,4,5,6};
    require(a.strides==std::vector<std::size_t>({3,1}),"row strides");
    require(a.at({1,2})==6,"row-major offset");
    const auto tv=transpose(view2(a));
    require(tv.rows==3 && tv.cols==2 && tv.at(2,1)==6,
            "transpose borrow/stride");
    Tensor bias({1,3});bias.values={10,20,30};
    const auto sum=broadcast_add_2d(view2(a),view2(bias));
    require(sum.values==std::vector<double>({11,22,33,14,25,36}),
            "broadcast row");
    const std::array<double,3> x{1,2,1};
    const auto z=matvec(view2(a),x);
    require(z==std::vector<double>({8,20}),"matvec");
    const auto prod=matmul(view2(a),tv);
    require(prod.at({0,0})==14 && prod.at({1,1})==77,
            "matmul transpose");
    bool rejected=false;
    try{(void)Tensor({std::numeric_limits<std::size_t>::max(),2});}
    catch(const std::runtime_error&){rejected=true;}
    require(rejected,"checked dimension multiplication");
}
void test_probabilities_and_gradients() {
    const std::array<double,3> high{10000,10001,9999};
    const auto p=softmax(high);
    require(near(std::accumulate(p.begin(),p.end(),0.),1.),
            "stable normalized probs");
    require(p[1]>p[0] && p[0]>p[2],"relative logits");
    const double loss=cross_entropy_logits(high,1);
    require(std::isfinite(loss) && loss>0,"stable cross entropy");

    Tensor W({2,2});W.values={0.2,-0.3,0.4,0.1};
    const std::array<double,2> x{1.5,-0.5};
    const std::array<double,2> b{0.1,-0.1};
    std::vector<double> dw,db;
    (void)dense_loss_gradients(view2(W),x,b,1,dw,db);
    const double h=1e-6;
    W.values[0]+=h;
    const double plus=cross_entropy_logits(dense_logits(view2(W),x,b),1);
    W.values[0]-=2*h;
    const double minus=cross_entropy_logits(dense_logits(view2(W),x,b),1);
    require(near((plus-minus)/(2*h),dw[0],1e-6),
            "dense backward vs central finite differences");
    std::vector<double> weights{1.};
    const std::array<double,1> grad{2.};
    Adam adam(1);adam.update(weights,grad,0.01);
    require(near(weights[0],0.99),"Adam first-step bias correction");
}
void test_quant_and_sparsity() {
    const std::array<double,4> w{-1.,-.5,.5,1.};
    const auto q=quantize_q8(w);
    const auto restored=dequant_q8(q);
    require(q.codes[0]==-127 && q.codes[3]==127,"symmetric signed q8");
    require(near(restored.front(),-1.)&&near(restored.back(),1.),
            "q8 endpoint reconstruction");
    const std::array<double,4> x{1,1,1,1};
    require(near(quantized_dot(q.codes,q.scale,x),0,1e-5),
            "quant dot cancels symmetric entries");

    const std::array<std::int8_t,5> ints{-8,-1,0,7,3};
    const auto bytes=pack_signed_int4(ints);
    require(bytes.size()==3,"two nibbles per byte");
    for(std::size_t i=0;i<ints.size();++i)
        require(unpack_signed_int4(bytes,i)==ints[i],
                "signed nibble round trip");

    const std::array<double,8> sparse{0,1,0,2,0,0,3,4};
    require(two_of_four_or_less(sparse),"2:4 valid mask");
    const std::array<double,4> dense{1,2,3,0};
    require(!two_of_four_or_less(dense),"2:4 reject three nonzeros");

    const auto binary=quantize_two_centroids(w);
    require(binary.bits.size()==1 && binary.count==4,
            "binary one bit per sample");
    require(near(binary_dequant(binary,0),-.75)&&
            near(binary_dequant(binary,3),.75),"two-centroid mean split");
}
} // namespace allmight::ml

int main() {
    try {
        allmight::ml::test_shape_and_matmul();
        allmight::ml::test_probabilities_and_gradients();
        allmight::ml::test_quant_and_sparsity();
        std::cout<<"phase29_tensor_quant_lab: all checks passed\n";
    } catch(const std::exception& e) {
        std::cerr<<"phase29_tensor_quant_lab: FAIL: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
}
