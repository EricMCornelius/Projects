#include <iostream>
#include <type_traits>
#include <tuple>

template <typename T>
struct SigType;

template <typename R, typename... Args>
struct SigType<R(Args...)> {
  template <std::size_t N>
  using args = typename std::tuple_element<N, std::tuple<Args...>>::type;

  typedef std::tuple<Args...> tuple;
  typedef R result;
  typedef R(*fptr)(Args...);
};

template <typename R, typename... Args>
struct SigType<R(*)(Args...)> {
  template <std::size_t N>
  using args = typename std::tuple_element<N, std::tuple<Args...>>::type;

  typedef std::tuple<Args...> tuple;
  typedef R result;
  typedef R(*fptr)(Args...);
};

template <typename R, typename C, typename... Args>
struct SigType<R (C::*)(Args...)> {
  template <std::size_t N>
  using args = typename std::tuple_element<N, std::tuple<Args...>>::type;

  typedef std::tuple<Args...> tuple;
  typedef R result;
  typedef R(*fptr)(Args...);
};

template <typename R, typename C, typename... Args>
struct SigType<R (C::*)(Args...) const> {
  template <std::size_t N>
  using args = typename std::tuple_element<N, std::tuple<Args...>>::type;

  typedef std::tuple<Args...> tuple;
  typedef R result;
  typedef R(*fptr)(Args...);
};

template <typename T>
struct SigType {
  typedef SigType<decltype(&T::operator())> internal;

  template <std::size_t N>
  using args = typename std::tuple_element<N, typename internal::tuple>::type;

  typedef typename internal::result result;
  typedef typename internal::fptr fptr;
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

enum class test_enum : uint16_t {
  ONE = 1,
  TWO,
  THREE
};

test_enum test_func2(int, double, void*) {
  return test_enum::ONE;
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

  auto lambda_ex = [](int a, double b) -> decltype(a) { return a; };
  typedef SigType<decltype(lambda_ex)> lambda_sig;
  typedef typename lambda_sig::args<0> lambda_1;
  static_assert(std::is_same<lambda_1, int>::value, "must be same");

  typedef SigType<enum test_enum(int, double, void*)> complicated_sig;
  typename complicated_sig::fptr tmp2 = &test_func2;

  std::cout << sizeof(tmp2(0,0,nullptr)) << std::endl;
  std::cout << static_cast<int>(tmp2(0,0,nullptr)) << std::endl;
}
