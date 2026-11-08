#include "test_check.h"
#include <ffgraph/FFreduce_crossing.h>
#include <algorithm>
#include <climits>
#include <cstdint>
#include <vector>

extern BOOL equal_matrix(int*,int*,int,int,int);
extern BOOL equal_matrixarray(int*,int*,int,int,int,int,int*);
namespace {
// Frozen pre-optimization active rectangle, including row/column-zero IDs.
BOOL reference(const int* a,const int* b,int rows,int columns,int stride) {
  for(int row=0;row<=rows;++row)for(int col=0;col<=columns;++col)
    if(a[row*stride+col]!=b[row*stride+col])return NO;
  return YES;
}
BOOL referenceArray(const int* a,const int* b,int stride,int maxNode,int levels,
                    int matrixSize,const int* order) {
  for(int level=0;level<levels;++level)
    if(!reference(a+level*matrixSize,b+level*matrixSize,
                  order[(maxNode+1)*level],order[(maxNode+1)*(level+1)],stride))return NO;
  return YES;
}
std::vector<int> values(int size) {
  std::vector<int> result(size);
  for(int i=0;i<size;++i)result[i]=i%7==0?INT_MIN:i%7==1?INT_MAX:i*37-100003;
  return result;
}
std::uint64_t checks=0;
void rectangle(int rows,int columns,int stride) {
  auto a=values((rows+2)*stride),b=a;
  const auto original=a;
  FF_CHECK(equal_matrix(a.data(),a.data(),rows,columns,stride)==YES);
  FF_CHECK(equal_matrix(a.data(),b.data(),rows,columns,stride)==YES);
  for(std::size_t index=0;index<b.size();++index) {
    const int previous=b[index];b[index]=previous==INT_MIN?INT_MAX:INT_MIN;
    const auto expected=reference(a.data(),b.data(),rows,columns,stride);
    const auto input=b;
    FF_CHECK(equal_matrix(a.data(),b.data(),rows,columns,stride)==expected);
    FF_CHECK(a==original && b==input);
    b[index]=previous;++checks;
  }
  FF_CHECK(a==original && b==original);
}
void arrays() {
  constexpr int stride=11,maxNode=14,levels=5,matrixSize=stride*stride+9;
  auto a=values((levels+1)*matrixSize),b=a;
  const auto original=a;
  std::vector<int> order((levels+1)*(maxNode+1),0);
  const int widths[]={0,4,1,7,0,5};
  for(int i=0;i<=levels;++i)order[i*(maxNode+1)]=widths[i];
  for(std::size_t index=0;index<b.size();++index) {
    const int previous=b[index];b[index]=previous==INT_MIN?INT_MAX:INT_MIN;
    const auto expected=referenceArray(a.data(),b.data(),stride,maxNode,levels,matrixSize,order.data());
    const auto input=b;
    FF_CHECK(equal_matrixarray(a.data(),b.data(),stride,maxNode,levels,matrixSize,order.data())==expected);
    FF_CHECK(a==original && b==input);
    b[index]=previous;++checks;
  }
  FF_CHECK(a==original && b==original);
  FF_CHECK(equal_matrixarray(a.data(),a.data(),stride,maxNode,levels,matrixSize,order.data())==YES);
  FF_CHECK(equal_matrixarray(nullptr,nullptr,stride,maxNode,0,matrixSize,nullptr)==YES);
}
}
int main() {
  for(int stride=1;stride<=9;++stride)
    for(int rows=0;rows<stride;++rows)for(int columns=0;columns<stride;++columns)
      rectangle(rows,columns,stride);
  for(const auto shape:std::vector<std::vector<int>>{{0,7,11},{9,0,13},{5,8,17},
      {3,31,37},{31,3,37},{12,12,13},{7,31,32},{31,7,32},{47,64,65},{3,512,1024}})
    rectangle(shape[0],shape[1],shape[2]);
  FF_CHECK(equal_matrix(nullptr,nullptr,-1,10,5)==YES);
  FF_CHECK(equal_matrix(nullptr,nullptr,10,-1,5)==YES);
  arrays();
  std::cout << checks << " matrix comparisons match frozen active-rectangle semantics\n";
}
