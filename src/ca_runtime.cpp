/**
 * Copyright (c) 2023 Rusheng Xia <xrsh_2004@163.com>
 * CA Programming Language and CA Compiler are licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

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

