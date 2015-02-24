#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <tuple>
#include <array>
#include <functional>
#include <type_traits>

struct Value;
typedef std::string String;
typedef std::unordered_map<String, Value> Object;
typedef std::vector<Value> Array;
typedef double Number;
typedef std::uint32_t UInt32;
typedef std::int32_t Int32;
typedef std::uint64_t UInt64;
typedef std::int64_t Int64;
typedef bool Bool;
struct Null { };

struct Test {
  Test() {
    std::cout << "constructed Test" << std::endl;
  }

  Test(const Test& other) {
    std::cout << "copied constructed Test" << std::endl;
  }

  Test& operator = (const Test& other) {
    std::cout << "copy assigned Test" << std::endl;
    return *this;
  }

  Test& operator = (Test&& other) {
    std::cout << "move assigned Test" << std::endl;
    return *this;
  }

  Test(Test&& other) {
    std::cout << "move constructed Test" << std::endl;
  }

  std::string msg() {
    return "okay";
  }

  ~Test() {
    std::cout << "destructed Test" << std::endl;
  }
};

namespace detail {
  template <class T, class Tuple>
  struct Index;

  template <class T, class... Types>
  struct Index<typename std::remove_cv<T>::type, std::tuple<T, Types...>> {
    static const std::size_t value = 0;
  };

  template <class T, class U, class... Types>
  struct Index<T, std::tuple<U, Types...>> {
    static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
  };

  template<typename T> struct argument_type;
  template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };

  #define INIT_FUNC_ARRAY(name, signature, ...)                                                         \
  template <typename Functions, typename UnionType, typename Typelist, typename Head, typename... Tail> \
  auto init_##name##_impl(Functions& functions) {                                                       \
    functions[Index<Head, Typelist>::value] = __VA_ARGS__;                                              \
    return init_##name##_impl<Functions, UnionType, Typelist, Tail...>(functions);                      \
  }                                                                                                     \
                                                                                                        \
  template <typename Functions, typename UnionType, typename Typelist>                                  \
  auto init_##name##_impl(Functions& functions) {                                                       \
    return functions;                                                                                   \
  }                                                                                                     \
                                                                                                        \
  template <typename UnionType, typename... Args>                                                       \
  auto init_##name() {                                                                                  \
    std::array<std::function<signature>, sizeof...(Args)> functions;                                          \
    typedef std::tuple<Args...> Typelist;                                                               \
    return init_##name##_impl<decltype(functions), UnionType, Typelist, Args...>(functions);            \
  }                                                                                                     \

  INIT_FUNC_ARRAY(deleters, void(UnionType*), [](UnionType* ptr) {
    ptr->template get<Head>()->~Head();
  });

  INIT_FUNC_ARRAY(initializers, void(UnionType&, const UnionType&), [](UnionType& target, const UnionType& source) {
    ::new (&target.storage) Head(source.template as<Head>());
    target.tag = source.tag;
  });

  INIT_FUNC_ARRAY(copiers, void(UnionType&, const UnionType&), [](UnionType& target, const UnionType& source) {
    target.template as<Head>() = source.template as<Head>();
  });

  INIT_FUNC_ARRAY(movers, void(UnionType&, UnionType&&), [](UnionType& target, UnionType&& source) {
    target.template as<Head>() = std::move(source.template as<Head>());
  });

  // https://akrzemi1.wordpress.com/2013/10/10/too-perfect-forwarding/
  template <typename T, typename U>
  constexpr inline bool NonSelf()
  {
    using DecayedT = typename std::decay<T>::type;
    return !std::is_same<DecayedT, U>::value
    && !std::is_base_of<U, DecayedT>::value;
  }

  template <typename... Args>
  struct tagged_union {
    typedef std::tuple<Args...> typelist;

    uint16_t tag = 0;
    typename std::aligned_union<0, Args...>::type storage;

    void* ptr() {
      return reinterpret_cast<void*>(&storage);
    }

    const void* ptr() const {
      return reinterpret_cast<const void*>(&storage);
    }

    template <typename T>
    T& as() {
      return *get<T>();
    }

    template <typename T>
    const T& as() const {
      return *get<T>();
    }

    template <typename T>
    T* get() {
      return static_cast<T*>(ptr());
    }

    template <typename T>
    const T* get() const {
      return static_cast<const T*>(ptr());
    }

    template <typename T>
    void set(const T& val) {
      as<T>() = val;
      tag = Index<T, typelist>::value;
    };

    template <typename T>
    void set(T&& val) {
      as<T>() = std::forward<T>(val);
      tag = Index<T, typelist>::value;
    };

    template <typename T, typename std::enable_if<NonSelf<T, tagged_union>()>::type* = nullptr>
    tagged_union& operator = (T&& val) {
      std::cout << "union operator=&&" << std::endl;
      if (tag == Index<T, typelist>::value) {
        as<T>() = std::forward<val>();
      }
      else {
        deleters[tag](this);
        ::new (&storage) T(std::forward<val>());
      }
      return *this;
    }

    template <typename T>
    tagged_union& operator = (const T& val) {
      std::cout << "union operator=const&" << std::endl;
      if (tag == Index<T, typelist>::value) {
        as<T>() = val;
      }
      else {
        deleters[tag](this);
        ::new (&storage) T(val);
      }
      return *this;
    }

    template <typename T, typename... Init>
    tagged_union& emplace(Init&&... args) {
      if (tag)
        deleters[tag](this);

      ::new (&storage) T(std::forward<Init>(args)...);
      tag = Index<T, typelist>::value;
    }

    tagged_union() {
      std::cout << "default union" << std::endl;
    }

    tagged_union(const tagged_union& other) {
      std::cout << "const union&" << std::endl;
      tagged_union::initializers[other.tag](*this, other);
    }

    tagged_union& operator = (const tagged_union& other) {
      std::cout << "union operator=const union&" << std::endl;
      std::cout << tag << " " << other.tag << std::endl;
      if (tag == other.tag) {
        tagged_union::copiers[tag](*this, other);
      }
      else {
        tagged_union::deleters[tag](this);
        tagged_union::initializers[other.tag](*this, other);
      }
      return *this;
    }

    tagged_union& operator = (tagged_union&& other) {
      std::cout << "union operator=union&&" << std::endl;
      if (tag == other.tag) {
        tagged_union::movers[tag](*this, std::forward<tagged_union>(other));
      }
      else {
        tagged_union::deleters[tag](this);
        tagged_union::initializers[other.tag](*this, other);
      }
      return *this;
    }

    template <typename T, typename std::enable_if<NonSelf<T, tagged_union>()>::type* = nullptr>
    tagged_union(const T& val) {
      std::cout << "constT&" << std::endl;
      typedef typename std::remove_reference<T>::type type;
      ::new (&storage) type(val);
      tag = Index<type, typelist>::value;
    }

    template <typename T, typename std::enable_if<NonSelf<T, tagged_union>()>::type* = nullptr>
    tagged_union(T&& val) {
      std::cout << "T&&" << std::endl;
      typedef typename std::remove_reference<T>::type type;
      ::new (&storage) type(std::forward<T>(val));
      tag = Index<type, typelist>::value;
    }

    static const std::array<std::function<void(tagged_union*)>, sizeof...(Args)> deleters;
    static const std::array<std::function<void(tagged_union&,const tagged_union&)>, sizeof...(Args)> initializers;
    static const std::array<std::function<void(tagged_union&,const tagged_union&)>, sizeof...(Args)> copiers;
    static const std::array<std::function<void(tagged_union&,tagged_union&&)>, sizeof...(Args)> movers;

    ~tagged_union() {
      std::cout << "~union default" << std::endl;
      tagged_union::deleters[tag](this);
    }
  };

  template <typename... Args>
  const std::array<std::function<void(tagged_union<Args...>*)>, sizeof...(Args)> tagged_union<Args...>::deleters = init_deleters<tagged_union<Args...>, Args...>();

  template <typename... Args>
  const std::array<std::function<void(tagged_union<Args...>&,const tagged_union<Args...>&)>, sizeof...(Args)> tagged_union<Args...>::initializers = init_initializers<tagged_union<Args...>, Args...>();

  template <typename... Args>
  const std::array<std::function<void(tagged_union<Args...>&,const tagged_union<Args...>&)>, sizeof...(Args)> tagged_union<Args...>::copiers = init_copiers<tagged_union<Args...>, Args...>();

  template <typename... Args>
  const std::array<std::function<void(tagged_union<Args...>&,tagged_union<Args...>&&)>, sizeof...(Args)> tagged_union<Args...>::movers = init_movers<tagged_union<Args...>, Args...>();
}

