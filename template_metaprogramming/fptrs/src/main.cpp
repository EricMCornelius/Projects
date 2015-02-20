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
  typedef R(*fptr)(Args...);
};

template <typename R, typename... Args>
struct SigType<R(*)(Args...)> {
  template <std::size_t N>
  using args = typename std::tuple_element<N, std::tuple<Args...>>::type;

  typedef R result;
  typedef R(*fptr)(Args...);
};

template <typename R, typename C, typename... Args>
struct SigType<R (C::*)(Args...)> {
  template <std::size_t N>
  using args = typename std::tuple_element<N, std::tuple<Args...>>::type;

  typedef R result;
  typedef R(*fptr)(Args...);
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

void test_func(bool) {
  
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

  typedef typename SigType<void(*)(long)>::args<0> long_type;
  static_assert(std::is_same<long_type, long>::value, "must be long");

  typedef SigType<void(*)(bool)> bool_sig;
  typename bool_sig::fptr tmp = &test_func;
}
