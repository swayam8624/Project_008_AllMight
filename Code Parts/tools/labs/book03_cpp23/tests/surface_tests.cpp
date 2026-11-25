#include "book03/surface.hpp"
#include <iostream>
using namespace book03::surface;
void check(bool b){if(!b)throw std::runtime_error("surface contract");}
int main(){std::vector<V> p;for(int x=-4;x<=4;++x)for(int y=-4;y<=4;++y)p.push_back({.1*x,.1*y,2+.02*x-.03*y});auto n=fit_plane(p);double agreement=std::abs((- .2*n.normal[0]+.3*n.normal[1]+n.normal[2])/std::sqrt(1.13));check(agreement>1-1e-12&&n.variation<1e-12);try{(void)fit_plane({{0,0,0},{1,0,0},{2,0,0}});check(false);}catch(const std::domain_error&){}
Volume v(2,2,100,.02,.1);check(!v.crossing(0,0));v.integrate(0,0,1.01);v.integrate(0,0,1.03,3);check(std::abs(v.crossing(0,0).value()-1.025)<1e-12);check(v.at(0,0,0).value==1&&v.at(0,0,0).weight==4);check(v.at(0,0,99).weight==0);v.integrate(1,1,std::nullopt);check(!v.crossing(1,1));std::cout<<"P30 PASS normal agreement="<<agreement<<" weighted crossing="<<v.crossing(0,0).value()<<'\n';}
