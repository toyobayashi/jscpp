#ifndef __JSCPP_STRING_HPP__
#define __JSCPP_STRING_HPP__

#include "config.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <regex>

namespace js {

class JSCPP_API String {
private:
  std::wstring _str;

public:
  static String fromCharCode() noexcept;
  static String fromCharCode(uint32_t code) noexcept;
  static String fromCharCode(const std::vector<uint32_t>& codes);

  template <typename... Args>
  static String fromCharCode(uint32_t code, const Args&... args) {
    String tmp = (wchar_t)(uint16_t)(code);
    return tmp + fromCharCode(args...);
  }

  static String fromCodePoint(uint32_t code);
  static String fromCodePoint(const std::vector<uint32_t>& codes);

private:
  void _trimZero();

public:
  String() = default;

  String(char c) noexcept;
  String(wchar_t c) noexcept;
  String(const char* str) noexcept;
  String(const std::string& str) noexcept;
  String(const wchar_t* wstr) noexcept;
  String(const std::wstring& wstr) noexcept;
  String(bool b);
  String(int n);
  String(unsigned int n);
  String(long n);
  String(unsigned long n);
  String(long long n);
  String(unsigned long long n);
  String(float n);
  String(double n);
  String(long double n);

  using iterator = std::wstring::iterator;
  using const_iterator = std::wstring::const_iterator;

  iterator begin() noexcept;
  const_iterator cbegin() const noexcept;
  std::reverse_iterator<iterator> rbegin() noexcept;
  std::reverse_iterator<const_iterator> crbegin() const noexcept;
  iterator end() noexcept;
  const_iterator cend() const noexcept;
  std::reverse_iterator<iterator> rend() noexcept;
  std::reverse_iterator<const_iterator> crend() const noexcept;

  size_t length() const noexcept;

  const std::wstring& ref() const noexcept;

  std::wstring wstr() const noexcept;

  std::string str() const noexcept;
  std::string toString() const noexcept;

  const wchar_t* data() const noexcept;

  const wchar_t& operator[](size_t index) const noexcept;
  wchar_t& operator[](size_t index) noexcept;

  String& operator+=(const String& str);

  friend JSCPP_API std::ostream& operator<<(std::ostream& out, const String& str);
  friend JSCPP_API std::istream& operator>>(std::istream& in, String& str);

  String charAt(size_t index) const noexcept;

  uint16_t charCodeAt(size_t index = 0) const noexcept;
  uint32_t codePointAt(size_t position = 0) const noexcept;

  String concat() const noexcept;
  String concat(const String& s) const;

  template <typename... Args>
  String concat(const String& s, const Args&... args) const {
    String tmp = this->concat(s);
    return tmp.concat(args...);
  }

  bool startsWith(const String& searchString, size_t position = 0) const;
  bool endsWith(const String& search) const;
  bool endsWith(const String& search, size_t length) const;

  bool includes(const String& searchString = L"undefined", size_t position = 0U) const noexcept;

  size_t indexOf(const String& searchValue = L"undefined", size_t fromIndex = 0U) const noexcept;
  size_t lastIndexOf(const String& searchValue = L"undefined", size_t fromIndex = std::wstring::npos) const noexcept;

  String substring(size_t indexStart) const;
  String substring(size_t indexStart, size_t indexEnd) const;

  String padEnd(size_t targetLength, const String& str = L" ") const;
  String padStart(size_t targetLength, const String& str = L" ") const;

  String slice(int beginIndex = 0) const;
  String slice(int beginIndex, int endIndex) const;

  String repeat(size_t count = 0) const;

  String replace(const String& substr, const String& newSubStr) const;
  String replace(const std::wregex& regexp, const String& newSubStr) const;

  std::vector<String> split() const noexcept;
  std::vector<String> split(const String& seprator, int limit = -1) const;

  String toLowerCase() const noexcept;
  String toUpperCase() const noexcept;

  String trim() const;
  String trimEnd() const;
  String trimStart() const;
  String trimRight() const;
  String trimLeft() const;

  int compare(const String& s) const noexcept;

  void swap(String& s) noexcept;
};

JSCPP_API void swap(String& l, String& r) noexcept;

JSCPP_API String operator+(const String& l, const String& r);
JSCPP_API String operator+(const char* l, const String& r);
JSCPP_API String operator+(char l, const String& r);
JSCPP_API String operator+(const wchar_t* l, const String& r);
JSCPP_API String operator+(wchar_t l, const String& r);
JSCPP_API String operator+(const String& l, const char* r);
JSCPP_API String operator+(const String& l, char r);
JSCPP_API String operator+(const String& l, const wchar_t* r);
JSCPP_API String operator+(const String& l, wchar_t r);

JSCPP_API String operator+(const String& l, bool r);
JSCPP_API String operator+(const String& l, int r);
JSCPP_API String operator+(const String& l, unsigned int r);
JSCPP_API String operator+(const String& l, long r);
JSCPP_API String operator+(const String& l, unsigned long r);
JSCPP_API String operator+(const String& l, long long r);
JSCPP_API String operator+(const String& l, unsigned long long r);
JSCPP_API String operator+(const String& l, float r);
JSCPP_API String operator+(const String& l, double r);
JSCPP_API String operator+(const String& l, long double r);

JSCPP_API String operator+(bool l, const String& r);
JSCPP_API String operator+(int l, const String& r);
JSCPP_API String operator+(unsigned int l, const String& r);
JSCPP_API String operator+(long l, const String& r);
JSCPP_API String operator+(unsigned long l, const String& r);
JSCPP_API String operator+(long long l, const String& r);
JSCPP_API String operator+(unsigned long long l, const String& r);
JSCPP_API String operator+(float l, const String& r);
JSCPP_API String operator+(double l, const String& r);
JSCPP_API String operator+(long double l, const String& r);

JSCPP_API bool operator==(const String& l, const String& r) noexcept;
JSCPP_API bool operator==(const char* l, const String& r) noexcept;
JSCPP_API bool operator==(const wchar_t* l, const String& r) noexcept;
JSCPP_API bool operator==(const String& l, const char* r) noexcept;
JSCPP_API bool operator==(const String& l, const wchar_t* r) noexcept;

JSCPP_API bool operator<(const String& l, const String& r) noexcept;

}

namespace std {

template<>
struct hash<::js::String> {
  size_t operator()(const ::js::String& str) const noexcept {
    return std::hash<std::wstring>{}(str.ref());
  }
};

}

#endif
