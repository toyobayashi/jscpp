#ifndef __JSCPP_STRING_HPP__
#define __JSCPP_STRING_HPP__

#include <cstddef>
#include <cstdint>
#include <utility>
#include <iostream>
#include <vector>
#include <regex>
#include "utf8.hpp"

namespace js {

class String;

class String {
private:
  std::wstring _str;

public:
  static String fromCharCode(uint16_t code) noexcept {
    return (wchar_t)code;
  }

  static String fromCodePoint(uint32_t code) noexcept {
    size_t len = sizeof(wchar_t);
    if (len == 2) {
      return (wchar_t)(code & 0x0000FFFFU);
    }
    return (wchar_t)code;
  }

private:
  void _trimZero() noexcept {
    size_t c = 0;
    size_t len = _str.length();
    for (size_t i = len - 1; i >= 0; i--) {
      if (_str[i] == L'0') c++; else break;
    }
    _str.resize(len - c);
  }

public:
  String() = default;

  String(char c) noexcept: _str(JSCPP_WSTR({ c, '\0' })) {}
  String(wchar_t c) noexcept: _str({ c, L'\0' }) {}
  String(const char* str) noexcept : _str(JSCPP_WSTR(str)) {}
  String(const std::string& str) noexcept : _str(JSCPP_WSTR(str)) {}
  String(const wchar_t* wstr) noexcept : _str(wstr) {}
  String(const std::wstring& wstr) noexcept : _str(wstr) {}
  String(int n) noexcept : _str(std::to_wstring(n)) {}
  String(unsigned int n) noexcept : _str(std::to_wstring(n)) {}
  String(long n) noexcept : _str(std::to_wstring(n)) {}
  String(unsigned long n) noexcept : _str(std::to_wstring(n)) {}
  String(long long n) noexcept : _str(std::to_wstring(n)) {}
  String(unsigned long long n) noexcept : _str(std::to_wstring(n)) {}
  String(float n) noexcept : _str(std::to_wstring(n)) { _trimZero(); }
  String(double n) noexcept : _str(std::to_wstring(n)) { _trimZero(); }
  String(long double n) noexcept : _str(std::to_wstring(n)) { _trimZero(); }

  size_t length() const noexcept {
    return _str.length();
  }

  const std::wstring& getWString() const noexcept {
    return _str;
  }

  std::wstring toWString() const noexcept {
    return _str;
  }

  std::string toString() const noexcept {
    return JSCPP_STR(_str);
  }

  const wchar_t* data() const noexcept {
    return _str.data();
  }

  const wchar_t& operator[](size_t index) const noexcept {
    return _str[index];
  }
  wchar_t& operator[](size_t index) noexcept {
    return _str[index];
  }

  String& operator+=(const String& str) {
    _str += str._str;
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& out, const String& str);
  friend std::istream& operator>>(std::istream& in, String& str);

  String charAt(size_t index) const noexcept {
    if (index >= _str.length()) return String();
    return _str[index];
  }

  uint16_t charCodeAt(size_t index = 0) const noexcept {
    if (index >= _str.length()) return (uint16_t)0U;
    uint32_t c = _str[index];
    if (c > 65535) {
      return (uint16_t)(c & 0x0000FFFFU);
    }
    return (uint16_t)c;
  }

  uint32_t codePointAt(size_t index = 0) const noexcept {
    if (index >= _str.length()) return 0U;
    return (uint32_t)_str[index];
  }

  String concat() const noexcept {
    return *this;
  }

  String concat(const String& s) const noexcept {
    return _str + s._str;
  }

  template <typename... Args>
  String concat(const String& s, Args... args) const noexcept {
    String tmp = this->concat(s);
    return tmp.concat(args...);
  }

  bool startsWith(const String& searchString, size_t position = 0) const noexcept;
  bool endsWith(const String& search) const noexcept;
  bool endsWith(const String& search, size_t length) const noexcept;

  bool includes(const String& searchString = L"undefined", size_t position = 0U) const noexcept {
    if (position + searchString.length() > this->length()) {
      return false;
    } else {
      return this->indexOf(searchString, position) != std::wstring::npos;
    }
  }

  size_t indexOf(const String& searchValue = L"undefined", size_t fromIndex = 0U) const noexcept {
    return _str.find_first_of(searchValue._str, fromIndex);
  }

