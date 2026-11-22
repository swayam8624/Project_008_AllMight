// Inspect this under GCC and Clang: generic code may inline and optimize,
// but a universal "zero overhead" claim cannot follow from one probe.
template<typename T>
T generic_sum3(T a, T b, T c) { return a + b + c; }

extern "C" float allmight_m018_sum3(float a, float b, float c) {
    return generic_sum3<float>(a, b, c);
}
