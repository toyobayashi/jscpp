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

  JSCPP_API String normalize(const String&);
  JSCPP_API String join();
  JSCPP_API String join(const String& arg1);
  JSCPP_API String join(const String& arg1, const String& arg2);

  template <typename... Args>
  inline String join(const String& arg1, const String& arg2, Args... args) {
    String tmp = win32::join(arg1, arg2);
    return win32::join(tmp, args...);
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

  JSCPP_API String normalize(const String&);
  JSCPP_API String join();
  JSCPP_API String join(const String& arg1);
  JSCPP_API String join(const String& arg1, const String& arg2);

  template <typename... Args>
  inline String join(const String& arg1, const String& arg2, Args... args) {
    String tmp = posix::join(arg1, arg2);
    return posix::join(tmp, args...);
  }
}

#ifdef _WIN32

inline bool isAbsolute(const String& path) { return win32::isAbsolute(path); }

template <typename... Args>
inline String resolve(Args... args) {
  return win32::resolve(args...);
}

inline String normalize(const String& path) { return win32::normalize(path); }

template <typename... Args>
inline String join(Args... args) {
  return win32::join(args...);
}

#else

inline bool isAbsolute(const String& path) { return posix::isAbsolute(path); }

template <typename... Args>
inline String resolve(Args... args) {
  return posix::resolve(args...);
}

inline String normalize(const String& path) { return posix::normalize(path); }

template <typename... Args>
inline String join(Args... args) {
  return posix::join(args...);
}

#endif

}

}

#endif
