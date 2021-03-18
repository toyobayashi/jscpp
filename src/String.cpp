#include "jscpp/utf8.hpp"
#include "jscpp/String.hpp"
#include "throw.hpp"

#include <utility>

namespace js {

String String::fromCharCode() noexcept {
  return "";
}

String String::fromCharCode(uint32_t code) noexcept {
  return (wchar_t)(uint16_t)code;
}

String String::fromCharCode(const std::vector<uint32_t>& codes) {
  String tmp;
  for (const uint32_t c : codes) {
    tmp += (wchar_t)(uint16_t)c;
  }
  return tmp;
}

String String::fromCodePoint(uint32_t code) {
  return fromCodePoint(std::vector<uint32_t>({ code }));
}

String String::fromCodePoint(const std::vector<uint32_t>& codes) {
  std::vector<uint32_t> codeUnits;
  size_t codeLen = 0;
  String result;
  size_t index = 0;
  size_t len = codes.size();
  for (; index != len; ++index) {
    uint32_t codePoint = codes[index];
    if (codePoint >= 0x10FFFF) {
      throwError(String("Invalid code point: ") + codePoint);
    }

    if (codePoint <= 0xFFFF) {
      codeUnits.push_back(codePoint);
      codeLen = codeUnits.size();
    } else {
      codePoint -= 0x10000;
      codeUnits.push_back((codePoint >> 10) + 0xD800);
      codeUnits.push_back((codePoint % 0x400) + 0xDC00);
      codeLen = codeUnits.size();
    }
    if (codeLen >= 0x3fff) {
      result += fromCharCode(codeUnits);
      codeUnits.clear();
    }
  }
  return result + fromCharCode(codeUnits);
}

void String::_trimZero() {
  size_t c = 0;
  size_t len = _str.length();
  for (size_t i = len - 1; i >= 0; i--) {
    if (_str[i] == L'0') c++; else break;
  }
  _str.resize(len - c);
}

String::String(char c) noexcept: _str(::js::wstr({ c })) {}
String::String(wchar_t c) noexcept: _str({ c }) {}
String::String(const char* str) noexcept : _str(::js::wstr(str)) {}
String::String(const std::string& str) noexcept : _str(::js::wstr(str)) {}
String::String(const wchar_t* wstr) noexcept : _str(wstr) {}
String::String(const std::wstring& wstr) noexcept : _str(wstr) {}
String::String(bool b): _str(b ? L"true" : L"false") {}
String::String(int n): _str(std::to_wstring(n)) {}
String::String(unsigned int n): _str(std::to_wstring(n)) {}
String::String(long n): _str(std::to_wstring(n)) {}
String::String(unsigned long n): _str(std::to_wstring(n)) {}
String::String(long long n): _str(std::to_wstring(n)) {}
String::String(unsigned long long n): _str(std::to_wstring(n)) {}
String::String(float n): _str(std::to_wstring(n)) { _trimZero(); }
String::String(double n): _str(std::to_wstring(n)) { _trimZero(); }
String::String(long double n): _str(std::to_wstring(n)) { _trimZero(); }

String::iterator String::begin() noexcept { return _str.begin(); }
String::const_iterator String::cbegin() const noexcept { return _str.cbegin(); }
std::reverse_iterator<String::iterator> String::rbegin() noexcept { return _str.rbegin(); }
std::reverse_iterator<String::const_iterator> String::crbegin() const noexcept { return _str.crbegin(); }
String::iterator String::end() noexcept { return _str.end(); }
String::const_iterator String::cend() const noexcept { return _str.cend(); }
std::reverse_iterator<String::iterator> String::rend() noexcept { return _str.rend(); }
std::reverse_iterator<String::const_iterator> String::crend() const noexcept { return _str.crend(); }

size_t String::length() const noexcept {
  return _str.length();
}

const std::wstring& String::ref() const noexcept {
  return _str;
}

std::wstring String::wstr() const noexcept {
  return _str;
}

std::string String::str() const noexcept {
  return ::js::str(_str);
}
std::string String::toString() const noexcept {
  return ::js::str(_str);
}

const wchar_t* String::data() const noexcept {
  return _str.data();
}

const wchar_t& String::operator[](size_t index) const noexcept {
  return _str[index];
}
wchar_t& String::operator[](size_t index) noexcept {
  return _str[index];
}

String& String::operator+=(const String& str) {
  _str += str._str;
  return *this;
}

std::ostream& operator<<(std::ostream& out, const String& str) {
  out << ::js::str(str._str);
  return out;
}

std::istream& operator>>(std::istream& in, String& str) {
  std::string tmp;
  in >> tmp;
  str += tmp;
  return in;
}

String String::charAt(size_t index) const noexcept {
  if (index >= _str.length()) return String();
  return _str[index];
}

uint16_t String::charCodeAt(size_t index) const noexcept {
  if (index >= _str.length()) return (uint16_t)0U;
  uint32_t c = _str[index];
  if (c > 65535) {
    return (uint16_t)(c & 0x0000FFFFU);
  }
  return (uint16_t)c;
}

uint32_t String::codePointAt(size_t position) const noexcept {
  String string = *this;
  size_t size = string.length();
  size_t index = position;
  if (index >= size) {
    return 0U;
  }
  uint16_t first = string.charCodeAt(index);
  uint16_t second;
  if (
    first >= 0xD800 && first <= 0xDBFF &&
    size > index + 1
  ) {
    second = string.charCodeAt(index + 1);
    if (second >= 0xDC00 && second <= 0xDFFF) {
      return ((uint32_t)first - 0xD800) * 0x400 + (uint32_t)second - 0xDC00 + 0x10000;
    }
  }
  return first;
}

String String::concat() const noexcept {
  return *this;
}

String String::concat(const String& s) const {
  return _str + s._str;
}

bool String::startsWith(const String& searchString, size_t position) const {
  return this->substring(position, position + searchString.length()) == searchString;
}

bool String::endsWith(const String& search) const {
  size_t length = _str.length();
  return this->substring(length - search.length(), length) == search;
}
bool String::endsWith(const String& search, size_t length) const {
  size_t len = _str.length();
  if (length > len) {
    length = len;
  }
  return this->substring(length - search.length(), length) == search;
}

bool String::includes(const String& searchString, size_t position) const noexcept {
  if (position + searchString.length() > this->length()) {
    return false;
  } else {
    return this->indexOf(searchString, position) != std::wstring::npos;
  }
}

size_t String::indexOf(const String& searchValue, size_t fromIndex) const noexcept {
  return _str.find(searchValue._str, fromIndex);
}

size_t String::lastIndexOf(const String& searchValue, size_t fromIndex) const noexcept {
  return _str.rfind(searchValue._str, fromIndex);
}

String String::substring(size_t indexStart) const {
  size_t len = _str.length();
  if (indexStart > len) indexStart = len;
  return _str.substr(indexStart);
}

String String::substring(size_t indexStart, size_t indexEnd) const {
  size_t len = _str.length();
  if (indexStart > len) indexStart = len;
  if (indexEnd > len) indexEnd = len;
  if (indexStart == indexEnd) return String();
  if (indexStart > indexEnd) std::swap(indexStart, indexEnd);

  return _str.substr(indexStart, indexEnd - indexStart);
}

String String::padEnd(size_t targetLength, const String& str) const {
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
String String::padStart(size_t targetLength, const String& str) const {
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

String String::slice(int beginIndex) const {
  if (beginIndex < 0) beginIndex = (int)length() + beginIndex;
  return this->substring(beginIndex);
}
String String::slice(int beginIndex, int endIndex) const {
  if (beginIndex < 0) beginIndex = (int)length() + beginIndex;
  if (endIndex < 0) endIndex = (int)length() + endIndex;
  return this->substring(beginIndex, endIndex);
}

String String::repeat(size_t count) const {
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

String String::replace(const String& substr, const String& newSubStr) const {
  String res = *this;
  size_t index = res.indexOf(substr);
  if (index != std::wstring::npos) {
    res._str.replace(index, substr.length(), newSubStr._str);
  }
  return res;
}

String String::replace(const std::wregex& regexp, const String& newSubStr) const {
  return std::regex_replace(_str, regexp, newSubStr._str);
}

std::vector<String> String::split() const noexcept {
  return { *this };
}

std::vector<String> String::split(const String& seprator, int limit) const {
  size_t seplen = seprator.length();
  std::vector<String> res;
  if (seplen == 0) {
    size_t len = _str.length();
    size_t cap = limit >= 0 ? ((size_t)limit < len ? limit : len) : len;
    res.reserve(cap);
    for (size_t s = 0; s < len; s++) {
      if (limit >= 0 && res.size() == limit) {
        return res;
      }
      res.emplace_back(_str[s]);
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
    res.emplace_back(left);
    right = this->substring(rightStart);
    sub = right;
  }
  if (limit >= 0 && res.size() == limit) {
    return res;
  }
  res.emplace_back(sub);
  return res;
}

String String::toLowerCase() const noexcept {
  String res = *this;
  size_t len = res.length();
  for (size_t i = 0; i < len; i++) {
    if (res[i] >= 65 && res[i] <= 90) {
      res[i] += 32;
    }
  }
  return res;
}

String String::toUpperCase() const noexcept {
  String res = *this;
  size_t len = res.length();
  for (size_t i = 0; i < len; i++) {
    if (res[i] >= 97 && res[i] <= 122) {
      res[i] -= 32;
    }
  }
  return res;
}

String String::trim() const {
  return this->replace(std::wregex(L"[\\s\\uFEFF\\xA0]+|[\\s\\uFEFF\\xA0]+$"), "");
}
String String::trimEnd() const {
  return this->replace(std::wregex(L"[\\s\\uFEFF\\xA0]+$"), "");
}
String String::trimStart() const {
  return this->replace(std::wregex(L"^[\\s\\uFEFF\\xA0]+"), "");
}
String String::trimRight() const {
  return this->trimEnd();
}
String String::trimLeft() const {
  return this->trimStart();
}

int String::compare(const String& s) const noexcept {
  return _str.compare(s._str);
}

void String::swap(String& s) noexcept {
  std::swap(this->_str, s._str);
}

void swap(String& l, String& r) noexcept {
  return l.swap(r);
}

String operator+(const String& l, const String& r) {
  return l.concat(r);
}

String operator+(const char* l, const String& r) {
  return String(l).concat(r);
}

String operator+(char l, const String& r) {
  return String(l).concat(r);
}

String operator+(const wchar_t* l, const String& r) {
  return String(l).concat(r);
}

String operator+(wchar_t l, const String& r) {
  return String(l).concat(r);
}

String operator+(const String& l, const char* r) {
  return l.concat(r);
}

String operator+(const String& l, char r) {
  return l.concat(r);
}

String operator+(const String& l, const wchar_t* r) {
  return l.concat(r);
}

String operator+(const String& l, wchar_t r) {
  return l.concat(r);
}

String operator+(const String& l, bool r) { return l.concat(r); }
String operator+(const String& l, int r) { return l.concat(r); }
String operator+(const String& l, unsigned int r) { return l.concat(r); }
String operator+(const String& l, long r) { return l.concat(r); }
String operator+(const String& l, unsigned long r) { return l.concat(r); }
String operator+(const String& l, long long r) { return l.concat(r); }
String operator+(const String& l, unsigned long long r) { return l.concat(r); }
String operator+(const String& l, float r) { return l.concat(r); }
String operator+(const String& l, double r) { return l.concat(r); }
String operator+(const String& l, long double r) { return l.concat(r); }

String operator+(bool l, const String& r) { return String(l).concat(r); }
String operator+(int l, const String& r) { return String(l).concat(r); }
String operator+(unsigned int l, const String& r) { return String(l).concat(r); }
String operator+(long l, const String& r) { return String(l).concat(r); }
String operator+(unsigned long l, const String& r) { return String(l).concat(r); }
String operator+(long long l, const String& r) { return String(l).concat(r); }
String operator+(unsigned long long l, const String& r) { return String(l).concat(r); }
String operator+(float l, const String& r) { return String(l).concat(r); }
String operator+(double l, const String& r) { return String(l).concat(r); }
String operator+(long double l, const String& r) { return String(l).concat(r); }

bool operator==(const String& l, const String& r) noexcept {
  return l.compare(r) == 0;
}

bool operator==(const char* l, const String& r) noexcept {
  return r.compare(l) == 0;
}

bool operator==(const wchar_t* l, const String& r) noexcept {
  return r.compare(l) == 0;
}

bool operator==(const String& l, const char* r) noexcept {
  return l.compare(r) == 0;
}

bool operator==(const String& l, const wchar_t* r) noexcept {
  return l.compare(r) == 0;
}

bool operator<(const String& l, const String& r) noexcept {
  return l.compare(r) < 0;
}

}
