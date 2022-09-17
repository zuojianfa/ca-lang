#include <cstddef>
#include <cstdint>

template<class T> struct GenSlice {
  T *ptr;
  size_t len;
};

extern "C" {
  using I8Slice = GenSlice<int8_t>;
  using I16Slice = GenSlice<int16_t>;
  using I32Slice = GenSlice<int32_t>;
  using I64Slice = GenSlice<int64_t>;
  using U8Slice = GenSlice<uint8_t>;
  using U16Slice = GenSlice<uint16_t>;
  using U32Slice = GenSlice<uint32_t>;
  using U64Slice = GenSlice<uint64_t>;
}