  size_t lastIndexOf(const String& searchValue = L"undefined", size_t fromIndex = std::wstring::npos) const noexcept {
    return _str.find_last_of(searchValue._str, fromIndex);
  }

  String substring(size_t indexStart) const noexcept {
    size_t len = _str.length();
    if (indexStart > len) indexStart = len;
    return _str.substr(indexStart);
  }

  String substring(size_t indexStart, size_t indexEnd) const noexcept {
    size_t len = _str.length();
    if (indexStart > len) indexStart = len;
    if (indexEnd > len) indexEnd = len;
    if (indexStart == indexEnd) return String();
    if (indexStart > indexEnd) std::swap(indexStart, indexEnd);

    return _str.substr(indexStart, indexEnd - indexStart);
  }

  String padEnd(size_t targetLength, const String& str = L" ") const noexcept;
  String padStart(size_t targetLength, const String& str = L" ") const noexcept;

  String slice(int beginIndex = 0) const noexcept {
    if (beginIndex < 0) beginIndex = (int)length() + beginIndex;
    return this->substring(beginIndex);
  }
  String slice(int beginIndex, int endIndex) const noexcept {
    if (beginIndex < 0) beginIndex = (int)length() + beginIndex;
    if (endIndex < 0) endIndex = (int)length() + endIndex;
    return this->substring(beginIndex, endIndex);
  }

  String repeat(size_t count = 0) const noexcept {
    String str = *this;
    if (str.length() == 0 || count == 0) {
      return String();
    }
    if (str.length() * count >= 1 << 28) {
      return String();
    }
    String rpt;
    for (;;) {
      if ((count & 1) == 1) {
        rpt += str;
      }
      count = count >> 1;
      if (count == 0) {
        break;
      }
      str += str;
    }
    return rpt;
  }

  String replace(const String& substr, const String& newSubStr) const noexcept {
    String res = *this;
    size_t index = res.indexOf(substr);
    if (index != std::wstring::npos) {
      res._str.replace(index, substr.length(), newSubStr._str);
    }
    return res;
  }

  String replace(const std::wregex& regexp, const String& newSubStr) const noexcept {
    return std::regex_replace(_str, regexp, newSubStr._str);
  }

  std::vector<String> split() const noexcept {
    return { *this };
  }

  std::vector<String> split(const String& seprator, int limit = -1) const noexcept {
    size_t seplen = seprator.length();
    std::vector<String> res;
    if (seplen == 0) {
      size_t len = _str.length();
      for (size_t s = 0; s < len; s++) {
        if (limit >= 0 && res.size() == limit) {
          return res;
        }
        res.push_back(String(_str[s]));
      }
      return res;
    }
    String sub = *this;
    size_t rnindex;
    String left;
    String right;
    size_t rightStart = 0;
    while ((rnindex = sub.indexOf(seprator)) != std::wstring::npos) {
      left = this->substring(rightStart, rightStart + rnindex);
      rightStart = rightStart + rnindex + seplen;
      if (limit >= 0 && res.size() == limit) {
        return res;
      }
      res.push_back(left);
      right = this->substring(rightStart);
      sub = right;
    }
    if (limit >= 0 && res.size() == limit) {
      return res;
    }
    res.push_back(sub);
    return res;
  }

  String toLowerCase() const noexcept {
    String res = *this;
    size_t len = res.length();
    for (size_t i = 0; i < len; i++) {
      if (res[i] >= 65 && res[i] <= 90) {
        res[i] += 32;
      }
    }
    return res;
  }

  String toUpperCase() const noexcept {
    String res = *this;
    size_t len = res.length();
    for (size_t i = 0; i < len; i++) {
      if (res[i] >= 97 && res[i] <= 122) {
        res[i] -= 32;
      }
    }
    return res;
  }

  String trim() const noexcept {
    return this->replace(std::wregex(L"[\\s\\uFEFF\\xA0]+|[\\s\\uFEFF\\xA0]+$"), "");
  }
  String trimEnd() const noexcept {
    return this->replace(std::wregex(L"[\\s\\uFEFF\\xA0]+$"), "");
  }
  String trimStart() const noexcept {
    return this->replace(std::wregex(L"^[\\s\\uFEFF\\xA0]+"), "");
  }

  int compare(const String& s) const noexcept {
    return _str.compare(s._str);
  }

