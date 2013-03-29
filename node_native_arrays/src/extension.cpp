#include <v8.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <functional>

/*
#include <GL/glew.h>
#include <GL/glfw3.h>
#include <GL/glfw3native.h>

static void error_callback(int error, const char* description) {
  std::cerr << description << std::endl;
}

static void init(int width, int height) {
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    std::cerr << "Unable to initialize glfw" << std::endl;
    exit(1);
  }

  auto window = glfwCreateWindow(width, height, "Simple example", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    std::cerr << "Unable to create glfw window" << std::endl;
    exit(1);
  }

  glfwMakeContextCurrent(window);

  auto err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "Unable to initialize glew" << std::endl;
    exit(1);
  }
}
*/

using namespace v8;

constexpr char source_str[] = 
R"(
native function Test();
// native function Init();
native function Sample();
)";

v8::Handle<v8::Value>
Test2(const v8::Arguments& args) {
  std::cout << "Test2: " << args.Length() << std::endl;
  return v8::Integer::New(999);
}

v8::Handle<v8::Value>
Test(const v8::Arguments& args) {
  std::cout << "Test1: " << args.Length() << std::endl;
  auto t2 = v8::FunctionTemplate::New(Test2);
  t2->SetClassName(v8::String::New("Test Function Class"));
  return t2->GetFunction();
}

/*
v8::Handle<v8::Value>
Init(const v8::Arguments& args) {
  if (args.Length() == 2 && args[0]->IsNumber() && args[1]->IsNumber()) {
    init(args[0]->ToNumber()->Value(), args[1]->ToNumber()->Value());
    return v8::Undefined();
  }
  return v8::ThrowException(v8::String::New("Invalid arguments to glfw init"));
}
*/

template <typename Elem>
class Buffer {
public:
  typedef Elem value_type;

  Buffer(void* mem, std::size_t size) : _mem(static_cast<Elem*>(mem)), _size(size) { }

  Elem& operator[](std::size_t idx) {
    return _mem[idx];
  }

  const Elem& operator[](std::size_t idx) const {
    return _mem[idx];
  }

  Elem* begin() {
    return _mem;
  }

  const Elem* begin() const {
    return _mem;
  }

  Elem* end() {
    return &_mem[_size];
  }

  const Elem* end() const {
    return &_mem[_size];
  }

  const std::size_t size() const {
    return _size;
  }

private:
  Elem* _mem;
  std::size_t _size;
};

template <ExternalArrayType>
struct ArrayTraits {
  static constexpr const char* name = "Undefined";
  typedef Buffer<void*> type;
};

template <>
struct ArrayTraits<kExternalByteArray> {
  static constexpr const char* name = "Int8Array";
  typedef Buffer<int8_t> type;
};

template <>
struct ArrayTraits<kExternalUnsignedByteArray> {
  static constexpr const char* name = "Uint8Array";
  typedef Buffer<uint8_t> type;
};

template <>
struct ArrayTraits<kExternalShortArray> {
  static constexpr const char* name = "Int16Array";
  typedef Buffer<int16_t> type;
};

template <>
struct ArrayTraits<kExternalUnsignedShortArray> {
  static constexpr const char* name = "Uint16Array";
  typedef Buffer<uint16_t> type;
};

template <>
struct ArrayTraits<kExternalIntArray> {
  static constexpr const char* name = "Int32Array";
  typedef Buffer<int32_t> type;
};

template <>
struct ArrayTraits<kExternalUnsignedIntArray> {
  static constexpr const char* name = "Uint32Array";
  typedef Buffer<uint32_t> type;
};

template <>
struct ArrayTraits<kExternalFloatArray> {
  static constexpr const char* name = "Float32Array";
  typedef Buffer<float> type;
};

template <>
struct ArrayTraits<kExternalDoubleArray> {
  static constexpr const char* name = "Float64Array";
  typedef Buffer<double> type;
};

template <>
struct ArrayTraits<kExternalPixelArray> {
  static constexpr const char* name = "Uint8ClampedArray";
  typedef Buffer<uint8_t> type;
};