struct Value {
  typedef detail::tagged_union<Null, String, Object, Array, Number, Bool, Int32, Int64, UInt32, UInt64, Test> data_type;
  data_type data;

  Value() { }

  Value(const Value& val) : data(val.data) {}

  Value(const char* val) : Value(String(val)) {}

  Value& operator = (const Value& other) {
    data = other.data;
    return *this;
  }

  Value& operator = (Value&& other) {
    data = std::move(other.data);
    return *this;
  }

  template <typename T, typename std::enable_if<detail::NonSelf<T, Value>()>::type* = nullptr>
  Value(T&& val) : data(std::forward<T>(val)) {}

  ~Value() { }

  template <typename T>
  T& as() {
    return data.template as<T>();
  }

  template <typename T>
  const T& as() const {
    return data.template as<T>();
  }

  template <typename T, typename... Args>
  Value& emplace(Args&&... args) {
    data.emplace<T, Args...>(std::forward<Args>(args)...);
    return *this;
  }
};

/*
template <>
String& Value::as<std::string>() {
  return as<String>();
}
*/

int main(int argc, char* argv[]) {
  Value value("this is a test");

  Value value2(value);
  value2.as<std::string>() = "this is also a test";

  const Value value3(value);
  Value value4((Test()));

  Value value5((Test()));
  value5 = value4;

  value5 = std::move(value4);
  value4 = "done";

  std::cout << value.as<std::string>() << "|" << value2.as<std::string>() << "|" << value3.as<std::string>() << "|" << value4.as<std::string>() << std::endl;
}