  void swap(String& s) noexcept {
    std::swap(this->_str, s._str);
  }
};

inline void swap(String& l, String& r) noexcept {
  return l.swap(r);
}

inline std::ostream& operator<<(std::ostream& out, const String& str) {
  out << JSCPP_STR(str._str);
  return out;
}

inline std::istream& operator>>(std::istream& in, String& str) {
  in >> str;
  return in;
}

inline String operator+(const String& l, const String& r) noexcept {
  return l.concat(r);
}

inline String operator+(const char* l, const String& r) noexcept {
  return String(l).concat(r);
}

inline String operator+(char l, const String& r) noexcept {
  return String(l).concat(r);
}

inline String operator+(const wchar_t* l, const String& r) noexcept {
  return String(l).concat(r);
}

inline String operator+(wchar_t l, const String& r) noexcept {
  return String(l).concat(r);
}

inline String operator+(const String& l, const char* r) noexcept {
  return l.concat(r);
}

inline String operator+(const String& l, char r) noexcept {
  return l.concat(r);
}

inline String operator+(const String& l, const wchar_t* r) noexcept {
  return l.concat(r);
}

inline String operator+(const String& l, wchar_t r) noexcept {
  return l.concat(r);
}

inline String operator+(const String& l, int r) noexcept { return l.concat(r); }
inline String operator+(const String& l, unsigned int r) noexcept { return l.concat(r); }
inline String operator+(const String& l, long r) noexcept { return l.concat(r); }
inline String operator+(const String& l, unsigned long r) noexcept { return l.concat(r); }
inline String operator+(const String& l, long long r) noexcept { return l.concat(r); }
inline String operator+(const String& l, unsigned long long r) noexcept { return l.concat(r); }
inline String operator+(const String& l, float r) noexcept { return l.concat(r); }
inline String operator+(const String& l, double r) noexcept { return l.concat(r); }
inline String operator+(const String& l, long double r) noexcept { return l.concat(r); }

inline String operator+(int l, const String& r) noexcept { return String(l).concat(r); }
inline String operator+(unsigned int l, const String& r) noexcept { return String(l).concat(r); }
inline String operator+(long l, const String& r) noexcept { return String(l).concat(r); }
inline String operator+(unsigned long l, const String& r) noexcept { return String(l).concat(r); }
inline String operator+(long long l, const String& r) noexcept { return String(l).concat(r); }
inline String operator+(unsigned long long l, const String& r) noexcept { return String(l).concat(r); }
inline String operator+(float l, const String& r) noexcept { return String(l).concat(r); }
inline String operator+(double l, const String& r) noexcept { return String(l).concat(r); }
inline String operator+(long double l, const String& r) noexcept { return String(l).concat(r); }

inline bool operator==(const String& l, const String& r) noexcept {
  return l.compare(r) == 0;
}

inline bool operator==(const char* l, const String& r) {
  return r.compare(l) == 0;
}

inline bool operator==(const wchar_t* l, const String& r) {
  return r.compare(l) == 0;
}

inline bool operator==(const String& l, const char* r) {
  return l.compare(r) == 0;
}

inline bool operator==(const String& l, const wchar_t* r) {
  return l.compare(r) == 0;
}

inline bool operator<(const String& l, const String& r) noexcept {
  return l.compare(r) < 0;
}

inline bool String::startsWith(const String& searchString, size_t position) const noexcept {
  return this->substring(position, position + searchString.length()) == searchString;
}

inline bool String::endsWith(const String& search) const noexcept {
  size_t length = _str.length();
  return this->substring(length - search.length(), length) == search;
}
inline bool String::endsWith(const String& search, size_t length) const noexcept {
  size_t len = _str.length();
  if (length > len) {
    length = len;
  }
  return this->substring(length - search.length(), length) == search;
}
inline String String::padEnd(size_t targetLength, const String& str) const noexcept {
  size_t len = _str.length();
  if (len > targetLength) {
    return *this;
  }
  targetLength = targetLength - len;
  String padString = str;
  size_t plen = padString.length();
  if (targetLength > plen) {
    padString += padString.repeat(targetLength / padString.length());
  }
  return *this + padString.substring(0, targetLength);
}
inline String String::padStart(size_t targetLength, const String& str) const noexcept {
  size_t len = _str.length();
  if (len > targetLength) {
    return *this;
  } else {
    targetLength = targetLength - len;
    String padString = str;
    if (targetLength > padString.length()) {
      padString += padString.repeat(targetLength / padString.length());
    }
    return padString.substring(0, targetLength) + *this;
  }
}

}

#endif
