#include "test_check.h"
#include "fgraphic/FFCanvasCoordinates.h"
#include <climits>
#include <limits>

int main() {
  using namespace ffgraph_canvas;
  int result=99;
  FF_CHECK(toInt(17.9,result) && result==17);
  FF_CHECK(toInt(-17.9,result) && result==-17);
  FF_CHECK(toInt(double(INT_MAX)+0.5,result) && result==INT_MAX);
  FF_CHECK(toInt(double(INT_MIN)-0.5,result) && result==INT_MIN);
  for(double value:{2147483648.0,-2147483649.0,4e9,-4e9,1e300,
                    std::numeric_limits<double>::quiet_NaN(),
                    std::numeric_limits<double>::infinity()}) {
    result=99;FF_CHECK(!toInt(value,result) && result==99);
    FF_CHECK(integer(value)==0);
  }
  FF_CHECK(supported(FFPoint(17.9,-17.9,9.9),true));
  FF_CHECK(!supported(FFPoint(4e9f,0,10),true));
  FF_CHECK(!supported(FFPoint(-4e9f,0,10),true));
  FF_CHECK(!supported(FFPoint(0,0,4e9f),true));
  FF_CHECK(supported(FFPoint(2147483520.0f,0,100),true));
  FF_CHECK(!supported(FFPoint(2147483520.0f,0,200),true));
  FF_CHECK(!supported(FFPoint(float(INT_MIN),0,1),true));
  FF_CHECK(!supported(FFPoint(0,0,std::numeric_limits<float>::quiet_NaN())));
  std::cout << "checked canvas conversions, legacy truncation and node footprints\n";
}
