#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#else

#include <clocale>
#include <cstddef>

#endif

#include "jscpp/utf8.hpp"

namespace js {

std::wstring fromUtf8(const std::string& str) {
#ifdef _WIN32
  int len = MultiByteToWideChar(65001, 0, str.c_str(), -1, nullptr, 0);
  if (len == 0) return L"";
  std::wstring res;
  res.resize(len - 1);
  MultiByteToWideChar(65001, 0, str.c_str(), -1, &res[0], len);
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

std::wstring fromString(const std::string& str) {
#ifdef _WIN32
  int len = MultiByteToWideChar(0, 0, str.c_str(), -1, nullptr, 0);
  if (len == 0) return L"";
  std::wstring res;
  res.resize(len - 1);
  MultiByteToWideChar(0, 0, str.c_str(), -1, &res[0], len);
  return res;
#else
  size_t len = std::mbstowcs(nullptr, str.c_str(), 0);
  std::wstring res;
  res.resize(len);
  std::mbstowcs(&res[0], str.c_str(), len + 1);
  return res;
#endif
}

std::string toUtf8(const std::wstring& wstr) {
#ifdef _WIN32
  int len = WideCharToMultiByte(65001, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (len == 0) return "";
  std::string res;
  res.resize(len - 1);
  WideCharToMultiByte(65001, 0, wstr.c_str(), -1, &res[0], len, nullptr, nullptr);
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

std::string toString(const std::wstring& wstr) {
#ifdef _WIN32
  int len = WideCharToMultiByte(0, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (len == 0) return "";
  std::string res;
  res.resize(len - 1);
  WideCharToMultiByte(0, 0, wstr.c_str(), -1, &res[0], len, nullptr, nullptr);
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
