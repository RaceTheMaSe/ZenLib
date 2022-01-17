#pragma once
#include <cinttypes>
#include <cstring>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#elif __CLANG__
#pragma Clang diagnostic push
#pragma Clang diagnostic ignored "-Wcast-qual"
#endif

namespace Utils
{
  using unaligned_int32 = int32_t;
  using unaligned_int16 = int16_t;
  using unaligned_float = float;
  /**
  * @brief performs an unaligned memory-access
  */
  template <typename T>
  void getUnaligned(T* dst, const void* src)
  {
    memcpy(dst, src, sizeof(T));
  }

  template <typename T>
  void unalignedRead(T& dst, const void*& src)
  {
    getUnaligned(&dst, src);
    uint8_t*& ptr = ((uint8_t*&)src);
    ptr += sizeof(T);
  }
}  // namespace Utils

#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif __CLANG__
#pragma Clang diagnostic pop
#endif
