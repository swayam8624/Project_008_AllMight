#include <vendor_math.hpp>
int main() {
  // This compile-time check proves INTERFACE_COMPILE_DEFINITIONS propagated.
#ifndef VENDOR_MATH_IMPORTED
#error expected imported target usage requirement
#endif
  return vendor::forty_two() == 42 ? 0 : 1;
}
