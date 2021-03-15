#ifndef __JSCPP_ENCODING_HPP__
#define __JSCPP_ENCODING_HPP__

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#else

#include <clocale>
#include <cstddef>

#endif

#include <string>

#ifdef JSCPP_UTF8
  #define JSCPP_STR js::toUtf8
  #define JSCPP_WSTR js::fromUtf8
#else
  #define JSCPP_STR js::toString
  #define JSCPP_WSTR js::fromString
#endif

namespace js {

inline std::wstring fromUtf8(const std::string& str) {
#ifdef _WIN32
  int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
  if (len == 0) return L"";
  std::wstring res;
  res.resize(len - 1);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &res[0], len);
  return res;
#else
  std::string targetLocale = "en_US.utf8";
  const char* cLocale = std::setlocale(LC_CTYPE, nullptr);
  std::string locale(cLocale ? cLocale : "");

  if (locale != targetLocale) {
    std::setlocale(LC_CTYPE, targetLocale.c_str());
  }

  size_t len = std::mbstowcs(nullptr, str.c_str(), 0);
  std::wstring res;
  res.resize(len);
  std::mbstowcs(&res[0], str.c_str(), len + 1);

  if (locale != "") {
    std::setlocale(LC_CTYPE, locale.c_str());
  }
  return res;
#endif
}

inline std::wstring fromString(const std::string& str) {
#ifdef _WIN32
  int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
  if (len == 0) return L"";
  std::wstring res;
  res.resize(len - 1);
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &res[0], len);
  return res;
#else
  size_t len = std::mbstowcs(nullptr, str.c_str(), 0);
  std::wstring res;
  res.resize(len);
  std::mbstowcs(&res[0], str.c_str(), len + 1);
  return res;
#endif
}

inline std::string toUtf8(const std::wstring& wstr) {
#ifdef _WIN32
  int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (len == 0) return "";
  std::string res;
  res.resize(len - 1);
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &res[0], len, nullptr, nullptr);
  return res;
#else
  std::string targetLocale = "en_US.utf8";
  const char* cLocale = std::setlocale(LC_CTYPE, nullptr);
  std::string locale(cLocale ? cLocale : "");

  if (locale != targetLocale) {
    std::setlocale(LC_CTYPE, targetLocale.c_str());
  }

  size_t len = std::wcstombs(nullptr, wstr.c_str(), 0);
  std::string res;
  res.resize(len);
  std::wcstombs(&res[0], wstr.c_str(), len + 1);

  if (locale != "") {
    std::setlocale(LC_CTYPE, locale.c_str());
  }
  return res;
#endif
}

inline std::string toString(const std::wstring& wstr) {
#ifdef _WIN32
  int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (len == 0) return "";
  std::string res;
  res.resize(len - 1);
  WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &res[0], len, nullptr, nullptr);
  return res;
#else
  size_t len = std::wcstombs(nullptr, wstr.c_str(), 0);
  std::string res;
  res.resize(len);
  std::wcstombs(&res[0], wstr.c_str(), len + 1);
  return res;
#endif
}

}

#endif
