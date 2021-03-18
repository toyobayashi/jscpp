#ifndef __JSCPP_PATH_HPP__
#define __JSCPP_PATH_HPP__

#include "String.hpp"

namespace js {

namespace path {

struct ParsedPath {
  String root;
  String dir;
  String base;
  String ext;
  String name;
};

namespace win32 {
  JSCPP_API bool isAbsolute(const String&);
  JSCPP_API String resolve(const String& arg1 = L"", const String& arg2 = L"");

  template <typename... Args>
  inline String resolve(const String& arg1, const String& arg2, Args... args) {
    String tmp = win32::resolve(arg1, arg2);
    return win32::resolve(tmp, args...);
  }
}

namespace posix {
  JSCPP_API bool isAbsolute(const String&);
  JSCPP_API String resolve(const String& arg1 = L"", const String& arg2 = L"");

  template <typename... Args>
  inline String resolve(const String& arg1, const String& arg2, Args... args) {
    String tmp = posix::resolve(arg1, arg2);
    return posix::resolve(tmp, args...);
  }
}

#ifdef _WIN32

inline bool isAbsolute(const String& path) { return win32::isAbsolute(path); }

inline String resolve(const String& arg1 = L"", const String& arg2 = L"") { return win32::resolve(arg1, arg2); }
template <typename... Args>
inline String resolve(const String& arg1, const String& arg2, Args... args) {
  String tmp = resolve(arg1, arg2);
  return resolve(tmp, args...);
}

#else

inline bool isAbsolute(const String& path) { return posix::isAbsolute(path); }

inline String resolve(const String& arg1 = L"", const String& arg2 = L"") { return posix::resolve(arg1, arg2); }
template <typename... Args>
inline String resolve(const String& arg1, const String& arg2, Args... args) {
  String tmp = resolve(arg1, arg2);
  return resolve(tmp, args...);
}

#endif

}

}

#endif
