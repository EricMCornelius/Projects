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
    std::cout << "copied test" << std::endl;
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
  struct Index<T, std::tuple<T, Types...>> {
    static const std::size_t value = 0;
  };

  template <class T, class U, class... Types>
  struct Index<T, std::tuple<U, Types...>> {
    static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
  };

  template <typename Deleters, typename UnionType, typename Typelist, typename Head, typename... Tail>
  auto init_deleters_impl(Deleters& deleters) {
    deleters[Index<Head, Typelist>::value] = [](UnionType* ptr) {
      std::cout << "cleanup: " << Index<Head, Typelist>::value << std::endl;
      delete static_cast<std::shared_ptr<Head>*>(ptr->get_ptr());
    };
    return init_deleters_impl<Deleters, UnionType, Typelist, Tail...>(deleters);
  }

  template <typename Deleters, typename UnionType, typename Typelist>
  auto init_deleters_impl(Deleters& deleters) {
    return deleters;
  }

  template <typename UnionType, typename... Args>
  auto init_deleters() {
    std::array<std::function<void(UnionType*)>, sizeof...(Args)> deleters;
    typedef std::tuple<Args...> Typelist;
    return init_deleters_impl<decltype(deleters), UnionType, Typelist, Args...>(deleters);
  }

  template <typename Copiers, typename UnionType, typename Typelist, typename Head, typename... Tail>
  auto init_copiers_impl(Copiers& copiers) {
    copiers[Index<Head, Typelist>::value] = [](UnionType* ptr) {
      std::cout << "copy: " << Index<Head, Typelist>::value << std::endl;
      return new std::shared_ptr<Head>(*static_cast<std::shared_ptr<Head>*>(ptr->get_ptr()));
    };
    return init_copiers_impl<Copiers, UnionType, Typelist, Tail...>(copiers);
  }

  template <typename Copiers, typename UnionType, typename Typelist>
  auto init_copiers_impl(Copiers& copiers) {
    return copiers;
  }

  template <typename UnionType, typename... Args>
  auto init_copiers() {
    std::array<std::function<void*(UnionType*)>, sizeof...(Args)> copiers;
    typedef std::tuple<Args...> Typelist;
    return init_copiers_impl<decltype(copiers), UnionType, Typelist, Args...>(copiers);
  }

  template <typename... Args>
  union tagged_union {
    typedef std::tuple<Args...> typelist;

    struct tag {
      uint64_t padding : 48;
      uint16_t tag : 16;
    } tag;
    void* ptr;

    void* get_ptr() {
      return reinterpret_cast<void*>(reinterpret_cast<uint64_t>(ptr) & 0x0000ffffffffffff);
    }

    void set_ptr(void* ptr_, uint16_t tag_) {
      ptr = ptr_;
      tag.tag = tag_;
    }

    template <typename T>
    T& as() {
      return **get<T>();
    }

    template <typename T>
    std::shared_ptr<T>* get() {
      return static_cast<std::shared_ptr<T>*>(get_ptr());
    }

    template <typename T>
    std::shared_ptr<T> clone() {
      return std::shared_ptr<T>(*static_cast<std::shared_ptr<T>*>(get_ptr()));
    }

    template <typename T>
    void set(std::shared_ptr<T>* ptr) {
      set_ptr(ptr, Index<T, typelist>::value);
    };

    template <typename T>
    tagged_union(T&& val) {
      set(
        new std::shared_ptr<T>(
          std::make_shared<T>(
            std::forward<T>(val)
          )
        )
      );
    }

    tagged_union() : ptr(nullptr) { }

    static const std::array<std::function<void(tagged_union*)>, sizeof...(Args)> deleters;
    static const std::array<std::function<void*(tagged_union*)>, sizeof...(Args)> copiers;

    ~tagged_union() {
      delete static_cast<std::shared_ptr<void>*>(get_ptr());
      //tagged_union::deleters[tag.tag](this);
    }

    std::shared_ptr<void> clone() {
      return *static_cast<std::shared_ptr<void>*>(get_ptr());
      //return static_cast<std::shared_ptr<void>*>(copiers[tag.tag](this));
    }
  };

  template <typename... Args>
  const std::array<std::function<void(tagged_union<Args...>*)>, sizeof...(Args)> tagged_union<Args...>::deleters = init_deleters<tagged_union<Args...>, Args...>();

  template <typename... Args>
  const std::array<std::function<void*(tagged_union<Args...>*)>, sizeof...(Args)> tagged_union<Args...>::copiers = init_copiers<tagged_union<Args...>, Args...>();
}

struct Value {
  typedef detail::tagged_union<Null, String, Object, Array, Number, Bool, Int32, Int64, UInt32, UInt64, Test> data_type;
  data_type data;

  Value() { }

  template <typename T>
  Value(T&& val) : data(std::forward<T>(val)) {}

  Value(const char* val) : Value(String(val)) {}

  ~Value() {

  }

  template <typename T>
  T& as() {
    return data.template as<T>();
  }

  auto clone() {
    return data.clone();
  }
};

template <>
String& Value::as<std::string>() {
  return data.template as<String>();
}

int main(int argc, char* argv[]) {
  Value last((Test()));
  std::cout << last.as<Test>().msg() << std::endl;

  Value value2(1);

  Value value("this is a test");
  std::cout << value.as<std::string>() << std::endl;


  {
    auto copy = last.clone();
    std::cout << copy.use_count();
    std::cout << " " << std::static_pointer_cast<Test>(copy)->msg() << std::endl;
  }
}
