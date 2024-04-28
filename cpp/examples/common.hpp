#pragma once

#include <cassert>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>

#include <type_traits>

#include <array>

#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif


extern std::ostream* kOutStream;


template <typename T>
class has_cbegin_cend
{
  typedef char one;
  struct two { char x[2]; };
  template <typename C> static one test(decltype(&C::cbegin));
  template <typename C> static two test(...);

public:
  static const bool value = sizeof(test<T>(0)) == sizeof(char) && !std::is_same_v<T, std::string>;
};


template<typename C>
std::enable_if_t<has_cbegin_cend<C>::value, std::ostream&>
operator<<(std::ostream& os, const C& x) {
  os << '[';
  // for (const auto& item : x) os << item << ", ";
  { 
    const auto cend = x.cend();
    const auto cbegin = x.cbegin();
    for (auto item = cbegin; item != cend; ++item) {
      if (item != cbegin) os << ", ";
      os << *item;
    }
  }
  os << ']';
  return os;
}


// param[in]  msg
// param[in]  b   number of new lines after msg
inline void print(const char* msg, int b=0, char c='\0') {
  if (kOutStream == nullptr) return;
  *kOutStream << msg;
  if (c) {
    const auto N = std::strlen(msg);
    *kOutStream << '\n';
    for (int n = 0; n < N; ++n) *kOutStream << c;
    *kOutStream << '\n';
  }
  for (int n = 0; n < b; ++n) *kOutStream << '\n';
}

// param[in]  a   number of new lines before msg
// param[in]  msg
// param[in]  b   number of new lines after msg
inline void print(int a, const char* msg, int b=0, char c='\0') {
  if (kOutStream == nullptr) return;
  for (int n = 0; n < a; ++n) *kOutStream << '\n';
  print(msg, b, c);
}


template<typename T>
inline void print_mv(const char* msg, const T& val) {
  if (kOutStream == nullptr) return;
  *kOutStream << msg << val << '\n';
}


template<typename Container>
inline void print_seq(const Container& x) {
  if (kOutStream == nullptr) return;
  *kOutStream << '[';
  for (const auto& item : x) *kOutStream << item << ", ";
  *kOutStream << ']';
}

///! print expression and its value in one line
#define printv(expr) {                                  \
  if (kOutStream != nullptr) {*kOutStream << #expr << ": " << (expr) << "\n";} \
}

///! print expression and its value in new line
#define printvl(expr) {                                  \
  if (kOutStream != nullptr) {*kOutStream << #expr << ":\n" << (expr) << "\n";} \
}
