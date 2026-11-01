#include <container/MPlex.h>
#include "../container/MFPlex.cc"
#include "test_check.h"
#include <stdexcept>
int main() {
 MPlex<int> p;
 p.add_high(10); p.add_high(20); p.add_high(30);
 p.del_index(2); p.del_high();
 FF_CHECK(p.high()==0 && p.count()==1 && p.available()==0 && p.OK());
 p.clear(); p.add_high(10); p.add_high(20); p.add_high(30);
 p.del_index(0); p.del_low();
 FF_CHECK(p.low()==2 && p.count()==1 && p.available()==0 && p.OK());
 p.clear(); p.add_high(1); p.add_high(2); p.append(p);
 FF_CHECK(p.count()==4 && p[2]==1 && p[3]==2 && p.OK());
 p.prepend(p);
 FF_CHECK(p.count()==8 && p[p.low()]==1 && p[p.low()+3]==2 && p.OK());
 p.clear(); const int only = p.add_high(1); p.del_index(only);
 bool threw=false; try { p.del_high(); } catch(const std::out_of_range&) { threw=true; }
 FF_CHECK(threw);
}
