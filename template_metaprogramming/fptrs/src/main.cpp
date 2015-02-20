#include <iostream>
#include <type_traits>
#include <tuple>

template <typename... Args>
struct SigType;

template <typename R, typename... Args>
struct SigType<R(Args...)> {
  template <std::size_t N>
  using args = typename std::tuple_element<N, std::tuple<Args...>>::type;

  typedef R result;
};

template <typename R, typename C, typename... Args>
struct SigType<R (C::*)(Args...)> {
  template <std::size_t N>
  using args = typename std::tuple_element<N, std::tuple<Args...>>::type;

  typedef R result;
};

struct Test {
  bool operator()(std::string) {
    return true;
  }

  bool test(double) {

  }
};

int fp_test(int, float) {
  return 0;
}

int main(int argc, char* argv[]) {
  typedef typename SigType<decltype(fp_test)>::args<0> int_type;
  typedef typename SigType<decltype(fp_test)>::args<1> float_type;
  static_assert(std::is_same<int_type, int>::value, "must be int");
  static_assert(std::is_same<float_type, float>::value, "must be float");

  typedef typename SigType<decltype(&Test::test)>::args<0> double_type;
  static_assert(std::is_same<double_type, double>::value, "must be double");
  typedef typename SigType<decltype(&Test::operator())>::args<0> string_type;
  static_assert(std::is_same<string_type, std::string>::value, "must be string");
}
