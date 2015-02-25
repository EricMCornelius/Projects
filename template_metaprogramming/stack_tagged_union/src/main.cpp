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

  // http://stackoverflow.com/questions/22758291/how-can-i-detect-if-a-type-can-be-streamed-to-an-stdostream
  template<typename S, typename T>
  class is_streamable {
    template<typename SS, typename TT>
    static auto test(int)
    -> decltype( std::declval<SS&>() << std::declval<TT>(), std::true_type() );

    template<typename, typename>
    static auto test(...) -> std::false_type;

  public:
    static const bool value = decltype(test<S,T>(0))::value;
  };

  template <typename Type>
  struct deleter {
    template <typename UnionType>
    static void exec(UnionType* type) {
      type->template as<Type>().~Type();
    }
  };

  template <typename Type>
  struct assigner {
    template <typename UnionType>
    static void exec(UnionType* target, const UnionType& source) {
      target->template as<Type>() = source.template as<Type>();
    }

    template <typename UnionType>
    static void exec(UnionType* target, UnionType&& source) {
      target->template as<Type>() = std::move(source.template as<Type>());
    }
  };

  template <typename Type>
  struct constructor {
    template <typename UnionType>
    static void exec(UnionType* target, const UnionType& source) {
      ::new (&target->storage) Type(source.template as<Type>());
    }

    template <typename UnionType>
    static void exec(UnionType* target, UnionType&& source) {
      ::new (&target->storage) Type(source.template as<Type>());
      target->tag = source.tag;
    }
  };

  template <typename T, typename P, typename = void>
  struct accepts_type : std::false_type { };

  template <typename T, typename P>
  struct accepts_type<T, P, std::enable_if_t<std::is_same<decltype(std::declval<T>()(std::declval<P>())), void>::value>> : std::true_type { };

  template <typename Type, typename Visitor>
  decltype(auto) visit(Visitor&& v, const Type& t, typename std::enable_if<accepts_type<Visitor, Type>::value, bool>::type* = 0) {
    return v(t);
  }

  template <typename Type, typename Visitor>
  void visit(Visitor&& v, const Type& t, typename std::enable_if<!accepts_type<Visitor, Type>::value, bool>::type* = 0) {

  }

  template <typename Type>
  struct visitor {
    template <typename UnionType, typename Visitor>
    static decltype(auto) exec(UnionType* target, Visitor&& visitor) {
      return visit(std::forward<Visitor>(visitor), target->template as<Type>());
    }
  };

  template <std::size_t index, typename Typelist, typename Enable = void>
  struct tuple_index {
    typedef typename std::tuple_element<0, Typelist>::type type;
  };

  template <std::size_t index, typename Typelist>
  struct tuple_index<index, Typelist, typename std::enable_if<index < std::tuple_size<Typelist>::value>::type> {
    typedef typename std::tuple_element<index, Typelist>::type type;
  };

  template <template<typename> class Functor, typename UnionType, typename... Args>
  inline decltype(auto) dispatch(UnionType* val, Args&&... args) {
    typedef typename UnionType::typelist typelist;
    switch (val->tag) {
      case 0: {
        typedef typename tuple_index<0, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 1: {
        typedef typename tuple_index<1, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 2: {
        typedef typename tuple_index<2, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 3: {
        typedef typename tuple_index<3, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 4: {
        typedef typename tuple_index<4, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 5: {
        typedef typename tuple_index<5, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 6: {
        typedef typename tuple_index<6, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 7: {
        typedef typename tuple_index<7, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 8: {
        typedef typename tuple_index<8, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 9: {
        typedef typename tuple_index<9, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 10: {
        typedef typename tuple_index<10, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 11: {
        typedef typename tuple_index<11, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 12: {
        typedef typename tuple_index<12, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 13: {
        typedef typename tuple_index<13, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 14: {
        typedef typename tuple_index<14, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 15: {
        typedef typename tuple_index<15, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      default: {

      }
    }

    typedef typename tuple_index<0, typelist>::type Type;
    return Functor<Type>::exec(val, std::forward<Args>(args)...);
  }

  template <template<typename> class Functor, typename UnionType, typename... Args>
  inline decltype(auto) dispatch(const UnionType* val, Args&&... args) {
    typedef typename UnionType::typelist typelist;
    switch (val->tag) {
      case 0: {
        typedef typename tuple_index<0, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 1: {
        typedef typename tuple_index<1, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 2: {
        typedef typename tuple_index<2, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 3: {
        typedef typename tuple_index<3, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 4: {
        typedef typename tuple_index<4, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 5: {
        typedef typename tuple_index<5, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 6: {
        typedef typename tuple_index<6, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 7: {
        typedef typename tuple_index<7, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 8: {
        typedef typename tuple_index<8, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 9: {
        typedef typename tuple_index<9, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 10: {
        typedef typename tuple_index<10, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 11: {
        typedef typename tuple_index<11, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 12: {
        typedef typename tuple_index<12, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 13: {
        typedef typename tuple_index<12, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 14: {
        typedef typename tuple_index<12, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      case 15: {
        typedef typename tuple_index<12, typelist>::type Type;
        return Functor<Type>::exec(val, std::forward<Args>(args)...);
      }
      default: {

      }
    }

    typedef typename tuple_index<0, typelist>::type Type;
    return Functor<Type>::exec(val, std::forward<Args>(args)...);
  }

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
      if (tag == Index<T, typelist>::value) {
        as<T>() = std::forward<val>();
      }
      else {
        dispatch<deleter>(this);
        ::new (&storage) T(std::forward<val>());
      }
      return *this;
    }

    template <typename T>
    tagged_union& operator = (const T& val) {
      if (tag == Index<T, typelist>::value) {
        as<T>() = val;
      }
      else {
        dispatch<deleter>(this);
        ::new (&storage) T(val);
      }
      return *this;
    }

    template <typename T, typename... Init>
    tagged_union& emplace(Init&&... args) {
      if (tag)
        dispatch<deleter>(this);

      ::new (&storage) T(std::forward<Init>(args)...);
      tag = Index<T, typelist>::value;
    }

    tagged_union() {

    }

    tagged_union(const tagged_union& other) {
      tag = other.tag;
      dispatch<constructor>(this, other);
    }

    tagged_union& operator = (const tagged_union& other) {
      if (tag == other.tag) {
        dispatch<assigner>(this, other);
      }
      else {
        dispatch<deleter>(this);
        tag = other.tag;
        dispatch<constructor>(this, other);
      }
      return *this;
    }

    tagged_union& operator = (tagged_union&& other) {
      if (tag == other.tag) {
        dispatch<assigner>(this, std::forward<tagged_union>(other));
      }
      else {
        dispatch<deleter>(this);
        tag = other.tag;
        dispatch<constructor>(this, std::forward<tagged_union>(other));
      }
      return *this;
    }

    template <typename T, typename std::enable_if<NonSelf<T, tagged_union>()>::type* = nullptr>
    tagged_union(const T& val) {
      typedef typename std::remove_reference<T>::type type;
      ::new (&storage) type(val);
      tag = Index<type, typelist>::value;
    }

    template <typename T, typename std::enable_if<NonSelf<T, tagged_union>()>::type* = nullptr>
    tagged_union(T&& val) {
      typedef typename std::remove_reference<T>::type type;
      ::new (&storage) type(std::forward<T>(val));
      tag = Index<type, typelist>::value;
    }

    ~tagged_union() {
      dispatch<deleter>(this);
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& v) {
      return dispatch<visitor>(this, std::forward<Visitor>(v));
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& v) const {
      return dispatch<visitor>(this, std::forward<Visitor>(v));
    }
  };
}


struct stream_visitor {
  std::ostream& _out;

  stream_visitor(std::ostream& out = std::cout) : _out(out) { }

  template <typename T>
  typename std::enable_if<detail::is_streamable<std::ostream, T>::value, void>::type operator()(const T& obj) {
    _out << obj;
  }
};

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

  template <typename Visitor>
  decltype(auto) visit(Visitor&& visitor) {
    return data.visit(std::forward<Visitor>(visitor));
  }

  template <typename Visitor>
  decltype(auto) visit(Visitor&& visitor) const {
    return data.visit(std::forward<Visitor>(visitor));
  }
};

struct ValueRef {
  ValueRef(const Value& v) : val(v) {};
  ValueRef(Value& v) : val(v) {};

  const Value& val;
};

std::ostream& operator << (std::ostream& out, ValueRef v) {
  v.val.visit(stream_visitor(out));
  return out;
}


int main(int argc, char* argv[]) {
  Value value("this is a test");

  value.visit([](const std::string& str) {
    std::cout << str << std::endl;
  });

  std::string cpy;
  value.visit([&](const std::string& str) {
    cpy = str;
  });

  std::cout << "copy: " << cpy << std::endl;

  Value value2 = Test();
  std::cout << "nothing: " << value2 << std::endl;
}
