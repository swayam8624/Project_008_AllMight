#include "book03/visibility.hpp"
#include <iostream>
#include <random>
using namespace book03;
using Triangle=std::array<DepthVertex,3>;
void check(bool b,const char* m){if(!b)throw std::runtime_error(m);}
Triangle tri(double a,double b,double c){return {{{{0,0},a},{{8,0},b},{{0,8},c}}};}
int main(){try{
    for(auto order:{VisibilityOrder::Forward,VisibilityOrder::Reverse}){
        auto q=[&](double x){return order==VisibilityOrder::Forward?x:1-x;};
        VisibilityBuffer b(8,8,{order,DepthStorage::Float64,TiePolicy::KeepFirst});
        auto near=tri(q(.25),q(.25),q(.25)),far=tri(q(.75),q(.75),q(.75));
        b.draw(far,2);b.draw(near,1);check(b.at(1,1)->id==1,"near wins");
        b.clear();b.draw(near,1);auto s=b.draw(far,2);
        check(b.at(1,1)->id==1&&s.passed==0&&s.failed==s.coverage.covered,"order independence");
        b.clear();b.draw(tri(q(1),q(1),q(1)),3);check(b.at(1,1)->id==3,"empty accepts far");
        b.clear();
        // Sloped surface crosses constant-depth surface; both draw orders.
        auto slope=tri(q(.125),q(.625),q(.125)),flat=tri(q(.375),q(.375),q(.375));
        for(int rev=0;rev<2;++rev){
            b.clear();if(rev){b.draw(flat,2);b.draw(slope,1);}else{b.draw(slope,1);b.draw(flat,2);}
            for(int y=0;y<8;++y)for(int x=0;x<8;++x){
                auto p=b.at(x,y);
                if(x+y>=7){check(!p,"outside");continue;}
                // Diagonal is exclusive for this triangle. Centers are
                // half integers, so crossing at x=4 is never a tie.
                check(p&&p->id==(x<4?1u:2u),"crossing visibility");
                double want=q(std::min(.375,.125+.5*(x+.5)/8));
                check(std::abs(p->depth-want)<1e-14,"screen linear depth");
            }
        }
    }
    for(auto tie:{TiePolicy::KeepFirst,TiePolicy::KeepLast,TiePolicy::LowestID}){
        VisibilityBuffer b(2,2,{VisibilityOrder::Forward,DepthStorage::Float32,tie});
        bool equal=false;
        b.submit(0,0,.5,9,equal);check(!equal,"empty not tie");
        b.submit(0,0,.5+1e-10,3,equal);check(equal,"storage tie");
        check(b.at(0,0)->id==(tie==TiePolicy::KeepFirst?9u:3u),"tie policy");
        b.clear();b.submit(0,0,.5,3,equal);b.submit(0,0,.5,9,equal);
        check(b.at(0,0)->id==(tie==TiePolicy::KeepLast?9u:3u),"reverse tie order");
    }
    // Independent minimum/maximum oracle on quantized scalar observations.
    std::mt19937 rng(190019);
    for(auto order:{VisibilityOrder::Forward,VisibilityOrder::Reverse})
    for(auto storage:{DepthStorage::Float64,DepthStorage::Float32,DepthStorage::UNorm24}){
        VisibilityBuffer b(1,1,{order,storage,TiePolicy::LowestID});
        double best=order==VisibilityOrder::Forward?2:-1;std::uint32_t id=0;
        for(std::uint32_t i=0;i<3000;++i){
            double q=(rng()%1025)/1024.;
            double z=storage==DepthStorage::Float64?q:storage==DepthStorage::Float32?
                double(float(q)):std::round(q*16777215.)/16777215.;
            if(order==VisibilityOrder::Forward?z<best:z>best){best=z;id=i;}
            bool tie=false;b.submit(0,0,q,i,tie);
            check(b.at(0,0)->depth==best&&b.at(0,0)->id==id,"scalar oracle");
        }
    }
    bool rejected=false;try{(void)stored_depth(std::numeric_limits<double>::quiet_NaN(),DepthStorage::Float32);}
    catch(const std::invalid_argument&){rejected=true;}check(rejected,"NaN rejected");
    std::cout<<"Book03/P19 visibility contracts: PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