#define TYPED_CALL(function, trait, type, ...) {                                     \
  switch(type) {                                                                     \
    case kExternalByteArray:                                                         \
      function(ArrayTraits<kExternalByteArray>::trait , ## __VA_ARGS__);             \
      break;                                                                         \
    case kExternalUnsignedByteArray:                                                 \
      function(ArrayTraits<kExternalUnsignedByteArray>::trait , ## __VA_ARGS__);     \
      break;                                                                         \
    case kExternalShortArray:                                                        \
      function(ArrayTraits<kExternalShortArray>::trait , ## __VA_ARGS__);            \
      break;                                                                         \
    case kExternalUnsignedShortArray:                                                \
      function(ArrayTraits<kExternalUnsignedShortArray>::trait , ## __VA_ARGS__);    \
      break;                                                                         \
    case kExternalIntArray:                                                          \
      function(ArrayTraits<kExternalIntArray>::trait , ## __VA_ARGS__);              \
      break;                                                                         \
    case kExternalUnsignedIntArray:                                                  \
      function(ArrayTraits<kExternalUnsignedIntArray>::trait , ## __VA_ARGS__);      \
      break;                                                                         \
    case kExternalFloatArray:                                                        \
      function(ArrayTraits<kExternalFloatArray>::trait , ## __VA_ARGS__);            \
      break;                                                                         \
    case kExternalDoubleArray:                                                       \
      function(ArrayTraits<kExternalDoubleArray>::trait , ## __VA_ARGS__);           \
      break;                                                                         \
    case kExternalPixelArray:                                                        \
      function(ArrayTraits<kExternalPixelArray>::trait , ## __VA_ARGS__);            \
      break;                                                                         \
  }                                                                                  \
}

#define TYPED_CONSTRUCT(function, trait, type, ...) {                                     \
  switch(type) {                                                                          \
    case kExternalByteArray:                                                              \
      function(typename ArrayTraits<kExternalByteArray>::trait(__VA_ARGS__));             \
      break;                                                                              \
    case kExternalUnsignedByteArray:                                                      \
      function(typename ArrayTraits<kExternalUnsignedByteArray>::trait(__VA_ARGS__));     \
      break;                                                                              \
    case kExternalShortArray:                                                             \
      function(typename ArrayTraits<kExternalShortArray>::trait(__VA_ARGS__));            \
      break;                                                                              \
    case kExternalUnsignedShortArray:                                                     \
      function(typename ArrayTraits<kExternalUnsignedShortArray>::trait(__VA_ARGS__));    \
      break;                                                                              \
    case kExternalIntArray:                                                               \
      function(typename ArrayTraits<kExternalIntArray>::trait(__VA_ARGS__));              \
      break;                                                                              \
    case kExternalUnsignedIntArray:                                                       \
      function(typename ArrayTraits<kExternalUnsignedIntArray>::trait(__VA_ARGS__));      \
      break;                                                                              \
    case kExternalFloatArray:                                                             \
      function(typename ArrayTraits<kExternalFloatArray>::trait(__VA_ARGS__));            \
      break;                                                                              \
    case kExternalDoubleArray:                                                            \
      function(typename ArrayTraits<kExternalDoubleArray>::trait(__VA_ARGS__));           \
      break;                                                                              \
    case kExternalPixelArray:                                                             \
      function(typename ArrayTraits<kExternalPixelArray>::trait(__VA_ARGS__));            \
      break;                                                                              \
  }                                                                                       \
}

template <typename Container>
void print(const Container& cont) {
  std::cout << cont.size() << std::endl;
  for (const auto& elem : cont) {
    std::cout << elem << " ";
  }
  std::cout << std::endl;
}

template <typename Container>
void randomize(Container cont) {
  for (auto& elem : cont) {
    elem = rand();
  }
  /*
  auto i = 0;
  for (i = 0; i < cont.size(); ++i) {
    cont[i] = rand();
  //  std::cout << "Set element: " << i << " -> " << cont[i] << std::endl;
  }
  */
}

v8::Handle<v8::Value>
Sample(const v8::Arguments& args) {
  Local<Object> arr = args[0]->ToObject();
  auto size = arr->GetIndexedPropertiesExternalArrayDataLength();
  const ExternalArrayType type = arr->GetIndexedPropertiesExternalArrayDataType();
  auto func = [](const char* name) { std::cout << name << std::endl; }; 
  TYPED_CALL(func, name, type);

  TYPED_CONSTRUCT(print, type, type, arr->GetIndexedPropertiesExternalArrayData(), size);
  TYPED_CONSTRUCT(randomize, type, type, arr->GetIndexedPropertiesExternalArrayData(), size);
  return v8::Handle<v8::Value>();
}

class TestExtension : public v8::Extension {
public:
  TestExtension() : v8::Extension("TestExtension", source_str) { set_auto_enable(true); }

  virtual v8::Handle<v8::FunctionTemplate>
  GetNativeFunction(v8::Handle<v8::String> name) {
    std::cout << "Register...: " << *v8::String::Utf8Value(name) << std::endl;
    if (name->Equals(v8::String::New("Test")))
      return v8::FunctionTemplate::New(Test);
    //else if(name->Equals(v8::String::New("Init")))
    //  return v8::FunctionTemplate::New(Init);
    else if(name->Equals(v8::String::New("Sample")))
      return v8::FunctionTemplate::New(Sample);
    
    return v8::FunctionTemplate::New(Test);
  }
};

DeclareExtension test(new TestExtension());
