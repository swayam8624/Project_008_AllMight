// C++20 swept disk-AABB broadphase witness: conservative candidates, never final collisions.
// Build: c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror swept_broadphase_lab.cpp -o /tmp/swept_broadphase && /tmp/swept_broadphase
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

struct Vec { double x{},y{}; };
struct Disk { std::uint64_t id{}; Vec p{},v{}; double radius{}; };
struct Box { double minX{},maxX{},minY{},maxY{}; std::size_t index{}; };
void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
void validate(const Disk& d){
 require(std::isfinite(d.p.x)&&std::isfinite(d.p.y)&&std::isfinite(d.v.x)&&
         std::isfinite(d.v.y)&&std::isfinite(d.radius)&&d.radius>0,"invalid disk");
}
Box swept_box(const Disk& d,double dt,std::size_t index){
 validate(d);require(std::isfinite(dt)&&dt>=0,"invalid time");
 const double ex=std::fma(d.v.x,dt,d.p.x),ey=std::fma(d.v.y,dt,d.p.y);
 require(std::isfinite(ex)&&std::isfinite(ey),"nonfinite endpoint");
 const double negInf=-std::numeric_limits<double>::infinity();
 const double posInf=std::numeric_limits<double>::infinity();
 // Outward-round the conservative bounds to avoid loss at an exact boundary.
 const Box b{std::nextafter(std::min(d.p.x,ex)-d.radius,negInf),
             std::nextafter(std::max(d.p.x,ex)+d.radius,posInf),
             std::nextafter(std::min(d.p.y,ey)-d.radius,negInf),
             std::nextafter(std::max(d.p.y,ey)+d.radius,posInf),index};
 require(std::isfinite(b.minX)&&std::isfinite(b.maxX)&&
         std::isfinite(b.minY)&&std::isfinite(b.maxY),"nonfinite swept AABB");
 return b;
}
using Pair=std::pair<std::size_t,std::size_t>;
std::vector<Pair> broadphase(const std::vector<Disk>& disks,double dt){
 std::vector<Box> boxes;
 boxes.reserve(disks.size());
 for(std::size_t i=0;i<disks.size();++i) {
  for(std::size_t j=0;j<i;++j)require(disks[i].id!=disks[j].id,"duplicate disk id");
  boxes.push_back(swept_box(disks[i],dt,i));
 }
 std::sort(boxes.begin(),boxes.end(),[&](const Box& a,const Box& b){
  if(a.minX!=b.minX)return a.minX<b.minX;
  return disks[a.index].id<disks[b.index].id;
 });
 std::vector<Box> active;
 std::vector<Pair> pairs;
 for(const Box& box:boxes){
  // Everything ending strictly before this minimum X cannot overlap this box.
  active.erase(std::remove_if(active.begin(),active.end(),[&](const Box& a){
   return a.maxX<box.minX;
  }),active.end());
  for(const Box& a:active)if(a.minY<=box.maxY&&box.minY<=a.maxY){
   const auto i=std::min(a.index,box.index),j=std::max(a.index,box.index);
   pairs.emplace_back(i,j);
  }
  active.push_back(box);
 }
 std::sort(pairs.begin(),pairs.end());
 require(std::adjacent_find(pairs.begin(),pairs.end())==pairs.end(),"duplicate candidate");
 return pairs;
}
// Independent geometry-oracle polynomial, *not* using bounding boxes.
bool collides(const Disk& A,const Disk& B,double dt){
 const double px=B.p.x-A.p.x,py=B.p.y-A.p.y;
 const double vx=B.v.x-A.v.x,vy=B.v.y-A.v.y;
 const double R=A.radius+B.radius;
 const double c=px*px+py*py-R*R;
 if(c<=0)return true;
 const double a=vx*vx+vy*vy,b=2*(px*vx+py*vy);
 if(a==0||b>=0)return false;
 const double disc=std::fma(b,b,-4*a*c);
 if(disc<0)return false;
 const double root=std::sqrt(disc),q=-0.5*(b-root);
 return q>0&&c/q<=dt;
}
void verify(const std::vector<Disk>& disks,double dt){
 const auto candidates=broadphase(disks,dt);
 for(std::size_t i=0;i<disks.size();++i)for(std::size_t j=i+1;j<disks.size();++j)
  if(collides(disks[i],disks[j],dt))
   require(std::binary_search(candidates.begin(),candidates.end(),Pair{i,j}),
           "broadphase false negative");
}
int main(){
 {
  const std::vector<Disk> disks{{1,{0,0},{0,0},1},
   {2,{100,0},{-200,0},1},{3,{0,100},{0,0},1}};
  verify(disks,1);
  const auto p=broadphase(disks,1);
  require(p.size()==1&&p[0]==Pair{0,1},"expected one swept pair");
 }
 {
  // All disks widely spaced with zero velocity: no candidates expected.
  std::vector<Disk> disks;
  for(std::uint64_t i=0;i<100;++i)disks.push_back({i+1,{5.0*i,0},{0,0},1});
  require(broadphase(disks,1).empty(),"disjoint scene had candidates");
  verify(disks,1);
 }
 {
  std::mt19937_64 gen(0x5A17u);
  std::uniform_real_distribution<double> coord(-50,50),vel(-25,25),radius(0.2,4);
  for(int trial=0;trial<200;++trial){
   std::vector<Disk> disks;
   for(std::uint64_t i=0;i<35;++i)disks.push_back({i+1,{coord(gen),coord(gen)},
       {vel(gen),vel(gen)},radius(gen)});
   verify(disks,0.1+0.01*(trial%40));
  }
 }
 {
  bool rejected=false;
  try{(void)broadphase({{1,{0,0},{0,0},1},{1,{3,0},{0,0},1}},1);}
  catch(const std::runtime_error&){rejected=true;}
  require(rejected,"duplicate identity accepted");
  rejected=false;
  try{(void)broadphase({{1,{0,0},{0,0},1}},-1);}
  catch(const std::runtime_error&){rejected=true;}
  require(rejected,"negative dt accepted");
 }
 std::cout<<"swept broadphase: deterministic + 200 randomized scenes PASS\n";
}
